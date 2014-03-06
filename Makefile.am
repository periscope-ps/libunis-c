AUTOMAKE_OPTIONS = foreign

CFLAGS=-Wall -I/usr/local/include -DVERSION=\"$(VERSION)\" -DPROG="\"$(PACKAGE)\"" -I.
LDFLAGS=-L/usr/local/lib -lcurl -lpthread -ljansson

#lib_LIBRARIES = libunis-c.a
lib_LTLIBRARIES = libunis-c.la

library_includedir = $(includedir)
library_include_HEADERS = unis_registration.h

libunis_c_la_SOURCES = curl_context.c  curl_context.h log.h  th-lock.c  th-lock.h  unis_registration.c  unis_registration.h  compat.c compat.h
libunis_c_la_LDFLAGS = -export-dynamic

AM_CFLAGS = -fPIC
ACLOCAL_AMFLAGS = -I m4
INCLUDES =
