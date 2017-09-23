#!/bin/bash
ffmpeg -f v4l2 -i /dev/video0 -vcodec libx264 -preset:v ultrafast -tune:v zerolatency -f h264 udp://233.233.233.223:6666                                                                            
