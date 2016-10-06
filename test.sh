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

# test11
test11=`./symbol_print_test`
if [[ $test11 != 'hello' ]]; then
	echo -n "test11 failed.Actual:${test11}"
	exit 1
fi

# test12
test12=`./string_print_test`
if [[ $test12 != 'hello string' ]]; then
	echo -n "test12 failed.Actual:${test12}"
	exit 1
fi

# test13
test13=`./list_print_test`
if [[ $test13 != '(1 2)' ]]; then
	echo -n "test13 failed.Actual:${test13}"
	exit 1
fi

# test14
test14=`./nested_list_print_test`
if [[ $test14 != '(lambda (x) (+ x 1))' ]]; then
	echo -n "test14 failed.Actual:${test14}"
	exit 1
fi

# test15
test15=`./dot_pair_print_test`
if [[ $test15 != '(1 . 2)' ]]; then
	echo -n "test15 failed.Actual:${test15}"
	exit 1
fi

# test16
test16=`./nil_print_test`
if [[ $test16 != 'nil' ]]; then
	echo -n "test16 failed.Actual:${test16}"
	exit 1
fi

# test17
test17=`echo -n "(1 . 2)" | ./lexer_test`
if [[ $test17 != '(,1,.,2,),' ]]; then
	echo -n "test17 failed.Actual:${test17}"
	exit 1
fi

# test18
test18=`echo -n "(define add (lambda (x y) (+ x y)))" | ./unget_token_test`
if [[ $test18 != "(,define,add,(,lambda,(,x,y,),(,+,x,y,),),)," ]]; then
	echo -n "test18 failed.Actual:${test18}"
	exit 1
fi

# test19
test19=`echo -n "1" | ./read_test`
if [[ $test19 != "1" ]]; then
	echo -n "test19 failed.Actual:${test19}"
	exit 1
fi

# test20
test20=`echo -n '"hello world"' | ./read_test`
if [[ $test20 != 'hello world' ]]; then
	echo -n "test20 failed.Actual:${test20}"
	exit 1
fi

# test21
test21=`echo -n 'nil' | ./read_test`
if [[ $test21 != 'nil' ]]; then
	echo -n "test21 failed.Actual:${test21}"
	exit 1
fi

# test22
test22=`echo -n "(+ 2 3)" | ./read_test`
if [[ $test22 != "(+ 2 3)" ]]; then
	echo -n "test22 failed.Actual:${test22}"
	exit 1
fi

# test23
test23=`echo -n "(define add (lambda (x y) (+ x y)))" | ./read_test`
if [[ $test23 != "(define add (lambda (x y) (+ x y)))" ]]; then
	echo -n "test23 failed.Actual:${test23}"
	exit 1
fi

# test24
test24=`echo -n "(1 . 2)" | ./read_test`
if [[ $test24 != "(1 . 2)" ]]; then
	echo -n "test24 failed.Actual:${test24}"
	exit 1
fi

# test25
test25=`echo -n "(1 . (2 . 3))" | ./read_test`
if [[ $test25 != "(1 2 . 3)" ]]; then
	echo -n "test25 failed.Actual:${test25}"
	exit 1
fi

# test26
test26=`echo -n "1" | ./eval_test`
if [[ $test26 != "1" ]]; then
	echo -n "test26 failed.Actual:${test26}"
	exit 1
fi

# test27
test27=`echo -n '"hello world"' | ./eval_test`
if [[ $test27 != 'hello world' ]]; then
	echo -n "test27 failed.Actual:${test27}"
	exit 1
fi

# test28
test28=`echo -n 'nil' | ./eval_test`
if [[ $test28 != 'nil' ]]; then
	echo -n "test28 failed.Actual:${test28}"
	exit 1
fi

# test29
test29=`echo -n '(quote (1 23 456))' | ./eval_test`
if [[ $test29 != '(1 23 456)' ]]; then
	echo -n "test29 failed.Actual:${test29}"
	exit 1
fi

# test30
test30=`echo -n '(quote (a bc def))' | ./eval_test`
if [[ $test30 != '(a bc def)' ]]; then
	echo -n "test30 failed.Actual:${test30}"
	exit 1
fi

# test31
test31=`./env_test`
if [[ $test31 != 'Success' ]]; then
	echo -n "test31 failed.${test31}"
	exit 1
fi

# test32
test32=`./define_test`
if [[ $test32 != 'Success' ]]; then
	echo -n "test32 failed.${test32}"
	exit 1
fi

test33=`echo -n '(if nil 1 2)' | ./eval_test`
if [[ $test33 != '2' ]]; then
	echo -n "test33 failed.Expect:'2'.Actual:${test33}"
	exit 1
fi

test34=`echo -n '(if 1 1 2)' | ./eval_test`
if [[ $test34 != '1' ]]; then
	echo -n "test34 failed.Expect:'1'.Actual:${test34}"
	exit 1
fi

test35=`echo -n 't' | ./eval_test`
if [[ $test35 != 't' ]]; then
	echo -n "test35 failed.Expect:'t'.Actual:${test35}"
	exit 1
fi

test36=`echo -n 'nil' | ./eval_test`
if [[ $test36 != 'nil' ]]; then
	echo -n "test36 failed.Expect:'nil'.Actual:${test36}"
	exit 1
fi

test37=`echo -n '(atom (quote 1 2))' | ./eval_test`
if [[ $test37 != 'nil' ]]; then
	echo -n "test37 failed.Expect:'nil'.Actual:${test37}"
	exit 1
fi

test38=`echo -n '(atom 1)' | ./eval_test`
if [[ $test38 != 't' ]]; then
	echo -n "test38 failed.Expect:'t'.Actual:${test38}"
	exit 1
fi

test39=`echo -n '(eq 1 1)' | ./eval_test`
if [[ $test39 != 't' ]]; then
	echo -n "test39 failed.Expect:'t'.Actual:${test39}"
	exit 1
fi

test40=`echo -n '(eq 1 2)' | ./eval_test`
if [[ $test40 != 'nil' ]]; then
	echo -n "test40 failed.Expect:'nil'.Actual:${test40}"
	exit 1
fi

test41=`echo -n '(eq "hello" "world")' | ./eval_test`
if [[ $test41 != 'nil' ]]; then
	echo -n "test41 failed.Expect:'nil'.Actual:${test41}"
	exit 1
fi

test42=`echo -n '(eq "hello" "hello")' | ./eval_test`
if [[ $test42 != 't' ]]; then
	echo -n "test42 failed.Expect:'t'.Actual:${test42}"
	exit 1
fi

test43=`echo -n '(eq hello hello)' | ./eval_test`
if [[ $test43 != 't' ]]; then
	echo -n "test43 failed.Expect:'t'.Actual:${test43}"
	exit 1
fi

test44=`echo -n '(eq hello world)' | ./eval_test`
if [[ $test44 != 'nil' ]]; then
	echo -n "test44 failed.Expect:'nil'.Actual:${test44}"
	exit 1
fi

test45=`echo -n '(cons 1 2)' | ./eval_test`
if [[ $test45 != '(1 . 2)' ]]; then
	echo -n "test45 failed.Expect:'(1 . 2)'.Actual:${test45}"
	exit 1
fi

test46=`echo -n '(cons hello world)' | ./eval_test`
if [[ $test46 != '(hello . world)' ]]; then
	echo -n "test46 failed.Expect:'(hello . world)'.Actual:${test46}"
	exit 1
fi

test47=`echo -n '(cons hello nil)' | ./eval_test`
if [[ $test47 != '(hello)' ]]; then
	echo -n "test47 failed.Expect:'(hello)'.Actual:${test47}"
	exit 1
fi
