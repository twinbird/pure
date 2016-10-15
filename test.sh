#!/bin/sh

# test1
test1=`./pure tests/lambda_test.lisp`
if [[ $test1 != "2" ]]; then
	echo -n "test1 failed.Expect:2, Actual:${test1}"
	exit 1
fi

# test2
test2=`echo "(print 1)" | ./pure -e`
if [[ $test2 != "1" ]]; then
	echo -n "test2 failed.Expect:1, Actual:${test2}"
	exit 1
fi

# test3
test3=`echo '(print "hello")' | ./pure -e`
if [[ $test3 != '"hello"' ]]; then
	echo -n "test3 failed.Expect:'"hello"', Actual:${test3}"
	exit 1
fi
