AUTOMAKE_OPTIONS = foreign

CFLAGS=-Wall -I/usr/local/include -DVERSION=\"$(VERSION)\" -DPROG="\"$(PACKAGE)\"" -I.
LDFLAGS=-L/usr/local/lib -lcurl -pthread -ljansson

lib_LIBRARIES = libunis-c.a

libunis_c_a_SOURCES = curl_context.c	curl_context.h log.h  th-lock.c  th-lock.h	unis_registration.c  unis_registration.h
libunis_c_a_LIBADD =

AM_CFLAGS = -fPIC
INCLUDES =
