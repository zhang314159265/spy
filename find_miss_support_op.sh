#!/bin/bash

function find_op_list() {
	path=$1
	shift
	cat $path | grep 'case TARGET' | sed 's/.*TARGET.\(.*\).:.*/\1/' >> /tmp/oplist
}

rm -f /tmp/oplist
find_op_list Include/eval.h
find_op_list ../cpy/Python/ceval.c

cat /tmp/oplist | sort | uniq -c | grep -v ' 2 ' | awk '{print $2}'
