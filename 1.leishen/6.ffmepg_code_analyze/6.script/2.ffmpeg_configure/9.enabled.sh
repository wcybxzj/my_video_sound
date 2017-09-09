#!/bin/sh

die(){
	echo 'die!'
}

enabled(){
    test "${1#!}" = "$1" && op='=' || op=!=
    eval test "x\$${1#!}" $op "xyes"
}

die_license_disabled() {
    enabled $1 || { enabled $2 && die "$2 is $1 and --enable-$1 is not specified."; }
}

gpl=no
#gpl=yes
libcdio=yes

die_license_disabled gpl libcdio

