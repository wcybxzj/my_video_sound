#!/bin/sh
#测试函数返回值做为if的条件

fun(){
	if [ $1 == 0 ]; then
		return 0
	else
		return 1
	fi
}

#fun 0
#fun 1

if fun 0 ; then
	echo 1
else
	echo 0
fi

if fun 1 ; then
	echo 1
else
	echo 0
fi
