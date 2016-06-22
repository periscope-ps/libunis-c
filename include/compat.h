// =============================================================================
//  periscope-ps (libunis-c)
//
//  Copyright (c) 2015-2016, Trustees of Indiana University,
//  All rights reserved.
//
//  This software may be modified and distributed under the terms of the BSD
//  license.  See the COPYING file for details.
//
//  This software was created at the Indiana University Center for Research in
//  Extreme Scale Technologies (CREST).
// =============================================================================
#ifndef COMPAT_H
#define COMPAT_H


#include <stdint.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/param.h>
#ifdef HAVE_MACHINE_ENDIAN_H
#include <machine/endian.h>
#else
#include <endian.h>
#endif

#ifdef HAVE_BYTESWAP_H
#include <byteswap.h>
#else
#ifdef HAVE_LIBKERN_OSBYTEORDER_H
#include <libkern/OSByteOrder.h>
#define bswap_16(x) OSSwapInt16(x)
#define bswap_32(x) OSSwapInt32(x)
#define bswap_64(x) OSSwapInt64(x)
#endif
#endif

#define TRUE 1
#define FALSE 0

int get_all_ips(char ***ret_ips, int *ret_ip_count);
char* Strdup(const char* source);
#endif
