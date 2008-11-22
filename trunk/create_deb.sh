#!/bin/sh

dst="debian_package/usr/include/ting"

mkdir -p $dst

cp src/*.hpp $dst

cp -r DEBIAN debian_package

dpkg -b debian_package libting-dev_0.2_i386.deb
