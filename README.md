libunis-c
=========
UNIS registration C library 

Building libunis-c library
-------------------------------------------------------------------------------
For building you need the following packages installed:

  - Curl Library (we tested with libcurl4-openssl-dev)
  - Jansson Library
  - Pthread


To build this library you should be able to unpack the tarball and then run configure with appropriate options as needed.
You can then run 'make' to build the library and 'make install' to install the library.

```
https://github.com/periscope-ps/libunis-c.git
cd libunis-c
./configure
make
make install
```

Run Example
-------------------------------------------------------------------------------
Make sure correct unis is being pointed and correct ssl settings are being used.
```
./build_example.sh
./example
```

Usage
-------------------------------------------------------------------------------
Take a look at example.c


