#!/bin/sh
ldflags_filter=echo

append(){
    var=$1
    shift
    eval "$var=\"\$$var $*\""
}

append LDFLASG "aa bbb"

#aa bbb
echo $LDFLASG

add_ldflags(){
	append LDFLASG $($ldflags_filter "$@")
}

add_ldflags 111 222 333
#aa bbb 111 222 333
echo $LDFLASG

