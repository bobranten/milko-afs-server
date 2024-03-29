/*
 * Copyright (c) 1995-2004, 2007 Kungliga Tekniska H�gskolan
 * (Royal Institute of Technology, Stockholm, Sweden).
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
RCSID("$Id: output.c,v 1.105 2007/05/12 06:47:20 lha Exp $");
#endif

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <list.h>
#include <ctype.h>
#include <err.h>
#include <roken.h>
#include "sym.h"
#include "output.h"
#include "types.h"
#include "lex.h"

/*
 * The name of the current package that we're generating stubs for
 */

char *package = "";

/*
 * This is the list of packages so we know how to generate
 * all Execute_package().
 */

List *packagelist = NULL;

/*
 * Add this in front of the real functions implementing the server
 * functions called.
 */

char *prefix = "";

/*
 * File handles for the generated files themselves.
 */

ydr_file headerfile,
    clientfile,
    serverfile,
    clienthdrfile,
    serverhdrfile,
    ydrfile;

static long tmpcnt = 0;

/*
 * Function to convert error codes with.
 * (the default, the empty string, conveniently means no conversion.)
 */

char *error_function = "";

typedef enum { ENCODE_RX, DECODE_RX, ENCODE_MEM, DECODE_MEM } EncodeType;

typedef enum { CLIENT, SERVER } Side;

typedef enum { FDECL, VDECL } DeclType;

static void print_type (char *name, Type *type, enum argtype argtype, 
			DeclType decl, FILE *f);
static Bool print_entry (List *list, Listitem *item, void *i);
static void generate_hdr_struct (Symbol *s, FILE *f);
static void generate_hdr_enum (Symbol *s, FILE *f);
static void generate_hdr_const (Symbol *s, FILE *f);
static void generate_hdr_typedef (Symbol *s, FILE *f);
static int sizeof_type (Type *type);
static int sizeof_symbol (Symbol *);
static void encode_type (char *name, Type *type, FILE *f, 
			 EncodeType encodetype, Side side);
static void display_type (char *where, char *name, Type *type, FILE *f);
static Bool encode_struct_entry (List *list, Listitem *item, void *arg);
static Bool encode_union_entry (List *list, Listitem *item, void *arg);
static void encode_struct (Symbol *s, char *name, FILE *f, 
			   EncodeType encodetype, Side side);
static void encode_enum (Symbol *s, char *name, FILE *f,
			 EncodeType encodetype, Side side);
static void encode_typedef (Symbol *s, char *name, FILE *f, 
			    EncodeType encodetype, Side side);
static void encode_symbol (Symbol *s, char *name, FILE *f, 
			   EncodeType encodetype, Side side);
static void print_symbol (char *where, Symbol *s, char *name, FILE *f);
static void free_symbol (char *where, Symbol *s, char *name, FILE *f);
static void free_type (char *where, char *name, Type *type, FILE *f);

static void
print_type (char *name, Type *type, enum argtype argtype, 
	    DeclType decl, FILE *f)
{
     switch (type->type) {
	  case YDR_TCHAR :
	       fprintf (f, "char %s", name);
	       break;
	  case YDR_TUCHAR :
	       fprintf (f, "unsigned char %s", name);
	       break;
	  case YDR_TSHORT :
	       fprintf (f, "int16_t %s", name);
	       break;
	  case YDR_TUSHORT :
	       fprintf (f, "uint16_t %s", name);
	       break;
	  case YDR_TLONG :
	       fprintf (f, "int32_t %s", name);
	       break;
	  case YDR_TULONG :
	       fprintf (f, "uint32_t %s", name);
	       break;
	  case YDR_TLONGLONG:
	       fprintf (f, "int64_t %s", name);
	       break;
	  case YDR_TULONGLONG:
	       fprintf (f, "uint64_t %s", name);
	       break;
	  case YDR_TSTRING :
	       if (type->size && decl == VDECL)
		   fprintf (f, "char %s[%d]", name, type->size);
	       else if (argtype != TIN && type->size == 0)
		   fprintf (f, "char **%s", name);
	       else
		   fprintf (f, "char *%s", name);
	       break;
	  case YDR_TPOINTER :
	  {
	       char *tmp;

	       tmp = (char *)emalloc (strlen(name) + 2);
	       *tmp = '*';
	       strcpy (tmp+1, name);
	       print_type (tmp, type->subtype, argtype, decl, f);
	       free (tmp);
	       break;
	  }
	  case YDR_TUSERDEF : 
	       switch(type->symbol->type) {
	       case YDR_TSTRUCT:
	       case YDR_TUNION:
		    fprintf (f, "struct %s %s", type->symbol->name, name);
		    break;
	       default:
		    fprintf (f, "%s %s", type->symbol->name, name);
		    break;
	       }
	       break;
	  case YDR_TVARRAY :
	  {
	       char *s;

	       s = (char *)emalloc (strlen (name) + 6);
	       *s = '*';
	       strcpy (s + 1, name);
	       strcat (s, "_len");

	       fprintf (f, "struct {\n");
	       if (type->indextype)
		    print_type ("len", type->indextype, argtype, decl, f);
	       else
		    fprintf (f, "unsigned %s", "len");
	       fprintf (f, ";\n");
	       strcpy(s + strlen(s) - 3, "val");
	       print_type ("*val", type->subtype, argtype, decl, f);
	       fprintf (f, ";\n} %s", name);
	       free(s);
	       break;
	  }
	  case YDR_TARRAY :
	       print_type (name, type->subtype, argtype, decl, f);
	       fprintf (f, "[ %d ]", type->size);
	       break;
	  case YDR_TOPAQUE :
	       fprintf (f, "char %s", name);
	       break;
	  default :
	       abort();
     }
}

static Bool
print_entry (List *list, Listitem *item, void *i)
{
     StructEntry *s = (StructEntry *)listdata (item);
     FILE *f = (FILE *)i;

     fprintf (f, "     ");
     print_type (s->name, s->type, TIN, VDECL, f);
     fprintf (f, ";\n");
     return FALSE;
}

/*
 * Return the size of this type in bytes.
 * In the case of a variable-sized type, return -1 (unknown) or
 * the negative maxsize.
 */

static Bool
sizeof_struct_iter (List *list, Listitem *item, void *arg)
{
     int *tot = (int *)arg;
     StructEntry *s = (StructEntry *)listdata (item);
     int sz;

     sz = sizeof_type (s->type);
     if (sz == -1) {
	  *tot = -1;
	  return TRUE;
     } else if (sz < 0 || *tot < 0) {
	  *tot = -1 * (abs(*tot) + abs(sz));
	  return FALSE;
     } else {
	  *tot = *tot + sz;
	  return FALSE;
     }
}

static int
sizeof_struct (Symbol *s)
{
     int tot = 0;

     if (s->u.list)
	 listiter (s->u.list, sizeof_struct_iter, &tot);
     return tot;
}

/*
 *
 */

static Bool
sizeof_union_iter (List *list, Listitem *item, void *arg)
{
     int *m = (int *)arg;
     StructEntry *s = (StructEntry *)listdata (item);
     int sz;

     sz = sizeof_type (s->type);
     if (sz == -1) {
	  *m = -1;
	  return TRUE;
     }
     if (abs(sz) > *m)
	 *m = abs(sz);
     return FALSE;
}

static int
sizeof_union (Symbol *s)
{
     int tot = 0;

     if (s->u.list)
	 listiter (s->u.list, sizeof_union_iter, &tot);
     if (tot == -1)
	 return -1;
     return - tot - 4;
}


static int
sizeof_type (Type *t)
{
     switch (t->type) {
     case YDR_TCHAR :
     case YDR_TUCHAR :
     case YDR_TSHORT :
     case YDR_TUSHORT :
     case YDR_TLONG :
     case YDR_TULONG :
	  return 4;
     case YDR_TLONGLONG :
     case YDR_TULONGLONG :
	  return 8;
     case YDR_TSTRING :
	 if (t->size == 0)
	     return -1;
	 else
	     return t->size;
     case YDR_TOPAQUE :
	  return 1;
     case YDR_TUSERDEF :
	  return sizeof_symbol (t->symbol);
     case YDR_TARRAY :
     {
	  int sz = sizeof_type (t->subtype);

	  if (sz == -1)
	       return -1;
	  return t->size * sz;
     }
     case YDR_TVARRAY :
     {
	 int sz;
	 if (t->size == 0)
	     return -1;
	 sz = sizeof_type(t->subtype);
	 if (sz == -1)
	     return -1;
	 return -1 * (t->size * abs(sz) + 4); /* 4 is size of the var part */
     }
     case YDR_TPOINTER :
	  return -1;
     default :
	  abort ();
     }
}

static int
sizeof_symbol (Symbol *s)
{
     switch (s->type) {
	  case YDR_TUNDEFINED :
	       fprintf (stderr, "What is %s doing in sizeof_type?\n", s->name);
	       return 0;
	  case YDR_TSTRUCT :
	       return sizeof_struct (s);
          case YDR_TUNION:
	       return sizeof_union (s);
	  case YDR_TENUM :
	       return 4;
	  case YDR_TCONST :
	       return 0;
	  case YDR_TENUMVAL :
	       return 0;
	  case YDR_TTYPEDEF :
	       return sizeof_type (s->u.type);
	  default :
	       abort ();
     }
}

/*
 * Generate header contents
 */

static void
generate_hdr_struct (Symbol *s, FILE *f)
{
     fprintf (f, "struct %s {\n", s->name);
     if (s->u.list)
	 listiter (s->u.list, print_entry, f);
     fprintf (f, "};\ntypedef struct %s %s;\n", s->name, s->name);
}

static void
generate_hdr_union (Symbol *s, FILE *f)
{
     fprintf (f, "struct %s {\n", s->name);
     fprintf (f, "int %s;\n", s->unionlabel);
     fprintf (f, "union {\n");
     if (s->u.list)
	 listiter (s->u.list, print_entry, f);
     fprintf (f, "} u;\n");
     fprintf (f, "};\ntypedef struct %s %s;\n", s->name, s->name);
}

static void
generate_hdr_enum (Symbol *s, FILE *f)
{
     Listitem *item;
     Symbol *e;

     fprintf (f, "enum %s {\n", s->name);
     if (s->u.list) {
	 for (item = listhead (s->u.list); 
	      item && listnext (s->u.list, item); 
	      item = listnext (s->u.list, item))
	 {
	     e = (Symbol *)listdata (item);
	     
	     fprintf (f, "     %s = %d,\n", e->name, e->u.val);	     
	 }
	 e = (Symbol *)listdata (item);
	 fprintf (f, "     %s = %d\n};\n", e->name, e->u.val);
     }
     fprintf (f, "typedef enum %s %s;\n",
	      s->name, s->name);
}

static void
generate_hdr_const (Symbol *s, FILE *f)
{
     fprintf (f, "#define %s %d\n", s->name, s->u.val);
}

static void
generate_hdr_typedef (Symbol *s, FILE *f)
{
     fprintf (f, "typedef ");
     print_type (s->name, s->u.type, TIN, VDECL, f);
     fprintf (f, ";\n");
}

void
generate_sizeof (Symbol *s, FILE *f)
{
     int sz;

     if (s->type == YDR_TCONST)
	 return;

     sz = sizeof_symbol (s);
     if (sz == -1) {
	 ; /* the size is unknown */
     } else {
	  char *name, *ms = "";
	  if (sz < 0)
	      ms = "MAX_";

	  name = estrdup (s->name);
	  fprintf (f, "#define %s_%sSIZE %d\n", strupr (name), ms, abs(sz));
	  free (name);
     }
}

void
generate_header (Symbol *s, FILE *f)
{
     switch (s->type) {
	  case YDR_TUNDEFINED :
	       fprintf (f, "What is %s doing in generate_header?", s->name);
	       break;
	  case YDR_TSTRUCT :
	       generate_hdr_struct (s, f);
	       break;
          case YDR_TUNION:
	       generate_hdr_union (s, f);
	       break;
	  case YDR_TENUM :
	       generate_hdr_enum (s, f);
	       break;
	  case YDR_TCONST :
	       generate_hdr_const (s, f);
	       break;
	  case YDR_TENUMVAL :
	       break;
	  case YDR_TTYPEDEF :
	       generate_hdr_typedef (s, f);
	  default :
	       break;
     }
     putc ('\n', f);
}

/*
 * Generate functions for encoding and decoding.
 */

static char *
encode_function (Type *type, EncodeType encodetype)
{
     if (type->flags & TASIS)
	  return "";
     else if (encodetype == ENCODE_RX || encodetype == ENCODE_MEM) {
	 switch (type->type) {
	 case YDR_TLONGLONG:
	 case YDR_TULONGLONG:
	     return "htobe64";
	 default:
	     return "htonl";
	     break;
	 }
     } else if (encodetype == DECODE_RX || encodetype == DECODE_MEM) {
	 switch (type->type) {
	 case YDR_TLONGLONG:
	 case YDR_TULONGLONG:
	     return "be64toh";
	 default:
	     return "ntohl";
	     break;
	 }
     } else
	  abort();
}

/*
 * encode/decode long/longlong
 */

static void
encode_int_type (char *name, char *tname, Type *type, 
		 FILE *f, EncodeType encodetype)
{
     switch (encodetype) {
	  case ENCODE_RX :
	       fprintf (f, "{ u%s u;\n"
			"u = %s (%s);\n"
			"if(rx_Write(call, &u, sizeof(u)) != sizeof(u))\n"
			"goto fail;\n"
			"}\n",
			tname,
			encode_function (type, encodetype),
			name);
	       break;
	  case DECODE_RX :
	       fprintf (f, "{ u%s u;\n"
			"if(rx_Read(call, &u, sizeof(u)) != sizeof(u))\n"
			"goto fail;\n"
			"%s = %s (u);\n"
			"}\n", tname, name,
			encode_function (type, encodetype));
	       break;
	  case ENCODE_MEM :
	       fprintf (f, "{ %s tmp = %s(%s); "
			"if (*total_len < sizeof(tmp)) goto fail;\n"
			"memcpy (ptr, (char*)&tmp, sizeof(tmp)); "
			"ptr += sizeof(tmp); "
			"*total_len -= sizeof(tmp);}\n",
			tname,
			encode_function (type, encodetype),
			name);
	       break;
	  case DECODE_MEM :
	       fprintf (f, "{ %s tmp; "
			"if (*total_len < sizeof(tmp)) goto fail;"
			"memcpy ((char*)&tmp, ptr, sizeof(tmp)); "
			"%s = %s(tmp); "
			"ptr += sizeof(tmp); "
			"*total_len -= sizeof(tmp);}\n", 
			tname,
			name,
			encode_function (type, encodetype));
	       break;
	  default :
	       abort ();
     }
}

static void
encode_long (char *name, Type *type, FILE *f, EncodeType encodetype)
{
    encode_int_type(name, "int32_t", type, f, encodetype);
}

static void
encode_longlong (char *name, Type *type, FILE *f, EncodeType encodetype)
{
    encode_int_type(name, "int64_t", type, f, encodetype);
}

/*
 * print long
 */ 

static void
print_long (char *where, char *name, Type *type, FILE *f)
{
    fprintf (f, "printf(\" %s = %%d\", %s%s);", name, where, name);
}

/*
 * print longlong
 */ 

static void
print_longlong (char *where, char *name, Type *type, FILE *f)
{
    fprintf (f, "printf(\" %s = %%d\", (int32_t)%s%s);", name, where, name);
}

/*
 *
 */

static void
gen_check_overflow(Type *indextype, Type *subtype, char *num, FILE *f)
{
     /* Check if we will overflow */
     fprintf(f, "{\n");
     print_type ("overI", indextype, TIN, VDECL, f);
     fprintf(f, ";\n");
     fprintf(f, "overI = ((");
     print_type ("", indextype, TIN, VDECL, f);
     fprintf(f, ")~((");
     print_type ("", indextype, TIN, VDECL, f);
     fprintf(f, ")0) >> 1) / sizeof(");
     print_type ("", subtype, TIN, VDECL, f);
     fprintf(f, ");\n");
     fprintf(f, "if (overI < %s) goto fail;\n", num);
     fprintf(f, "}\n");
}

/*
 *
 */

static void __attribute__ ((unused))
encode_char (char *name, Type *type, FILE *f, EncodeType encodetype)
{
     switch (encodetype) {
	  case ENCODE_RX :
	       fprintf (f,
			"if(rx_Write(call, &%s, sizeof(%s)) != sizeof(%s))\n"
			"goto fail;\n",
			name, name, name);
	       break;
	  case DECODE_RX :
	       fprintf (f,
			"if(rx_Read(call, &%s, sizeof(%s)) != sizeof(%s))\n"
			"goto fail;\n",
			name, name, name);
	       break;
	  case ENCODE_MEM :
	       fprintf (f, "{ if (*total_len < sizeof(char)) goto fail;\n"
			"*((char *)ptr) = %s; "
			"ptr += sizeof(char); *total_len -= sizeof(char);}\n",
			name);
	       break;
	  case DECODE_MEM :
	       fprintf (f, "{ if (*total_len < sizeof(char)) goto fail;\n"
			"%s = *((char *)ptr); "
			"ptr += sizeof(char); *total_len -= sizeof(char);}\n",
			name);
	       break;
	  default :
	       abort ();
     }
}

static void __attribute__ ((unused))
encode_short (char *name, Type *type, FILE *f, EncodeType encodetype)
{
     switch (encodetype) {
	  case ENCODE_RX :
	       fprintf (f, "{ int16_t u;\n"
			"u = %s (%s);\n"
			"if(rx_Write(call, &u, sizeof(u)) != sizeof(u))\n"
			"goto fail;\n"
			"}\n", 
			encode_function (type, encodetype),
			name);
	       break;
	  case DECODE_RX :
	       fprintf (f, "{ int16_t u;\n"
			"if(rx_Read(call, &u, sizeof(u)) != sizeof(u))\n"
	                "goto fail;\n"
			"%s = %s (u);\n"
			"}\n", name,
			encode_function (type, encodetype));
	       break;
	  case ENCODE_MEM :
	  fprintf (f, "{ in16_t tmp = %s(%s); "
	  	      "if (*total_len < sizeof(int16_t)) goto fail;\n"
	              "memcpy (ptr, (char*)&tmp, sizeof(int16_t)); "
	              "ptr += sizeof(int16_t); "
	              "*total_len -= sizeof(int16_t);}\n", 
	                encode_function (type, encodetype),
			name);
	       break;
	  case DECODE_MEM :
	       fprintf (f, "{ int16_t tmp; "
			"if (*total_len < sizeof(int16_t)) goto fail;\n"
			"memcpy ((char *)&tmp, ptr, sizeof(int16_t)); "
			"%s = %s(tmp); "
			"ptr += sizeof(int16_t); "
			"*total_len -= sizeof(int16_t); }\n", 
			name,
			encode_function (type, encodetype));
	       break;
	  default :
	       abort ();
     }
}

/*
 * encode/decode TSTRING
 */

static void
encode_string (char *name, Type *type, FILE *f, EncodeType encodetype,
	       Side side)
{
     Type lentype = {YDR_TULONG};
     char *nname;

     asprintf (&nname, "(%s%s)",
	       ((type->size == 0) && side == CLIENT 
		&& (encodetype == ENCODE_RX || encodetype == ENCODE_MEM))
	       ? "*" : "", name);

     switch (encodetype) {
	  case ENCODE_RX :
	       fprintf (f, "{ unsigned len;\n"
			"char zero[4] = {0, 0, 0, 0};\n"
			"unsigned padlen;\n"
			"len = strlen(%s);\n"
			"padlen = (4 - (len %% 4)) %% 4;\n",
			name);
	       encode_type ("len", &lentype, f, encodetype, side);
	       fprintf (f,
			"if(rx_Write(call, %s, len) != len)\n"
			"goto fail;\n"
			"if(rx_Write(call, zero, padlen) != padlen)\n"
			"goto fail;\n"
			"}\n", name);
	       break;
	  case DECODE_RX :
	       fprintf (f, "{ unsigned len;\n"
			"unsigned padlen;\n"
			"char zero[4] = {0, 0, 0, 0};\n");
	       encode_type ("len", &lentype, f, encodetype, side);
	       if (type->size != 0) {
		   fprintf (f,
			    "if (len >= %u) {\n"
			    "rx_SetCallError(call, ENOMEM);\n"
			    "goto fail;\n"
			    "}\n",
			    type->size);
	       } else {
		   fprintf(f, "if (len == (uint32_t) -1) {\n"
			   "rx_SetCallError(call, ENOMEM);\n"
			   "goto fail;\n"
			   "}\n");
		   fprintf(f, "%s = malloc(len + 1);\n"
			   "if (%s == NULL) {\n"
			   "rx_SetCallError(call, ENOMEM);\n"
			   "goto fail;\n"
			   "}\n", 
			   nname, nname);
	       }

	       fprintf (f, 
			"if(rx_Read(call, %s, len) != len)\n"
			"goto fail;\n"
			"%s[len] = '\\0';\n"
			"padlen = (4 - (len %% 4)) %% 4;\n"
			"if(rx_Read(call, zero, padlen) != padlen)\n"
			"goto fail;\n"
			"}\n", nname, nname);
	       break;
	  case ENCODE_MEM :
	       fprintf (f,
			"{\nunsigned len = strlen(%s);\n"
			"if (*total_len < len) goto fail;\n"
			"*total_len -= len;\n",
			name);
	       encode_type ("len", &lentype, f, encodetype, side);
	       fprintf (f, "strncpy (ptr, %s, len);\n", name);
	       fprintf (f, "ptr += len + (4 - (len %% 4)) %% 4;\n"
			"*total_len -= len + (4 - (len %% 4)) %% 4;\n}\n");
	       break;
	  case DECODE_MEM :
	       fprintf (f,
		   "{\nunsigned len;\n");
	       encode_type ("len", &lentype, f, encodetype, side);
	       fprintf (f,
			"if (*total_len < len) goto fail;\n"
			"*total_len -= len;\n");
	       if (type->size != 0) {
		   fprintf (f,
			    "if(len >= %u)\n"
			    "goto fail;\n",
			    type->size);
	       } else {
		   fprintf(f, "if (len == (uint32_t) -1) {\n"
			   "goto fail;\n"
			   "}\n");
		   fprintf(f, "%s = malloc(len + 1);\n"
			   "if (%s == NULL) {\n"
			   "goto fail;\n"
			   "}\n", 
			   nname, nname);
	       }
	       fprintf (f,
			"memcpy (%s, ptr, len);\n"
			"%s[len] = '\\0';\n"
			"ptr += len + (4 - (len %% 4)) %% 4;\n"
			"*total_len -= len + (4 - (len %% 4)) %% 4;\n}\n",
			nname, nname);
	       break;
	  default :
	       abort ();
     }
     free (nname);
}	       

/*
 * print TSTRING
 */

static void
print_string (char *where, char *name, Type *type, FILE *f)
{
    fprintf (f, "/* printing TSTRING %s%s */\n", where, name);
    fprintf (f, "printf(\" %s = %%s\", %s%s);", name, where, name);
}

#if 0
/*
 * free TSTRING
 */

static void
free_string (char *where, char *name, Type *type, FILE *f)
{
    fprintf (f, "free(%s%s);\n", where, name);
}
#endif

/*
 * encode/decode TARRAY 
 */

static void
encode_array (char *name, Type *type, FILE *f, EncodeType encodetype,
	      Side side)
{
     if (type->subtype->type == YDR_TOPAQUE) {
	  if (type->size % 4 != 0)
	       error_message (1, "Opaque array should be"
			      "multiple of 4");
	  switch (encodetype) {
	       case ENCODE_RX :
		    fprintf (f,
			     "if(rx_Write (call, %s, %d) != %d)\n"
			     "goto fail;",
			     name, type->size, type->size);
		    break;
	       case DECODE_RX :
		    fprintf (f,
			     "if(rx_Read (call, %s, %d) != %d)\n"
			     "goto fail;",
			     name, type->size, type->size);
		    break;
	       case ENCODE_MEM :
		    fprintf (f, "if (*total_len < %u) goto fail;\n"
			     "memcpy (ptr, %s, %u);\n", type->size, name,
			     type->size);
		    fprintf (f, "ptr += %u; *total_len -= %u;\n", 
			     type->size, type->size);
		    break;
	       case DECODE_MEM :
		    fprintf (f, "if (*total_len < %u) goto fail;"
			     "memcpy (%s, ptr, %u);\n", type->size, name,
			     type->size);
		    fprintf (f, "ptr += %u; *total_len -= %u;\n", 
			     type->size, type->size);
		    break;
	       default :
		    abort ();
	  }
     } else {
	  char tmp[256];

	  fprintf (f, "{\nint i%lu;\nfor(i%lu = 0; i%lu < %u;"
		   "++i%lu){\n", tmpcnt, tmpcnt, tmpcnt, type->size,tmpcnt);
	  snprintf(tmp, sizeof(tmp)-1, "%s[i%lu]", name, tmpcnt);
	  tmpcnt++;
	  if (type->flags)
	      type->subtype->flags |= type->flags;
	  encode_type (tmp , type->subtype, f, encodetype, side);
	  tmpcnt--;
	  fprintf (f, "}\n}\n");
     }
}

/*
 * print ARRAY
 */

static void
print_array (char *where, char *name, Type *type, FILE *f)
{
    fprintf (f, "{\nunsigned int i%lu;\n", tmpcnt);

    fprintf (f, "/* printing ARRAY %s%s */\n", where, name);

    if (type->subtype->type == YDR_TOPAQUE) {
	if (type->size % 4 != 0)
	    error_message (1, "print_array: Opaque array should be"
			   "multiple of 4");
	
	fprintf (f, "char *ptr = %s%s;\n", where, name);
	fprintf (f, "printf(\"0x\");");
	fprintf (f, "for (i%lu = 0; i%lu < %d; ++i%lu)\n"
		 "printf(\"%%x\", ptr[i%lu]);",
		 tmpcnt, tmpcnt,
		 type->size, tmpcnt, tmpcnt);

     } else {
	char *ptr;
	fprintf (f, "for (i%lu = 0; i%lu < %d; ++i%lu) {\n", 
		 tmpcnt, tmpcnt, type->size, tmpcnt);
	asprintf(&ptr, "%s%s[i%ld]", where, name, tmpcnt);
	tmpcnt++;
	display_type (ptr, "", type->subtype, f);
	tmpcnt--;
	free(ptr);
	fprintf (f, "\nif (i%lu != %d - 1) printf(\",\");\n", 
		 tmpcnt, type->size);
	
	fprintf (f, "}\n");
     }
    fprintf (f, "}\n");
}

/*
 * free TARRAY
 */

static void
free_array (char *where, char *name, Type *type, FILE *f)
{
    if (type->subtype->type == YDR_TOPAQUE) {
	; /* nothing */
    } else {
	char tmp[256];

	fprintf (f, "{\nint i%lu;\nfor(i%lu = 0; i%lu < %u;"
		 "++i%lu){\n", tmpcnt, tmpcnt, tmpcnt, type->size,tmpcnt);
	snprintf(tmp, sizeof(tmp)-1, "%s[i%lu]", name, tmpcnt);
	tmpcnt++;
	if (type->flags)
	    type->subtype->flags |= type->flags;
	free_type (tmp, "", type->subtype, f);
	tmpcnt--;
	fprintf (f, "}\n}\n");
    }
}

/*
 * encode/decode TVARRAY
 */

static void
encode_varray (char *name, Type *type, FILE *f, EncodeType encodetype,
	       Side side)
{
     char tmp[256];
     Type lentype = {YDR_TULONG};
     Type *indextype;
	       
     strcpy (tmp, name);
     strcat (tmp, ".len");

     indextype = type->indextype ? type->indextype : &lentype;

     encode_type (tmp, indextype, f, encodetype, side);
     if (encodetype == DECODE_MEM || encodetype == DECODE_RX) {
	 if (type->size != 0)
	     fprintf (f, "if (%s > %d) goto fail;\n", tmp, type->size);
	 if (encodetype == DECODE_MEM) {
	     fprintf (f, "if ((%s * sizeof(", tmp);
	     print_type ("", type->subtype, TIN, VDECL, f);
	     fprintf (f, ")) > *total_len) goto fail;\n");
	 }
     }
     gen_check_overflow(indextype, type->subtype, tmp, f);
     if (encodetype == DECODE_MEM || encodetype == DECODE_RX) {
	 fprintf (f, "%s.val = (", name);
	 print_type ("*", type->subtype, TIN, VDECL, f);
	 fprintf (f, ")malloc(sizeof(");
	 print_type ("", type->subtype, TIN, VDECL, f);
	 fprintf (f, ") * %s);\n", tmp);
	 fprintf (f, "if (%s.val == NULL) goto fail;\n", name);
     }
     if (type->subtype->type == YDR_TOPAQUE) {
	  switch (encodetype) {
	       case ENCODE_RX :
		    fprintf (f, "{\n"
			     "char zero[4] = {0, 0, 0, 0};\n"
			     "unsigned padlen = (4 - (%s %% 4)) %% 4;\n"
			     "if(rx_Write (call, %s.val, %s) != %s)\n"
			     "goto fail;\n"
			     "if(rx_Write (call, zero, padlen) != padlen)\n"
			     "goto fail;\n"
			     "}\n",
			     tmp, name, tmp, tmp);
		    break;
	       case DECODE_RX :
		    fprintf (f, "{\n"
			     "char zero[4] = {0, 0, 0, 0};\n"
			     "unsigned padlen = (4 - (%s %% 4)) %% 4;\n"
			     "if(rx_Read (call, %s.val, %s) != %s)\n"
			     "goto fail;\n"
			     "if(rx_Read (call, zero, padlen) != padlen)\n"
			     "goto fail;\n"
			     "}\n",
			     tmp, name, tmp, tmp);
		    break;
	       case ENCODE_MEM :
		    fprintf (f, "{\n"
			     "char zero[4] = {0, 0, 0, 0};\n"
			     "size_t sz = %s + (4 - (%s %% 4)) %% 4;\n"
			     "if (*total_len < sz) goto fail;\n"
			     "memcpy (ptr, %s.val, %s);\n"
			     "memcpy (ptr + %s, zero, (4 - (%s %% 4)) %% 4);\n"
			     "ptr += sz; *total_len -= sz;\n"
			     "}\n",
			     tmp, tmp, name, tmp, tmp, tmp);
		    break;
	       case DECODE_MEM :
		    fprintf (f, 
			     "{\n"
			     "memcpy (%s.val, ptr, %s);\n"
			     "ptr += %s + (4 - (%s %% 4)) %% 4;\n"
			     "}\n",
			     name, tmp, tmp, tmp);
		    break;
	       default :
		    abort ();
	  }
     } else {
	  fprintf (f, "{\nint i%lu;\nfor(i%lu = 0; i%lu < %s;"
		   "++i%lu){\n", tmpcnt, tmpcnt, tmpcnt, tmp, tmpcnt);
	  snprintf(tmp, sizeof(tmp)-1, "%s.val[i%lu]", name, tmpcnt);
	  tmpcnt++;
	  if (type->flags)
	      type->subtype->flags |= type->flags;
	  encode_type (tmp , type->subtype, f, encodetype, side);
	  tmpcnt--;
	  fprintf (f, "}\n}\n");
     }
}

/*
 * print TVARRAY
 */

static void
print_varray (char *where, char *name, Type *type, FILE *f)
{
    fprintf (f, "{\nunsigned int i%lu;\n", tmpcnt);

    fprintf (f, "/* printing YDR_TVARRAY %s%s */\n", where, name);

    if (type->subtype->type == YDR_TOPAQUE) {
	fprintf (f, "char *ptr = %s%s.val;\n", where, name);
	fprintf (f, "printf(\"0x\");");
	fprintf (f, "for (i%lu = 0; i%lu < %s%s.len; ++i%lu)\n"
		 "printf(\"%%x\", ptr[i%lu]);",
		 tmpcnt, tmpcnt,
		 where, name, tmpcnt, tmpcnt);
    } else {
	char *ptr;
	fprintf (f, "for (i%lu = 0; i%lu < %s%s.len; ++i%lu) {\n", 
		 tmpcnt, tmpcnt, where, name, tmpcnt);
	asprintf(&ptr, "%s%s.val[i%ld]", where, name, tmpcnt);
	tmpcnt++;
	display_type (ptr, "", type->subtype, f);
	tmpcnt--;
	free(ptr);
	fprintf (f, "\nif (i%lu != %s%s.len - 1) printf(\",\");\n", 
		 tmpcnt, where, name);
	
	fprintf (f, "}\n");
    }
    fprintf (f, "}\n");
}

/*
 * free TVARRAY
 */

static void
free_varray (char *where, char *name, Type *type, FILE *f)
{
    if (type->subtype->type != YDR_TOPAQUE) {
	char *ptr;
	fprintf (f, "{\n"
		 "unsigned int i%lu;\n", tmpcnt);
	fprintf (f, "for (i%lu = 0; i%lu < %s%s.len; ++i%lu) {\n", 
		 tmpcnt, tmpcnt, where, name, tmpcnt);
	asprintf(&ptr, "%s%s.val[i%ld]", where, name, tmpcnt);
	tmpcnt++;
	free_type (ptr, "", type->subtype, f);
	tmpcnt--;
	free(ptr);
	fprintf (f, "}\n");
	fprintf (f, "}\n");
    }
    fprintf (f, "free((%s%s).val);\n", where, name);
}

/*
 * encode/decode pointer
 */

static void
encode_pointer (char *name, Type *type, FILE *f, EncodeType encodetype,
		Side side)
{
     Type booltype = {YDR_TULONG};
     char tmp[256];

     sprintf (tmp, "*(%s)", name);

     switch(encodetype) {
     case ENCODE_RX:
	  abort ();
     case ENCODE_MEM:
	  fprintf(f, "{ unsigned bool;\n"
		  "bool = %s != NULL;\n", name);
	  encode_type ("bool", &booltype, f, encodetype, side);
	  fprintf (f, "if(%s) {\n", name);
	  encode_type (tmp, type->subtype, f, encodetype, side);
	  fprintf (f, "}\n"
		   "}\n");
	  break;
     case DECODE_RX:
	  abort();
     case DECODE_MEM:
	  fprintf(f, "{ unsigned bool;\n");
	  encode_type ("bool", &booltype, f, encodetype, side);
	  fprintf (f, "if(bool) {\n");
	  fprintf (f, "%s = malloc(sizeof(%s));\n"
		   "if (%s == NULL) return ENOMEM;\n", 
		   name, tmp, name);
	  encode_type (tmp, type->subtype, f, encodetype, side);
	  fprintf (f, "} else {\n"
		   "%s = NULL;\n"
		   "}\n"
		   "}\n", name);
	  break;
     default:
	  abort ();
     }
}

/*
 * free pointer
 */

static void
free_pointer (char *where, char *name, Type *type, FILE *f)
{
    char *tmp;

    asprintf (&tmp, "*(%s%s)", where, name);
    fprintf (f, "if(%s%s)", where, name);
    free_type(tmp, "", type->subtype, f);
    free(tmp);
}

/*
 * encode type
 */

static void
encode_type (char *name, Type *type, FILE *f, EncodeType encodetype,
	     Side side)
{
     switch (type->type) {
	  case YDR_TCHAR :
	  case YDR_TUCHAR :
	  case YDR_TSHORT :
	  case YDR_TUSHORT :
	  case YDR_TLONG :
	  case YDR_TULONG :
	       encode_long (name, type, f, encodetype);
	       break;
	  case YDR_TLONGLONG :
	  case YDR_TULONGLONG :
	       encode_longlong (name, type, f, encodetype);
	       break;
	  case YDR_TSTRING :
	       encode_string (name, type, f, encodetype, side);
	       break;
	  case YDR_TOPAQUE :
	       error_message (1,
			      "Type opaque only allowed as part of an array");
	       break;
	  case YDR_TUSERDEF :
	       encode_symbol (type->symbol, name, f, encodetype, side);
	       break;
	  case YDR_TARRAY :
	       encode_array (name, type, f, encodetype, side);
	       break;
	  case YDR_TVARRAY :
	       encode_varray (name, type, f, encodetype, side);
	       break;
	  case YDR_TPOINTER :
	       encode_pointer (name, type, f, encodetype, side);
	       break;
	  default :
	       abort();
	  }
}

/*
 * print type
 */

static void
display_type (char *where, char *name, Type *type, FILE *f)
{
    assert (where);

    switch (type->type) {
    case YDR_TCHAR :
    case YDR_TUCHAR :
    case YDR_TSHORT :
    case YDR_TUSHORT :
    case YDR_TLONG :
    case YDR_TULONG :
	print_long (where, name, type, f);
	break;
    case YDR_TLONGLONG :
    case YDR_TULONGLONG :
	print_longlong (where, name, type, f);
	break;
    case YDR_TSTRING :
	print_string (where, name, type, f);
	break;
    case YDR_TOPAQUE :
	fprintf (f, "printf(\"printing TOPAQUE\\n\");");
	break;
    case YDR_TUSERDEF :
	print_symbol (where, type->symbol, name, f);
	break;
    case YDR_TARRAY :
	print_array (where, name, type, f);
	break;
    case YDR_TVARRAY :
	print_varray (where, name, type, f);
	break;
    case YDR_TPOINTER :
	fprintf (f, "printf(\"printing TPOINTER\\n\");");
	break;
    default :
	abort();
    }
}

/*
 * free type
 */

static void
free_type (char *where, char *name, Type *type, FILE *f)
{
    switch (type->type) {
    case YDR_TCHAR :
    case YDR_TUCHAR :
    case YDR_TSHORT :
    case YDR_TUSHORT :
    case YDR_TLONG :
    case YDR_TULONG :
    case YDR_TLONGLONG :
    case YDR_TULONGLONG :
	break;
    case YDR_TSTRING :
#if 0
	free_string (where, name, type, f);
#endif
	break;
    case YDR_TOPAQUE :
	break;
    case YDR_TUSERDEF :
	free_symbol (where, type->symbol, name, f);
	break;
    case YDR_TARRAY :
	free_array (where, name, type, f);
	break;
    case YDR_TVARRAY :
	free_varray (where, name, type, f);
	break;
    case YDR_TPOINTER :
	free_pointer (where, name, type, f);
	break;
    default :
	abort();
    }
}

struct context {
     char *name;
     FILE *f;
     Symbol *symbol;
     EncodeType encodetype;
     Side side;
};

/*
 * helpfunction for encode_struct
 */

static Bool
encode_struct_entry (List *list, Listitem *item, void *arg)
{
     StructEntry *s = (StructEntry *)listdata (item);
     char tmp[256];
     struct context *context = (struct context *)arg;

     strcpy (tmp, context->name);
     strcat (tmp, ".");
     strcat (tmp, s->name);

     if (s->type->type == YDR_TPOINTER
	 && s->type->subtype->type == YDR_TUSERDEF
	 && s->type->subtype->symbol->type == YDR_TSTRUCT
	 && strcmp(s->type->subtype->symbol->name,
		   context->symbol->name) == 0) {
	 fprintf (context->f,
		  "ptr = ydr_encode_%s(%s, ptr);\n",
		  context->symbol->name,
		  tmp);
     } else {
	 encode_type (tmp, s->type, context->f, context->encodetype,
		      context->side);
     }

     return FALSE;
}

/*
 * encode/decode TSTRUCT
 */

static void
encode_struct (Symbol *s, char *name, FILE *f, EncodeType encodetype,
	       Side side)
{
     struct context context;

     context.name       = name;
     context.symbol     = s;
     context.f          = f;
     context.encodetype = encodetype;
     context.side       = side;

     if (s->u.list)
	 listiter (s->u.list, encode_struct_entry, (void *)&context);
}

/*
 * helpfunction for encode_union_struct
 */

static Bool
encode_union_entry (List *list, Listitem *item, void *arg)
{
     StructEntry *s = (StructEntry *)listdata (item);
     char *tmp;
     struct context *context = (struct context *)arg;

     asprintf(&tmp, "%s.u.%s", context->name, s->name);

     fprintf (context->f, "case %d: /* %s */\n", s->num, s->name);
     if (context->encodetype == ENCODE_MEM) {
	 fprintf (context->f, "ptr = ydr_encode_%s(&%s, ptr, total_len);\n",
		  s->type->symbol->name, tmp);
	 fprintf (context->f, "if (ptr == NULL) goto fail;\nbreak;\n");
     } else if (context->encodetype == ENCODE_RX){
	 abort();
     } else if (context->encodetype == DECODE_MEM) {
	 fprintf (context->f, "ptr = ydr_decode_%s(&%s, ptr, total_len);\n",
		  s->type->symbol->name, tmp);
	 fprintf (context->f, "if (ptr == NULL) goto fail;\nbreak;\n");
     } else if (context->encodetype == DECODE_RX){
	 abort();
     }
     free(tmp);

     return FALSE;
}

/*
 * encode/decode TUNION
 */

static void
encode_union (Symbol *s, char *name, FILE *f, EncodeType encodetype,
	       Side side)
{
     struct context context;
     Type type = {YDR_TLONG};
     char *str;

     fprintf(f, "/* encode/decode %s*/\n", s->unionlabel);
     asprintf(&str, "%s.%s", name, s->unionlabel);
     encode_type (str, &type, f, encodetype, side);

     fprintf(f, "switch(%s) {\n", str);
     free(str);

     context.name       = name;
     context.symbol     = s;
     context.f          = f;
     context.encodetype = encodetype;
     context.side       = side;

     if (s->u.list)
	 listiter (s->u.list, encode_union_entry, (void *)&context);

     fprintf(f, "default:\n");
     fprintf(f, "goto fail;\n");
     fprintf(f, "}\n");
}


/*
 * help function for print_struct
 */

struct printcontext {
    char *where;
    char *name;
    FILE *f;
    Symbol *symbol;
};

static Bool
print_structentry (List *list, Listitem *item, void *arg)
{
     StructEntry *s = (StructEntry *)listdata (item);
     struct printcontext *context = (struct printcontext *)arg;

     char *tmp;
     char *tmp2;

     asprintf(&tmp, ".%s", s->name);
     asprintf(&tmp2, "%s%s", context->where, context->name);

     if (s->type->type == YDR_TPOINTER
	 && s->type->subtype->type == YDR_TUSERDEF
	 && s->type->subtype->symbol->type == YDR_TSTRUCT
	 && s->type->subtype->symbol->type == YDR_TUNION
	 && strcmp(s->type->subtype->symbol->name,
		   context->symbol->name) == 0) {
	 fprintf (context->f,
		  "ydr_print_%s%s(%s%s, ptr);\n",
		  package,
		  context->symbol->name,
		  tmp2,
		  tmp);
     } else {
	 display_type (tmp2, tmp, s->type, context->f);
     }

     free(tmp);
     free(tmp2);

     fprintf (context->f, "\n");

     return FALSE;
}

/*
 * print TSTRUCT
 */

static void
print_struct (char *where, Symbol *s, char *name, FILE *f)
{
    struct printcontext context;
    
    context.name       = name;
    context.symbol     = s;
    context.f          = f;
    context.where      = where ;

    fprintf (f, "/* printing TSTRUCT %s%s */\n", where, name);
    
    if (s->u.list)
	listiter (s->u.list, print_structentry, (void *)&context);
}

/*
 * print TUNION
 */

static void
print_union (char *where, Symbol *s, char *name, FILE *f)
{
#if 0
    struct printcontext context;
    
    context.name       = name;
    context.symbol     = s;
    context.f          = f;
    context.where      = where ;
#endif
    fprintf (f, "/* printing TUNION %s%s */\n", where, name);
}

/*
 * help function for free_struct
 */

struct freecontext {
    char *where;
    char *name;
    FILE *f;
    Symbol *symbol;
};

static Bool
free_structentry (List *list, Listitem *item, void *arg)
{
     StructEntry *s = (StructEntry *)listdata (item);
     struct freecontext *context = (struct freecontext *)arg;

     char *tmp;
     char *tmp2;

     asprintf(&tmp, ".%s", s->name);
     asprintf(&tmp2, "%s%s", context->where, context->name);

     if (s->type->type == YDR_TPOINTER
	 && s->type->subtype->type == YDR_TUSERDEF
	 && s->type->subtype->symbol->type == YDR_TSTRUCT
	 && strcmp(s->type->subtype->symbol->name,
		   context->symbol->name) == 0) {
	 fprintf (context->f,
		  "ydr_free_%s%s(%s%s, ptr);\n",
		  package,
		  context->symbol->name,
		  tmp2,
		  tmp);
     } else {
	 free_type (tmp2, tmp, s->type, context->f);
     }

     free(tmp);
     free(tmp2);

     return FALSE;
}

/*
 * free TSTRUCT
 */

static void
free_struct (char *where, Symbol *s, char *name, FILE *f)
{
    struct freecontext context;
    
    context.name       = name;
    context.symbol     = s;
    context.f          = f;
    context.where      = where;

    if (s->u.list)
	listiter (s->u.list, free_structentry, (void *)&context);
}

/*
 * free TUNION
 */

static Bool
free_union_entry (List *list, Listitem *item, void *arg)
{
     StructEntry *s = (StructEntry *)listdata (item);
     struct freecontext *context = (struct freecontext *)arg;
     char *tmp;

     asprintf(&tmp, "%s.u.%s", context->name, s->name);

     fprintf (context->f, "case %d: /* %s */\n", s->num, s->name);
     fprintf (context->f, "ydr_free_%s(&%s);\n",
		  s->type->symbol->name, tmp);
     free(tmp);
     return FALSE;
}

static void
free_union (char *where, Symbol *s, char *name, FILE *f)
{
    struct freecontext context;
    
    fprintf(f, "switch(%s.%s) {\n", name, s->unionlabel);

    context.name       = name;
    context.symbol     = s;
    context.f          = f;
    context.where      = where;

    if (s->u.list)
	listiter (s->u.list, free_union_entry, (void *)&context);

     fprintf(f, "}\n");
}


/*
 * encode/decode TENUM
 */

static void
encode_enum (Symbol *s, char *name, FILE *f, EncodeType encodetype,
	     Side side)
{
     Type type = {YDR_TLONG};

     encode_type (name, &type, f, encodetype, side);
}

/*
 * print TENUM
 */

static Bool
gen_printenum (List *list, Listitem *item, void *arg)
{
    Symbol *s = (Symbol *)listdata (item);
    FILE *f = (FILE *)arg;
    
    fprintf (f, "case %d:\n"
	     "printf(\"%s\");\n"
	     "break;\n", 
	     (int) s->u.val, s->name);

     return FALSE;
}

static void
print_enum (char *where, Symbol *s, char *name, FILE *f)
{
    fprintf (f, "/* print ENUM %s */\n", where);

    fprintf (f, "printf(\"%s = \");", name);
    fprintf (f, "switch(%s) {\n", where);
    if (s->u.list)
	listiter (s->u.list, gen_printenum, f);
    fprintf (f, 
	     "default:\n"
	     "printf(\" unknown enum %%d\", %s);\n"
	     "}\n",
	     where);
}

/*
 * encode/decode TTYPEDEF
 */

static void
encode_typedef (Symbol *s, char *name, FILE *f, EncodeType encodetype,
		Side side)
{
     encode_type (name, s->u.type, f, encodetype, side);
}

/*
 * print TTYPEDEF
 */

static void
print_typedef (char *where, Symbol *s, char *name, FILE *f)
{
    display_type (where, name, s->u.type, f);
}

/*
 * free TTYPEDEF
 */

static void
free_typedef (char *where, Symbol *s, char *name, FILE *f)
{
    free_type (where, name, s->u.type, f);
}

/*
 * Encode symbol/TUSERDEF
 */

static void
encode_symbol (Symbol *s, char *name, FILE *f, EncodeType encodetype,
	       Side side)
{
     switch (s->type) {
	  case YDR_TSTRUCT :
	       encode_struct (s, name, f, encodetype, side);
	       break;
	  case YDR_TUNION :
	       encode_union (s, name, f, encodetype, side);
	       break;
	  case YDR_TENUM :
	       encode_enum (s, name, f, encodetype, side);
	       break;
	  case YDR_TTYPEDEF :
	       encode_typedef (s, name, f, encodetype, side);
	       break;
	  default :
	       abort();
	  }
}

/*
 * print symbol/TUSERDEF
 */

static void
print_symbol (char *where, Symbol *s, char *name, FILE *f)
{
     switch (s->type) {
	  case YDR_TSTRUCT :
	       print_struct (where, s, name, f);
	       break;
	  case YDR_TUNION :
	       print_union (where, s, name, f);
	       break;
	  case YDR_TENUM :
	       print_enum (where, s, name, f);
	       break;
	  case YDR_TTYPEDEF :
	       print_typedef (where, s, name, f);
	       break;
	  default :
	       abort();
	  }
}

/*
 * Generate a free function for symbol
 */

static void
free_symbol (char *where, Symbol *s, char *name, FILE *f)
{
     switch (s->type) {
	  case YDR_TSTRUCT :
	       free_struct (where, s, name, f);
	       break;
	  case YDR_TUNION :
	       free_union (where, s, name, f);
	       break;
	  case YDR_TENUM :
	       break;
	  case YDR_TTYPEDEF :
	       free_typedef (where, s, name, f);
	       break;
	  default :
	       abort();
	  }
}

/*
 * Generate the definition of an encode/decode function.
 */

static void
generate_function_definition (Symbol *s, FILE *f, Bool encodep)
{
     if (s->type == YDR_TSTRUCT
	 || s->type == YDR_TUNION
	 || s->type == YDR_TENUM
	 || s->type == YDR_TTYPEDEF)
     {
	  fprintf (f, 
		   "%schar *ydr_%scode_%s(%s%s *o, %schar *ptr, size_t *total_len)",
		   encodep ? "" : "const ",
		   encodep ? "en" : "de",
		   s->name,
		   encodep ? "const " : "",
		   s->name,
		   encodep ? "" : "const ");
     } else if (s->type == YDR_TCONST
		|| s->type == YDR_TENUMVAL 
		|| s->type == YDR_TTYPEDEF)
	  ;
     else
	  error_message (1, "What is %s (type %d) doing here?\n",
			 s->name, s->type);
}

/*
 * Generate the definition of a print function.
 */

static void
generate_printfunction_definition (Symbol *s, FILE *f)
{
     if (s->type == YDR_TSTRUCT
	 || s->type == YDR_TUNION
	 || s->type == YDR_TENUM
	 || s->type == YDR_TTYPEDEF) 
     {
	  fprintf (f, 
		   "void ydr_print_%s(%s *o)",
		   s->name, s->name);
     } else if (s->type == YDR_TCONST
		|| s->type == YDR_TENUMVAL 
		|| s->type == YDR_TTYPEDEF)
	  ;
     else
	  error_message (1, "What is %s (type %d) doing here?\n",
			 s->name, s->type);
}

/*
 * Generate a definition for a function to free `s', writing it to `f'
 */

static void
generate_freefunction_definition (Symbol *s, FILE *f)
{
     if (s->type == YDR_TSTRUCT
	 || s->type == YDR_TUNION
	 || s->type == YDR_TENUM
	 || s->type == YDR_TTYPEDEF) 
     {
	  fprintf (f, 
		   "void ydr_free_%s(%s *o)",
		   s->name, s->name);
     } else if (s->type == YDR_TCONST
		|| s->type == YDR_TENUMVAL 
		|| s->type == YDR_TTYPEDEF)
	  ;
     else
	  error_message (1, "What is %s (type %d) doing here?\n",
			 s->name, s->type);
}

/*
 * Generate an encode/decode function
 */

void
generate_function (Symbol *s, FILE *f, Bool encodep)
{
     if (s->type == YDR_TSTRUCT
	 || s->type == YDR_TUNION
	 || s->type == YDR_TENUM
	 || s->type == YDR_TTYPEDEF)
     {
	  generate_function_definition (s, f, encodep);
	  fprintf (f, "\n{\n");
	  if (!encodep)
	      fprintf (f, "memset(o, 0, sizeof(*o));\n");
	  encode_symbol (s, "(*o)", f,
			 encodep ? ENCODE_MEM : DECODE_MEM, CLIENT);
	  fprintf (f, "return ptr;\n"
		   "fail:\n");
	  if (!encodep) {
	      if (s->type != YDR_TUNION)
		  free_symbol ("", s, "(*o)", f);
	  }
	  fprintf (f, "errno = EINVAL;\n"
		   "return NULL;}\n");
     } else if (s->type == YDR_TCONST
		|| s->type == YDR_TENUMVAL 
		|| s->type == YDR_TTYPEDEF)
	  ;
     else
	  error_message (1, "What is %s (type %d) doing here?\n",
			 s->name, s->type);
}

/*
 * Generate a print function
 */

void
generate_printfunction (Symbol *s, FILE *f)
{
     if (s->type == YDR_TSTRUCT
	 || s->type == YDR_TENUM
	 || s->type == YDR_TTYPEDEF) {
	  generate_printfunction_definition (s, f);
	  fprintf (f, "\n{\n");
	  print_symbol ("(*o)",  s, "", f);
	  fprintf (f, "return;\n}\n");
     } else if (s->type == YDR_TUNION) {
	  fprintf (f, "/* print function */\n");
     } else if (s->type == YDR_TCONST
		|| s->type == YDR_TENUMVAL 
		|| s->type == YDR_TTYPEDEF)
	  ;
     else
	  error_message (1, "What is %s (type %d) doing here?\n",
			 s->name, s->type);
}

/*
 * Generate a free function for the type `s' and print it on `f'.
 */

void
generate_freefunction (Symbol *s, FILE *f)
{
     if (s->type == YDR_TSTRUCT
	 || s->type == YDR_TENUM
	 || s->type == YDR_TTYPEDEF) {
	  generate_freefunction_definition (s, f);
	  fprintf (f, "\n{\n");
	  free_symbol("(*o)", s, "", f);
	  fprintf (f, "return;\n}\n");
     } else if (s->type == YDR_TUNION) {
	  generate_freefunction_definition (s, f);
	  fprintf (f, "\n{\n");
	  free_union("", s, "(*o)", f);
	  fprintf (f, "return;\n}\n");
     } else if (s->type == YDR_TCONST
		|| s->type == YDR_TENUMVAL 
		|| s->type == YDR_TTYPEDEF)
	  ;

     else
	  error_message (1, "What is %s (type %d) doing here?\n",
			 s->name, s->type);
}

/*
 * Generate an prototype for an encode/decode function
 */

void
generate_function_prototype (Symbol *s, FILE *f, Bool encodep)
{
     if (s->type == YDR_TSTRUCT
	 || s->type == YDR_TUNION
	 || s->type == YDR_TENUM
	 || s->type == YDR_TTYPEDEF) 
     {
	  generate_function_definition (s, f, encodep);
	  fprintf (f, ";\n");
     } else if (s->type == YDR_TCONST
		|| s->type == YDR_TENUMVAL 
		|| s->type == YDR_TTYPEDEF)
	  ;
     else
	  error_message (1, "What is %s (type %d) doing here?\n",
			 s->name, s->type);
}

/*
 * Generate an prototype for a print function
 */

void
generate_printfunction_prototype (Symbol *s, FILE *f)
{
     if (s->type == YDR_TSTRUCT
	 || s->type == YDR_TUNION
	 || s->type == YDR_TENUM
	 || s->type == YDR_TTYPEDEF) {
	  generate_printfunction_definition (s, f);
	  fprintf (f, ";\n");
     } else if (s->type == YDR_TCONST
		|| s->type == YDR_TENUMVAL 
		|| s->type == YDR_TTYPEDEF)
	  ;
     else
	  error_message (1, "What is %s (type %d) doing here?\n",
			 s->name, s->type);
}

/*
 * Generate a prototype for a free function for the `s' type
 * and output it to `f'
 */

void
generate_freefunction_prototype(Symbol *s, FILE *f)
{
     if (s->type == YDR_TSTRUCT
	 || s->type == YDR_TUNION
	 || s->type == YDR_TENUM
	 || s->type == YDR_TTYPEDEF) {
	  generate_freefunction_definition (s, f);
	  fprintf (f, ";\n");
     } else if (s->type == YDR_TCONST
		|| s->type == YDR_TENUMVAL 
		|| s->type == YDR_TTYPEDEF)
	  ;
     else
	  error_message (1, "What is %s (type %d) doing here?\n",
			 s->name, s->type);
}

static Bool
gen1 (List *list, Listitem *item, void *arg)
{
     Argument *a = (Argument *)listdata (item);
     FILE *f = (FILE *)arg;

     if ((a->argtype == TOUT || a->argtype == TINOUT)
	 && a->type->type != YDR_TPOINTER
	 && a->type->type != YDR_TSTRING)
	 error_message (1, "Argument %s is OUT and not pointer or string.\n",
			a->name);
     fprintf (f, ", ");
     if (a->argtype == TIN)
	 fprintf (f, "const ");
     print_type (a->name, a->type, a->argtype, FDECL, f);
     fprintf (f, "\n");
     return FALSE;
}

static Bool
genin (List *list, Listitem *item, void *arg)
{
     Argument *a = (Argument *)listdata (item);
     FILE *f = (FILE *)arg;

     if (a->argtype == TIN || a->argtype == TINOUT) {
	  fprintf (f, ", %s ", a->argtype == TIN ? "const" : "");
	  print_type (a->name, a->type, a->argtype, FDECL, f);
	  fprintf (f, "\n");
     }
     return FALSE;
}

static Bool
genout (List *list, Listitem *item, void *arg)
{
     Argument *a = (Argument *)listdata (item);

     if (a->argtype == TOUT || a->argtype == TINOUT)
	  return gen1 (list, item, arg);
     else
	  return FALSE;
}

static Bool
gendeclare (List *list, Listitem *item, void *arg)
{
     Argument *a = (Argument *)listdata (item);
     FILE *f = (FILE *)arg;

     if (a->type->type == YDR_TPOINTER)
	  print_type (a->name, a->type->subtype, TIN, VDECL, f);
     else
	  print_type (a->name, a->type, TIN, VDECL, f);
     fprintf (f, ";\n");
     return FALSE;
}

static Bool
genzero (List *list, Listitem *item, void *arg)
{
     Argument *a = (Argument *)listdata (item);
     FILE *f = (FILE *)arg;

     fprintf (f, "memset(&%s, 0, sizeof(%s));\n",
	      a->name, a->name);
     return FALSE;
}

static Bool
genfree_isarrayp(Type *type)
{
    if (type->type == YDR_TVARRAY)
	return TRUE;
    if (type->type == YDR_TPOINTER)
	return genfree_isarrayp(type->subtype);
    if (type->type == YDR_TUSERDEF &&
	type->symbol &&
	type->symbol->type == YDR_TTYPEDEF)
	return genfree_isarrayp(type->symbol->u.type);
    
    return FALSE;
}


static Bool
genfree (List *list, Listitem *item, void *arg)
{
     Argument *a = (Argument *)listdata (item);
     FILE *f = (FILE *)arg;

     if (genfree_isarrayp(a->type))
	 fprintf(f, "free(%s.val);\n", a->name);
     else if (a->argtype != TIN
	      && a->type->type == YDR_TSTRING && a->type->size == 0)
	 fprintf (f, "free(%s);\n", a->name);
     return FALSE;
}

static Bool
genencodein (List *list, Listitem *item, void *arg)
{
    Argument *a = (Argument *)listdata (item);
    FILE *f = (FILE *)arg;
    
    if (a->argtype == TIN || a->argtype == TINOUT) {
	if (a->type->type == YDR_TPOINTER) {
	    char *tmp = (char *)emalloc (strlen (a->name) + 4);
	    
	    sprintf (tmp, "(*%s)", a->name);
	    
	    encode_type (tmp, a->type->subtype, f, ENCODE_RX, CLIENT);
	    free (tmp);
	} else
	    encode_type (a->name, a->type, f, ENCODE_RX, CLIENT);
    }
    return FALSE;
}

static Bool
gendecodeout (List *list, Listitem *item, void *arg)
{
     Argument *a = (Argument *)listdata (item);
     FILE *f = (FILE *)arg;

     if (a->argtype == TOUT || a->argtype == TINOUT) {
	 if (a->type->type == YDR_TPOINTER) {
	     char *tmp = (char *)emalloc (strlen (a->name) + 4);
	       
	     sprintf (tmp, "(*%s)", a->name);

	     encode_type (tmp, a->type->subtype, f, DECODE_RX, CLIENT);
	     free (tmp);
	 } else if(a->type->type == YDR_TSTRING) {
	     encode_type (a->name, a->type, f, DECODE_RX, CLIENT);
	 }
     }
     return FALSE;
}

static Bool
gendecodein (List *list, Listitem *item, void *arg)
{
     Argument *a = (Argument *)listdata (item);
     FILE *f = (FILE *)arg;

     if (a->argtype != TIN && a->argtype != TINOUT)
	  return TRUE;
     else {
	  if (a->type->type == YDR_TPOINTER) {
#if 0
	       char *tmp = (char *)emalloc (strlen (a->name) + 4);
	       
	       sprintf (tmp, "(*%s)", a->name);

	       encode_type (tmp, a->type->subtype, f, DECODE_RX, SERVER);
	       free (tmp);
#endif
	       encode_type (a->name, a->type->subtype, f, DECODE_RX, SERVER);
	  } else
	       encode_type (a->name, a->type, f, DECODE_RX, SERVER);
	  return FALSE;
     }
}

static Bool
genencodeout (List *list, Listitem *item, void *arg)
{
     Argument *a = (Argument *)listdata (item);
     FILE *f = (FILE *)arg;

     if (a->argtype == TOUT || a->argtype == TINOUT) {
	  if (a->type->type == YDR_TPOINTER)
	       encode_type (a->name, a->type->subtype, f, ENCODE_RX, SERVER);
	  else
	       encode_type (a->name, a->type, f, ENCODE_RX, SERVER);
     }
     return FALSE;
}

static Bool
findargtypeiter (List *list, Listitem *item, void *arg)
{
     Argument *a = (Argument *)listdata (item);
     int *type = (int *)arg;

     if (a->argtype == *type) {
	 (*type)++;
	 return TRUE;
     }
     return FALSE;
}

static Bool
findargtype(List *list, int type)
{
    int savedtype = type;
    listiter(list, findargtypeiter, &type);
    if (type != savedtype)
	return TRUE;
    return FALSE;
}

static Bool
genargs (List *list, Listitem *item, void *arg)
{
     Argument *a = (Argument *)listdata (item);
     FILE *f = (FILE *)arg;

     if (a->type->type == YDR_TPOINTER
	 || (a->argtype != TIN
	     && a->type->type == YDR_TSTRING && a->type->size == 0))
	  putc ('&', f);
     fputs (a->name, f);
     if (listnext (list, item))
	  fprintf (f, ", ");
     return FALSE;
}

/*
 * Generate the stub functions for this RPC call
 */

static void
generate_simple_stub (Symbol *s, FILE *f, FILE *headerf)
{
     Type type = {YDR_TLONG};
     char *op;

     fprintf (headerf, "int %s%s(\nstruct rx_connection *connection\n",
	      package, s->name);
     listiter (s->u.proc.arguments, gen1, headerf);
     fprintf (headerf, ");\n\n");

     fprintf (f, "int %s%s(\nstruct rx_connection *connection\n",
	      package, s->name);
     listiter (s->u.proc.arguments, gen1, f);
     fprintf (f, ")\n{\n"
	      "struct rx_call *call;\n"
	      "int ret = 0;\n"
	      "call = rx_NewCall (connection);\n");

     asprintf (&op, "%u", s->u.proc.id);
     
     encode_type (op, &type, f, ENCODE_RX, CLIENT);
     free (op);
     listiter (s->u.proc.arguments, genencodein, f);
     listiter (s->u.proc.arguments, gendecodeout, f);
     fprintf (f,
	      "return %s(rx_EndCall (call,0));\n"
	      "fail:\n"
	      "ret = %s(rx_GetCallError(call));\n"
	      "rx_EndCall (call, 0);\n"
	      "return ret;\n"
	      "}\n",
	      error_function,
	      error_function);
}

static void
generate_split_stub (Symbol *s, FILE *f, FILE *headerf)
{
     Type type = {YDR_TLONG};
     char *op;

     fprintf (headerf, "int Start%s%s(\nstruct rx_call *call\n",
	      package, s->name);
     listiter (s->u.proc.arguments, genin, headerf);
     fprintf (headerf, ");\n\n");

     fprintf (f, "int Start%s%s(\nstruct rx_call *call\n",
	      package, s->name);
     listiter (s->u.proc.arguments, genin, f);
     fprintf (f, ")\n{\n");


     asprintf (&op, "%u", s->u.proc.id);
     encode_type (op, &type, f, ENCODE_RX, CLIENT);
     free (op);
     listiter (s->u.proc.arguments, genencodein, f);
     fprintf (f, "return 0;\n");
     /* XXX only in arg */
     if (findargtype(s->u.proc.arguments, TIN) || 
	 findargtype(s->u.proc.arguments, TINOUT))
	 fprintf (f, "fail:\n"
                 "return %s(rx_GetCallError(call));\n",
		  error_function);

     fprintf (f, "}\n\n");

     fprintf (headerf, "int End%s%s(\nstruct rx_call *call\n", 
	      package, s->name);
     listiter (s->u.proc.arguments, genout, headerf);
     fprintf (headerf, ");\n\n");

     fprintf (f, "int End%s%s(\nstruct rx_call *call\n", 
	      package, s->name);
     listiter (s->u.proc.arguments, genout, f);
     fprintf (f, ")\n{\n");

     listiter (s->u.proc.arguments, gendecodeout, f);
     fprintf (f, "return 0;\n");
     /* XXX only out arg */
     if (findargtype(s->u.proc.arguments, TOUT) || 
	 findargtype(s->u.proc.arguments, TINOUT))
	 fprintf (f, "fail:\n"
                 "return %s(rx_GetCallError(call));\n",
		  error_function);

     fprintf (f, "}\n\n");
}

struct gen_args {
    FILE *f;
    int firstp;
    int arg_type;
};

static Bool
genmacro (List *list, Listitem *item, void *arg)
{
     Argument *a = (Argument *)listdata (item);
     struct gen_args *args = (struct gen_args *)arg;

     if (a->argtype == args->arg_type || a->argtype == TINOUT) {
	 fprintf (args->f, "%s%s",
		  args->firstp ? "" : ", ", a->name);
	 args->firstp = 0;
     }

     return FALSE;
}

static void
generate_multi (Symbol *s, FILE *f)
{
    struct gen_args gen_args;

    fprintf (f, "\n#include <rx/rx_multi.h>");
    fprintf (f, "\n#define multi_%s%s(", package, s->name);
    gen_args.f        = f;
    gen_args.firstp   = 1;
    gen_args.arg_type = TIN;
    listiter (s->u.proc.arguments, genmacro, &gen_args);
    fprintf (f, ") multi_Body(");
    fprintf (f, "Start%s%s(multi_call", package, s->name);
    gen_args.f        = f;
    gen_args.firstp   = 0;
    gen_args.arg_type = TIN;
    listiter (s->u.proc.arguments, genmacro, &gen_args);
    fprintf (f, "), End%s%s(multi_call", package, s->name);
    gen_args.f        = f;
    gen_args.firstp   = 0;
    gen_args.arg_type = TOUT;
    listiter (s->u.proc.arguments, genmacro, f);
    fprintf (f, "))\n");
}

void
generate_client_stub (Symbol *s, FILE *f, FILE *headerf)
{
    if (s->u.proc.flags & TSPLIT)
	  generate_split_stub (s, f, headerf);
    if (s->u.proc.flags & TSIMPLE)
	  generate_simple_stub (s, f, headerf);
}

/*
 * A list of all the functions that are to be recognized by the
 * server, later used in generate_server_switch.
 */

static List *func_list;

static void
generate_standard_c_prologue (FILE *f,
			      const char *filename,
			      const char *basename)
{
     fprintf (f, "/* Generated from %s.xg */\n", basename);
     fprintf (f, "#include \"%s.h\"\n\n", basename);
     fprintf (f, "#ifndef _GNU_SOURCE\n");
     fprintf (f, "#define _GNU_SOURCE 1\n");
     fprintf (f, "#endif /* _GNU_SOURCE */\n");
     fprintf (f, "#include <sys/types.h>\n");
     fprintf (f, "#include <stdio.h>\n");
     fprintf (f, "#include <stdlib.h>\n");
     fprintf (f, "#include <string.h>\n");
     fprintf (f, "#include <netinet/in.h>\n");
     fprintf (f, "#include <errno.h>\n");
     fprintf (f, "#ifdef RCSID\n"
	      "RCSID(\"%s generated from %s.xg with $Id: output.c,v 1.105 2007/05/12 06:47:20 lha Exp $\");\n"
	      "#endif\n\n", filename, basename);
     fprintf(f, 
	     "/* crap for operating systems that don't "
	     "provide 64 bit ops */\n"
	     "#ifndef htobe64\n"
	     "#if BYTE_ORDER == LITTLE_ENDIAN\n"
	     "static inline uint64_t\n"
	     "ydr_swap64(uint64_t x)\n"
	     "{\n"
	     "#define LT(n) n##ULL\n"
	     "   return \n"
	     "      ((LT(0x00000000000000ff) & x) << 56) | \n"
	     "      ((LT(0x000000000000ff00) & x) << 40) | \n"
	     "      ((LT(0x0000000000ff0000) & x) << 24) | \n"
	     "      ((LT(0x00000000ff000000) & x) << 8) | \n"
	     "      ((LT(0x000000ff00000000) & x) >> 8) | \n"
	     "      ((LT(0x0000ff0000000000) & x) >> 24) | \n"
	     "      ((LT(0x00ff000000000000) & x) >> 40) | \n"
	     "      ((LT(0xff00000000000000) & x) >> 56) ; \n"
	     "#undef LT\n"
	     "}\n"
	     "\n"
	     "#define htobe64(x) ydr_swap64((x))\n"
	     "#endif /* BYTE_ORDER */\n"
	     "#if BYTE_ORDER == BIG_ENDIAN\n"
	     "#define be64toh(x) (x)\n"
	     "#define htobe64(x) (x)\n"
	     "#endif /* BYTE_ORDER */\n"
	     "#endif /* htobe64 */\n"
	     "#ifndef be64toh\n"
	     "#define be64toh(x) htobe64(x)\n"
	     "#endif /* be64toh */\n"
	 );
}

/*
 * Convert filename into a cpp symbol
 */

static char *
cppfilesymbolname(const char *fn)
{
     char *symname, *tmp;

     symname = estrdup(fn);
     estrdup (symname);
     for (tmp = symname; *tmp; tmp++) {
	 if (tmp == symname && isdigit((unsigned char)*tmp))
	     *tmp = '_';
	 if (!isalpha((unsigned char)*tmp))
	     *tmp = '_';
     }
     return symname;
}

/*
 * open all files
 */

void
init_generate (const char *filename)
{
     char *tmp;
     char *fileupr;

     func_list = listnew ();

     asprintf (&tmp, "%s.h", filename);
     if (tmp == NULL)
	 err (1, "malloc");
     ydr_fopen (tmp, "w", &headerfile);
     free (tmp);

     fileupr = cppfilesymbolname(filename);
     fprintf (headerfile.stream, "/* Generated from %s.xg */\n", filename);
     fprintf (headerfile.stream, "#ifndef _%s_\n"
	      "#define _%s_\n\n", fileupr, fileupr);
     fprintf (headerfile.stream, "#include <atypes.h>\n\n");
     free (fileupr);
     
     asprintf (&tmp, "%s.ydr.c", filename);
     if (tmp == NULL)
	 err (1, "malloc");
     ydr_fopen (tmp, "w", &ydrfile);
     generate_standard_c_prologue (ydrfile.stream, tmp, filename);
     free (tmp);

     asprintf (&tmp, "%s.cs.c", filename);
     if (tmp == NULL)
	 err (1, "malloc");
     ydr_fopen (tmp, "w", &clientfile);
     generate_standard_c_prologue (clientfile.stream, tmp, filename);
     fprintf (clientfile.stream, "#include \"%s.cs.h\"\n\n", filename);
     free (tmp);

     asprintf (&tmp, "%s.ss.c", filename);
     if (tmp == NULL)
	 err (1, "malloc");
     ydr_fopen (tmp, "w", &serverfile);
     generate_standard_c_prologue (serverfile.stream, tmp, filename);
     fprintf (serverfile.stream, "#include \"%s.ss.h\"\n\n", filename);
     free (tmp);

     asprintf (&tmp, "%s.cs.h", filename);
     if (tmp == NULL)
	 err (1, "malloc");
     ydr_fopen (tmp, "w", &clienthdrfile);
     free (tmp);
     fprintf (clienthdrfile.stream, "/* Generated from %s.xg */\n", filename);
     fprintf (clienthdrfile.stream, "#include <rx/rx.h>\n");
     fprintf (clienthdrfile.stream, "#include \"%s.h\"\n\n", filename);

     asprintf (&tmp, "%s.ss.h", filename);
     if (tmp == NULL)
	 err (1, "malloc");
     ydr_fopen (tmp, "w", &serverhdrfile);
     free (tmp);
     fprintf (serverhdrfile.stream, "/* Generated from %s.xg */\n", filename);
     fprintf (serverhdrfile.stream, "#include <rx/rx.h>\n");
     fprintf (serverhdrfile.stream, "#include \"%s.h\"\n\n", filename);

     packagelist = listnew();
     if (packagelist == NULL)
	 err (1, "init_generate: listnew: packagelist");
}

void
close_generator (const char *filename)
{
     char *fileupr;
	  
     fileupr = cppfilesymbolname(filename);
     fprintf (headerfile.stream, "\n#endif /* %s */\n", fileupr);
     free(fileupr);
     ydr_fclose (&headerfile);
     ydr_fclose (&clientfile);
     ydr_fclose (&serverfile);
     ydr_fclose (&clienthdrfile);
     ydr_fclose (&serverhdrfile);
     ydr_fclose (&ydrfile);
}

/*
 * Generate the server-side stub function for the function in s and
 * write it to the file f.
 */

void
generate_server_stub (Symbol *s, FILE *f, FILE *headerf, FILE *h_file)
{
     fprintf (headerf, "int S%s%s%s(\nstruct rx_call *call\n",
	      prefix, package, s->name);
     listiter (s->u.proc.arguments, gen1, headerf);
     fprintf (headerf, ");\n\n");

     fprintf (f, "static int ydr_ps_%s%s(\nstruct rx_call *call)\n",
	      package, s->name);
     fprintf (f, "{\n"
	      "int32_t _result;\n");
     listiter (s->u.proc.arguments, gendeclare, f);
     listiter (s->u.proc.arguments, genzero, f);
     fprintf (f, "\n");
     listiter (s->u.proc.arguments, gendecodein, f);
     fprintf (f, "_result = S%s%s%s(", prefix, package, s->name);
     if (/* s->u.proc.splitp */ 1) {
	  fprintf (f, "call");
	  if (!listemptyp (s->u.proc.arguments))
	       fprintf (f, ", ");
     }
     listiter (s->u.proc.arguments, genargs, f);
     fprintf (f, ");\n");
     fprintf (f, "if (_result) goto funcfail;\n");
     listiter (s->u.proc.arguments, genencodeout, f);
     listiter (s->u.proc.arguments, genfree, f);
     fprintf (f, "return _result;\n");
     if (!listemptyp(s->u.proc.arguments)) {
	 fprintf(f, "fail:\n");
	 listiter (s->u.proc.arguments, genfree, f);
	 fprintf(f, "return rx_GetCallError(call);\n");

     }
     fprintf(f, "funcfail:\n"
	     "return _result;\n"
	     "}\n\n");

     listaddtail (func_list, s);
     if (s->u.proc.flags & TMULTI)
	 generate_multi (s, h_file);
}

struct gencase_context {
    FILE *f;
    char *package;
};

static Bool
gencase (List *list, Listitem *item, void *arg)
{
     Symbol *s = (Symbol *)listdata (item);
     struct gencase_context *c = (struct gencase_context *)arg;
     FILE *f = c->f;

     if (c->package == s->u.proc.package) {
	 fprintf (f, "case %u: {\n"
		  "_result = ydr_ps_%s%s(call);\n"
		  "break;\n"
		  "}\n",
		  s->u.proc.id, s->u.proc.package, s->name);
     }
     return FALSE;
}

/*
 *
 */

void
generate_server_switch (FILE *c_file,
			FILE *h_file)
{
     Type optype = {YDR_TULONG};
     Listitem *li;
     struct gencase_context c;

     c.f = c_file;
     
     li = listhead (packagelist);
     while (li) {
	 c.package = (char *)listdata (li);

	 fprintf (h_file,
		  "int32_t %sExecuteRequest(struct rx_call *call);\n",
		  c.package);
	 
	 fprintf (c_file,
		  "int32_t %sExecuteRequest(struct rx_call *call)\n"
		  "{\n"
		  "unsigned opcode;\n"
		  "int32_t _result;\n",
		  c.package);
	 
	 encode_type ("opcode", &optype, c_file, DECODE_RX, SERVER);
	 fprintf (c_file, "switch(opcode) {\n");
	 
	 listiter (func_list, gencase, &c);
	 
	 fprintf (c_file, "default:\n"
 		  "_result = RXGEN_OPCODE;\n"
		  "}\n"
		  "return _result;\n"
		  "fail:\n"
		  "return rx_GetCallError(call);\n"
		  "}\n\n");

	 li = listnext (packagelist, li);
     }
}

/*
 *
 */

void
ydr_fopen (const char *name, const char *mode, ydr_file *f)
{
     int streamfd;

     asprintf (&f->curname, "%sXXXXXX", name);
     if (f->curname == NULL)
	 err (1, "malloc");

     streamfd = mkstemp(f->curname);
     if (streamfd < 0)
	 err (1, "mkstemp %s failed", f->curname);
     f->stream = fdopen (streamfd, mode);
     if (f->stream == NULL)
	 err (1, "open %s mode %s", f->curname, mode);
     f->newname = estrdup(name);
}

void
ydr_fclose (ydr_file *f)
{
     if (fclose (f->stream))
	 err (1, "close %s", f->curname);
     if (rename(f->curname, f->newname))
	 err (1, "rename %s, %s", f->curname, f->newname);
     free(f->curname);
     free(f->newname);
}
