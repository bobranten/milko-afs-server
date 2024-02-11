

typedef struct key_stuff {
    krb5_cryptotype	ks_scrypto;	/* rx stream key */
    size_t		ks_overhead;
    size_t		ks_cksumsize;
} key_stuff;
