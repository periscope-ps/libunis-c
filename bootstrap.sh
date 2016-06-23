# =============================================================================
#  periscope-ps (libunis-c)
#
#  Copyright (c) 2015-2016, Trustees of Indiana University,
#  All rights reserved.
#
#  This software may be modified and distributed under the terms of the BSD
#  license.  See the COPYING file for details.
#
#  This software was created at the Indiana University Center for Research in
#  Extreme Scale Technologies (CREST).
# =============================================================================
#!/bin/sh
set -e
autoreconf --force --install -I config || exit 1
rm -rf autom4te.cache

