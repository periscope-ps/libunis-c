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
#!/bin/bash

RPMROOT=~/rpmbuild

if [ -z "$1" ];then
   echo "You didn't specify anything to build";
   exit 1;
fi

tar zcf $RPMROOT/SOURCES/${1}.tar.gz ${1};

# build the package
rpmbuild -bb ${1}.spec
