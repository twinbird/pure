#!/bin/sh

# test1
test1=`./pure tests/lambda_test.lisp`
if [[ $test1 != "2" ]]; then
	echo -n "test1 failed.Expect:2, Actual:${test1}"
	exit 1
fi

