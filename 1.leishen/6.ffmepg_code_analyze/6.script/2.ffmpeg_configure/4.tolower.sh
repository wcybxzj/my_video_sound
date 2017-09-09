#!/bin/sh

#[root@web11 my_ffmpeg]# ./3.tolower.sh
#ybx dabing

tolower(){
    echo "$@" | tr ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz
}


tolower YBX DABING
