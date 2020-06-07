#!/bin/sh

scons RELEASE=1 && scons pk3 && mkdir -p dll && cd client && scons && cp cgame.mp.i386.so ../dll && cd ..
