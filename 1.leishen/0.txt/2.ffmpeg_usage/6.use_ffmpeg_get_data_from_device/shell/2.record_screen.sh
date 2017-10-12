#!/bin/bash
#必须是在gnome才能成功 同时录像和录音
# x11grab表示是x11的界面 -i :0.0 表示偏移量
ffmpeg -f alsa -ac 2 -i pulse -f x11grab -r 30 -s 1024x768 -i :0.0 -acodec pcm_s16le -vcodec libx264 -preset ultrafast -crf 0 -threads 0 output.mkv
