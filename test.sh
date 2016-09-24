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

# test6
test6=`echo -n "(+ 1 2)" | ./lexer_test`
if [[ $test6 != "(,+,1,2,)," ]]; then
	echo -n "test6 failed.Actual:${test6}"
	exit 1
fi

# test7
test7=`echo -n "(+ 1 a)" | ./lexer_test`
if [[ $test7 != "(,+,1,a,)," ]]; then
	echo -n "test7 failed.Actual:${test7}"
	exit 1
fi

# test8
test8=`echo -n "(define add (lambda (x y) (+ x y)))" | ./lexer_test`
if [[ $test8 != "(,define,add,(,lambda,(,x,y,),(,+,x,y,),),)," ]]; then
	echo -n "test8 failed.Actual:${test8}"
	exit 1
fi

# test9
test9=`echo -n '(define str "in the (string)")' | ./lexer_test`
if [[ $test9 != '(,define,str,"in the (string)",),' ]]; then
	echo -n "test9 failed.Actual:${test9}"
	exit 1
fi

# test10
test10=`./int_print_test`
if [[ $test10 != '1' ]]; then
	echo -n "test10 failed.Actual:${test10}"
	exit 1
fi
