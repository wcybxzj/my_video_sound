#!/bin/sh

target=win32
#target=linux

exesuf() {
    case $1 in
        mingw32*|mingw64*|win32|win64|cygwin*|*-dos|freedos|opendos|os/2*|symbian) echo .exe ;;
    esac
}

EXESUF=$(exesuf $target)

#输出 .exe
echo $EXESUF
