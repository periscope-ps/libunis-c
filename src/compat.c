/*
 * There are two different licenses in use by this file.  The first one
 * is the classic 3-clause BSD license: (The old clause 3 has been removed,
 * pursant to ftp://ftp.cs.berkeley.edu/ucb/4bsd/README.Impt.License.Change
 *
 * * * *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE. 
 * * * *
 *
 * The second license is OpenBSD's ISC-like license, which is used for 
 * strlcpy() and strlcat().  See the license later on in the file.
 * 
 * Everthing else has had its copyright explicitly disclaimed by the author.
 */

#include "compat.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>
#include <errno.h>
#include <ifaddrs.h>
#ifndef JUNOS
#include <net/if.h>
#endif
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <unistd.h>
#include <limits.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <grp.h>
#include <pwd.h>

int _strlist_add(const char *str, char ***list, int *list_length) {
  char **new_list;
  char *new_str;

  new_str = strdup(str);
  if (!new_str)
    goto error_exit;

  new_list = realloc(*list, sizeof(char *) * (*list_length + 1));
  if (new_list == NULL)
    goto error_exit2;

  new_list[*list_length] = new_str;

  *list = new_list;
  *list_length = *list_length + 1;

  return 0;

error_exit2:
  free(new_str);
error_exit:
  return -1;
}

int get_all_ips(char ***ret_ips, int *ret_ip_count) {
	struct ifaddrs *ifaces, *curr;
	char **ips;
	int ip_count;

	if (getifaddrs(&ifaces) != 0) {
		fprintf(stderr, "Failed to get interfaces for host\n");
		return -1;
	}

	ips = NULL;
	ip_count = 0;

	for(curr = ifaces; curr != NULL; curr = curr->ifa_next) {
		char ip[255];
		size_t salen;

		if (!curr->ifa_addr)
			continue;

		if (curr->ifa_addr->sa_family == AF_INET)
			salen = sizeof (struct sockaddr_in);
		else if (curr->ifa_addr->sa_family == AF_INET6)
			salen = sizeof (struct sockaddr_in6);
		else
			continue;


		if (getnameinfo (curr->ifa_addr, salen, ip, sizeof (ip), NULL, 0, NI_NUMERICHOST) < 0) {
			continue;
		}

		if (_strlist_add(ip, &ips, &ip_count) != 0) {
			continue;
		}
	}

	*ret_ips = ips;
	*ret_ip_count = ip_count;

	return 0;
}

char* Strdup(const char* source) {
  if(source != NULL)
    return strdup(source);
  else
    return NULL;
}
