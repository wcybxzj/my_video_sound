#!/bin/bash
ffmpeg -f x11grab -s 1600x900 -r 50 -vcodec libx264 -preset:v ultrafast -tune:v zerolatency -crf 18 -f mpegts udp://localhost:1234
