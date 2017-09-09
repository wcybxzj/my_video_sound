#!/bin/bash
a=1
b=1

test $a = $b

[ $a = $b ] && echo 'real'
