#!/bin/sh

# test1
test1=`echo -n "(" | ./lexer_test`
if [[ $test1 != "(," ]]; then
	echo -n "test1 failed.Actual:${test1}"
	exit 1
fi

# test2
test2=`echo -n "()" | ./lexer_test`
if [[ $test2 != "(,)," ]]; then
	echo -n "test2 failed.Actual:${test2}"
	exit 1
fi

# test3
test3=`echo -n "(1)" | ./lexer_test`
if [[ $test3 != "(,1,)," ]]; then
	echo -n "test3 failed.Actual:${test3}"
	exit 1
fi

# test4
test4=`echo -n "(10)" | ./lexer_test`
if [[ $test4 != "(,10,)," ]]; then
	echo -n "test4 failed.Actual:${test4}"
	exit 1
fi

# test5
test5=`echo -n "(10 11 12)" | ./lexer_test`
if [[ $test5 != "(,10,11,12,)," ]]; then
	echo -n "test5 failed.Actual:${test5}"
	exit 1
fi
