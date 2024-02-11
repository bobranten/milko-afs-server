
Milko - a free and open source AFS server
-----------------------------------------

Milko is a free and open source server for the Andrew File System (AFS)


Background
----------

AFS is a world-wide distributed file system researched and developed at
Carnegie Mellon University and commercially marketed by Transarc Corporation
later acquired by IBM. In the early 2000s IBM decided to release the code
as open source and it became the OpenAFS project.

OpenAFS has been almost alone on the market for AFS implementations but in
the late 1990s the Arla project started at the Royal Insitute of Technology
in Stockholm, Sweden (by the same guys that created the independent Kerberos
implementation Heimdal) The goal of the Arla project was to create a free
and independent AFS client and server that was portable. Back in those days
it was aslo considered important to have implementations of Kerberos and
AFS outside of US because of export restrictions on cryptography.

The Arla project was for a while rather successful and developed an AFS
client that was stable and portable and also the beginning of an AFS
server that had basic features. However after 2007 the project went dead.
There are probably a number of reasons for this, among them that the
involved persons got jobs! Another reason is that a filesystem client
contains a kernelmode part and it takes very hard work to maintain and
keep it updated, even on Linux it is difficult for out of tree kernel
drivers to keep up with the changes to the mainline source tree. Because
of this the Arla project has been unmaintained for a number of years and
is not working on modern systems.

However after often thinking on the Arla project and considering starting
it again I came to the conclusion that it is the server part that is worth
keeping. A server is user mode software that is easy to maintain and port.
I think there is room on the free software market for an independent AFS
server that has a small and clean code base and I hope to atract developers
that want to help continuing this project.

Milko is a fork of the server part from the Arla project. So far I have
updated the build system to modern versions of GNU autoconfig and automake.
I have also updated the source code so it can be compiled on modern systems.

If you are an experineced AFS user you are welcome to start help testing
Milko. If you have not used AFS before you could start by installing a
precompiled package of OpenAFS on a Linux distrubition. Try both the client
and the server and then you can begin working with Milko.


Building Milko
--------------

To compile Milko you only need to have OpenSSL and either Heimdal or MIT
Kerberos installed in adition to the most basic build tools.
You build it with the commands:
```
./configure
make
make install
```
The default location is /usr/local but you can set the path prefix like this
if you want to install somewhere else:
```
./configure --prefix="/home/b/bosse/milko"
```


Whats installed
---------------

The installed software is:
```
/usr/local/etc: config files like CellSrvDB and ThisCell.
/usr/local/bin: bos, vos, pts and "sked".
/usr/local/libexec: bosserver, fileserver, ptserver and vldbserver.
/usr/man: man pages.
```


How to test
-----------

To test an AFS server you first need an AFS client installed on a second
computer, if you are running Linux there are precompiled packages of
OpenAFS called openafs-client on both Debian/Ubuntu based distributions
and on Red Hat. If you want to build OpenAFS from source you can find it
at http://openafs.org/ Also if you want to use a Macintosh as an AFS
client you can download a precompiled client based on OpenAFS from
https://www.auristor.com/openafs/client-installer/

When you have a working client you can start testing the server:

Read milko/README.milko and then you are on youre own...


Mailing list
------------

There is a mailing list to discuss this project at arla-drinkers@stacken.kth.se.


Bo Brantén
bosse@accum.se
___________________________________________________________________
- Arla in Swedish means `early'.  Most of the code has been written
early in the morning.
