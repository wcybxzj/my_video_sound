#!/bin/sh


avcode=1

# $*为"1 2 3"（一起被引号包住）
# $@为"1" "2" "3"（分别被包住）
# $#为3（参数数量）
enable(){
    set_all yes $*
}

set_all(){
    value=$1
    shift
    for var in $*; do
        eval $var=$value
    done
}

#实际就是执行了avcode="yes"
enable avcode

echo $avcode
