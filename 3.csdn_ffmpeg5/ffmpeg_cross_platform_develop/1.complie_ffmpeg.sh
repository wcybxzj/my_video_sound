#!/bin/sh
cd /Users/ybx/soft/ffmpeg-3.3
rm -rf ./output
./configure --arch=x86_64 --target-os=darwin --disable-yasm --prefix=./output \
			--disable-ffmpeg --disable-ffplay --disable-doc --disable-ffprobe --disable-bzlib --disable-ffserver
make -j8 install
