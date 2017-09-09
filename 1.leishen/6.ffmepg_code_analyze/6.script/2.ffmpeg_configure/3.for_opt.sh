#!/bin/sh

#知识点:Shell 扩展模式匹配
#${param#pattern}	从param前面最小删除pattern的匹配
#optval="${opt#*=}" 的意思是从opt变量开始删除一个全部内容加等于号

#[root@web11 my_ffmpeg]# ./3.for_opt.sh --extra-ldflags=abc --n-a-m-e=y_b_x
#abc
#other
#n_a_m_e

CMDLINE_SET="set1 set2"
CMDLINE_APPEND="aaa bbb"

#两者是否相等
#[ $var = $value ] && return 0
is_in(){
	value=$1
	shift
	for var in $*; do
		[ $var = $value ] && return 0
	done
	return 1
}

#Parse Options
#Parse Options部分用于解析Configure的附加参数。该部分的代码如下所示。
#在这里需要注意，取出opt的值一般都是“--extra-ldflags=XXX”的形式，
#通过“${opt#*=}”截取获得“=”号后面的内容作为optval，
#对于“--extra-ldflags=XXX”来说，optval取值为“XXX”。
#然后根据opt种类的不同，以及optval取值的不同，分别作不同的处理。
for opt do
	optval="${opt#*=}"
	case "$opt" in
		--extra-ldflags=*)
		echo $optval
		;;

		*)
		echo 'other'
		#删除"="右边的内容
		optname="${opt%%=*}"
		#删除左边的“--”
		optname="${optname#--}"
		optname=$(echo "$optname" | sed 's/-/_/g')
		echo $optname

		#看看是否在opt列表中，不在的话就会返回错误
		#if is_in $optname $CMDLINE_SET; then
		if is_in $optname $CMDLINE_SET; then
			echo "in CMDLINE_SET"
			eval $optname='$optval'
		elif is_in $optname $CMDLINE_APPEND; then
			echo "in CMDLINE_APPEND"
			append $optname "$optval"
		else
			echo 'die!!!!'
		fi

		;;
	esac
done
