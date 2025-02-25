#!/bin/bash

LIBATSC3_SRC_DIR=`pwd`

sudo ../linux-build-deps

# Correctly link xlocale.h
sudo ln -s /usr/include/locale.h /usr/include/xlocale.h

# Compile libmicrohttpd
cd ../libmicrohttpd && tar -xvzf libmicrohttpd-latest.tar.gz && cd libmicrohttpd-0.9.63 && ./configure --prefix=`pwd`/build && make install

# Compile Bento4
cd ../../bento && mv lib lib-tmp && mkdir lib && mv lib-tmp/place_libap4.a_bento_file_here lib && mv lib-tmp/.gitignore lib && rm -Rf lib-tmp && cd lib && git clone https://github.com/axiomatic-systems/Bento4.git && cd Bento4 && mkdir cmakebuild && cd cmakebuild && cmake -DCMAKE_BUILD_TYPE=Debug .. && make && mv * ../..

cd $LIBATSC3_SRC_DIR

# Compile SRT
cd ../srt && ./configure --prefix `pwd`/build && make && make install

cd $LIBATSC3_SRC_DIR

cd ../openssl && KERNEL_BITS=64 ./Configure  no-asm -g3 -O0 -fno-omit-frame-pointer -fno-inline-functions --prefix=`pwd`/build_ssl --openssldir=`pwd`/build_ssl '-Wl,-rpath,$(LIBRPATH)'c && make && make install

cd $LIBATSC3_SRC_DIR

# Compile libatsc3
make clean && make all
