FUSE implementation for exnodes
===============================

Building libunis-c library
-------------------------------------------------------------------------------
For building you need the following packages installed:

  - Curl Library
  - Jansson Library
  - Pthread
  - fuse


To build this library run the following commands:
```
mkdir mnt
make clean
make
```

Run Example
-------------------------------------------------------------------------------

```
./exnode mnt
```

The exnode url is defined in "exnode_data.h" which can be modified as per the need of the user.
