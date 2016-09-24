#!/bin/sh

# test1
test1=`echo -n "(" | ./lexer_test`
if [[ $test1 != "(" ]]; then
	echo -n "test1 failed.Actual:${test1}"
	exit 1
fi

