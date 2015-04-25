
lib_LTLIBRARIES = libunis-c.la

AM_CFLAGS = -fPIC -Wall -I/usr/include  -I/usr/local/include  -I../include -DVERSION=\"$(VERSION)\" -DPROG="\"$(PACKAGE)\"" @LIBCURL_INC@ @LIBJANSSON_INC@
AM_LDFLAGS=@LIBS@ @LIBCURL_LIB@ @LIBJANSSON_LIB@

ACLOCAL_AMFLAGS = -I m4

libunis_c_la_SOURCES = curl_context.c libunis_c_log.c th-lock.c compat.c  unis_registration.c unis_exnode.c
libunis_c_la_LIBADD = @LIBS@ @LIBCURL_LIB@ @LIBJANSSON_LIB@
libunis_c_la_LDFLAGS = -export-dynamic

