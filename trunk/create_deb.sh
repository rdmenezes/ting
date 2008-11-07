#!/bin/sh

dst="debian_package/usr/include/ting"

mkdir -p $dst

cp src/debug.hpp $dst
cp src/Exc.hpp $dst
cp src/gendef.hpp $dst
cp src/Ptr.hpp $dst
cp src/Thread.hpp $dst
cp src/ting.hpp $dst
cp src/types.hpp $dst

cp -r DEBIAN debian_package

dpkg -b debian_package libting-dev_0.2_i386.deb
