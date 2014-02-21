rm -rf *.o *.so* example
gcc -DDEBUG -fPIC -g -c unis_registration.c -I. -I/usr/include
gcc -fPIC -g -c th-lock.c -I. -I/usr/include
gcc -fPIC -g -c curl_context.c -I.
gcc -shared -Wl,-soname,libtest.so -o libtest.so th-lock.o curl_context.o unis_registration.o  -L/usr/lib -lcurl -pthread -ljansson
gcc example.c -o example -g -I/usr/include -I. -L. -ltest -pthread
