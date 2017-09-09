#!/bin/sh

#知识点:
#for没有in, opt指定的就是命令行过来的参数
#./2.for_opt.sh 11 22 33
#11
#22
#33

for opt do
	echo $opt
done

