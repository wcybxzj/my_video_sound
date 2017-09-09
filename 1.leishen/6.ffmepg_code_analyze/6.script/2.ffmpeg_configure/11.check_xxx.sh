#!/bin/sh
target_os=linux
cc=gcc
ld=gcc

logfile="config.log"
TMPFILES=$logfile

filter_cflags=echo
asflags_filter=echo
cflags_filter=echo
ldflags_filter=echo

AS_C='-c'
AS_O='-o $@'
CC_C='-c'
CC_E='-E -o $@'
CC_O='-o $@'
CXX_C='-c'
CXX_O='-o $@'
OBJCC_C='-c'
OBJCC_E='-E -o $@'
OBJCC_O='-o $@'
LD_O='-o $@'
LD_LIB='-l%'
LD_PATH='-L'
HOSTCC_C='-c'
HOSTCC_E='-E -o $@'
HOSTCC_O='-o $@'
HOSTLD_O='-o $@'





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

log_file(){
	log BEGIN $1
	pr -n -t $1 >> $logfile
	log END $1
}

append(){
	var=$1
	shift
	# eval命令将会首先扫描命令行进行所有的置换，然后再执行该命令
	# 按照上面的示例，置换后为 config_files="$config_files config.asm"
	eval "$var=\"\$$var $*\""
}

exesuf() {
	case $1 in
		mingw32*|mingw64*|win32|win64|cygwin*|*-dos|freedos|opendos|os/2*|symbian) echo .exe ;;
	esac
}

EXESUF=$(exesuf $target_os)
HOSTEXESUF=$(exesuf $host_os)

# set temporary file name
: ${TMPDIR:=$TEMPDIR}
: ${TMPDIR:=$TMP}
: ${TMPDIR:=/tmp}

#输出/tmp
#echo $TMPDIR
#exit

#set -C:鸟哥11.4.4

#输出11
#tmp=$(mktemp -u "/tmp/ffconf.XXXXXXXX").c && echo 11||echo 222

tmpfile(){
	tmp=$(mktemp -u "${TMPDIR}/ffconf.XXXXXXXX")$2 &&
	(set -C; exec > $tmp) 2>/dev/null ||
	die "Unable to create temporary file in $TMPDIR."
	append TMPFILES $tmp
	eval $1=$tmp
}

log(){
	echo "$@" >> $logfile
}






check_cmd(){
	#gcc -c -o /tmp/ffconf.FQKfdG7f.o /tmp/ffconf.EdS2ndXq.c
	#echo "$@"
	#exit
	log "$@"
	"$@" >> $logfile 2>&1
}

cc_o(){
	eval printf '%s\\n' $CC_O
}

check_cc(){
	log check_cc "$@"
	cat > $TMPC
	log_file $TMPC
	#很多检查都调用了这个check_cmd
	#-c 只编译不链接
	check_cmd $cc $CPPFLAGS $CFLAGS "$@" -c -o $TMPO $TMPC
}

#${f#-l}:如果$f开头是-l，那么删除-l返回剩余部分
#test xxxx&&&yyy||zzz 相当于三目操作 xxxx?yyy:zzzz
check_ld(){
	log check_ld "$@"
	type=$1
	shift 1
	flags=''
	libs=''
	for f; do
		#判断是flags还是lib
		test "${f}" = "${f#-l}" && flags="$flags $f" || libs="$libs $f"
	done
	check_$type $($filter_cflags $flags) || return
	check_cmd $ld $LDFLAGS $flags -o $TMPE $TMPO $libs $extralibs
}

add_ldflags(){
  append LDFLAGS $($ldflags_filter "$@")
}

test_ldflags(){
    log test_ldflags "$@"
    check_ld "cc" "$@" <<EOF
int main(void){ return 0; }
EOF
}

check_ldflags(){
    log check_ldflags "$@"
    test_ldflags "$@" && add_ldflags "$@"
}

#第一个参数为值，后面的参数为变量
set_all(){
	value=$1
	shift
	for var in $*; do
		eval $var=$value
	done
}

#把所有输入参数的值设置为"no"
disable(){
	set_all no $*
}

check_cc(){
	log check_cc "$@"
	cat > $TMPC
	log_file $TMPC
	check_cmd $cc $CPPFLAGS $CFLAGS "$@" $CC_C $(cc_o $TMPO) $TMPC
}

#check_func()用于检查函数。它的输入参数一个函数名。Configure中与check_func()有关的代码如下所示。

#check_func()用于检查函数。它的输入参数一个函数名
#从check_func()的定义可以看出，该函数首先将输入的第1个参数赋值给func，然后生成一个下述内容的C语言文件。
#extern int $func();
#int main(void){ $func(); }

check_func(){
	log check_func "$@"
	func=$1
	shift
	disable $func
	check_ld "cc" "$@" <<EOF && enable $func
extern int $func();
int main(void){ $func(); }
EOF
}

#trap 'rm -f -- $TMPFILES' EXIT

tmpfile TMPASM .asm
tmpfile TMPC   .c
tmpfile TMPCPP .cpp
tmpfile TMPE   $EXESUF
tmpfile TMPH   .h
tmpfile TMPM   .m
tmpfile TMPO   .o
tmpfile TMPS   .S
tmpfile TMPSH  .sh
tmpfile TMPV   .ver

check_ldflags -Wl,--as-needed
check_ldflags -Wl,-z,noexecstack

check_func isatty
