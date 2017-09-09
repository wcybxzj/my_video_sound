#!/bin/sh

#${file:-my.file.txt}	若 $file 没有设定或为空值,则使用 my.file.txt 作传回值
echo ${bigendian-no}
echo $bigendian
