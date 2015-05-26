#!/bin/sh
set -e
autoreconf --force --install -I config || exit 1
rm -rf autom4te.cache

