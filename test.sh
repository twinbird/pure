#!/bin/sh

# test1
# lambda�̓���m�F
test1=`./pure tests/lambda_test.lisp`
if [[ $test1 != "2" ]]; then
	echo -n "test1 failed.Expect:2, Actual:${test1}"
	exit 1
fi

# test2
# integer��read��print�m�F
test2=`echo "(print 1)" | ./pure -e`
if [[ $test2 != "1" ]]; then
	echo -n "test2 failed.Expect:1, Actual:${test2}"
	exit 1
fi

# test3
# string��read��print�m�F
test3=`echo '(print "hello")' | ./pure -e`
if [[ $test3 != '"hello"' ]]; then
	echo -n "test3 failed.Expect:'"hello"', Actual:${test3}"
	exit 1
fi

# test3_1
# nil��read��print�m�F
test3_1=`echo '(print nil)' | ./pure -e`
if [[ $test3_1 != 'nil' ]]; then
	echo -n "test3_1 failed.Expect:'nil', Actual:${test3_1}"
	exit 1
fi

# test3_2
# t��read��print�m�F
test3_2=`echo '(print t)' | ./pure -e`
if [[ $test3_2 != 't' ]]; then
	echo -n "test3_2 failed.Expect:'t', Actual:${test3_2}"
	exit 1
fi

# test3_3
# define��symbol��print�m�F
test3_3=`echo '(define a 1) (print a)' | ./pure -e`
if [[ $test3_3 != '1' ]]; then
	echo -n "test3_3 failed.Expect:'1', Actual:${test3_3}"
	exit 1
fi

# test4
# atom�֐� + string�Ń`�F�b�N
test4=`echo '(print (atom "hello"))' | ./pure -e`
if [[ $test4 != 't' ]]; then
	echo -n "test4 failed.Expect:'t', Actual:${test4}"
	exit 1
fi

# test4_1
# atom�֐� + integer�Ń`�F�b�N
test4_1=`echo '(print (atom 1))' | ./pure -e`
if [[ $test4_1 != 't' ]]; then
	echo -n "test4_1 failed.Expect:'t', Actual:${test4_1}"
	exit 1
fi

# test4_2
# atom�֐� + nil�Ń`�F�b�N
test4_2=`echo '(print (atom nil))' | ./pure -e`
if [[ $test4_2 != 't' ]]; then
	echo -n "test4_2 failed.Expect:'t', Actual:${test4_2}"
	exit 1
fi

# test4_3
# atom�֐� + t�Ń`�F�b�N
test4_3=`echo '(print (atom t))' | ./pure -e`
if [[ $test4_3 != 't' ]]; then
	echo -n "test4_3 failed.Expect:'t', Actual:${test4_3}"
	exit 1
fi

# test4_4
# atom�֐� + �y�A�Ń`�F�b�N.cons�ł̃��X�g�������`�F�b�N
test4_4=`echo '(print (atom (cons t nil)))' | ./pure -e`
if [[ $test4_4 != 'nil' ]]; then
	echo -n "test4_4 failed.Expect:'nil', Actual:${test4_4}"
	exit 1
fi

# test5
# cons��dot pair�\���̃`�F�b�N
test5=`echo '(print (cons "hello" "world"))' | ./pure -e`
if [[ $test5 != '("hello" . "world")' ]]; then
	echo -n "test5 failed.Expect:'("hello" . "world")', Actual:${test5}"
	exit 1
fi

# test5_1
# cons��list�\���̃`�F�b�N
test5_1=`echo '(print (cons "hello world" nil))' | ./pure -e`
if [[ $test5_1 != '("hello world")' ]]; then
	echo -n "test5_1 failed.Expect:'("hello world")', Actual:${test5_1}"
	exit 1
fi

# test6
# eq integer ���l�`�F�b�N
test6=`echo '(print (eq 1 1))' | ./pure -e`
if [[ $test6 != 't' ]]; then
	echo -n "test6 failed.Expect:'t', Actual:${test6}"
	exit 1
fi

# test6_1
# eq integer �ْl�`�F�b�N
test6_1=`echo '(print (eq 1 2))' | ./pure -e`
if [[ $test6_1 != 'nil' ]]; then
	echo -n "test6_1 failed.Expect:'nil', Actual:${test6_1}"
	exit 1
fi

# test6_2
# eq string �ْl�`�F�b�N
test6_2=`echo '(print (eq "hello" "world"))' | ./pure -e`
if [[ $test6_2 != 'nil' ]]; then
	echo -n "test6_2 failed.Expect:'nil', Actual:${test6_2}"
	exit 1
fi

# test6_3
# eq string ���l�`�F�b�N
test6_3=`echo '(print (eq "hello" "hello"))' | ./pure -e`
if [[ $test6_3 != 't' ]]; then
	echo -n "test6_3 failed.Expect:'t', Actual:${test6_3}"
	exit 1
fi

# test6_4
# eq nil ���l�`�F�b�N
test6_4=`echo '(print (eq nil nil))' | ./pure -e`
if [[ $test6_4 != 't' ]]; then
	echo -n "test6_4 failed.Expect:'t', Actual:${test6_4}"
	exit 1
fi

# test6_5
# eq nil �ْl�`�F�b�N
test6_5=`echo '(print (eq nil t))' | ./pure -e`
if [[ $test6_5 != 'nil' ]]; then
	echo -n "test6_5 failed.Expect:'nil', Actual:${test6_5}"
	exit 1
fi

# test6_6
# eq t ���l�`�F�b�N
test6_6=`echo '(print (eq t t))' | ./pure -e`
if [[ $test6_6 != 't' ]]; then
	echo -n "test6_6 failed.Expect:'t', Actual:${test6_6}"
	exit 1
fi

# test7
# quote �A�g��
test7=`echo '(print (quote 1))' | ./pure -e`
if [[ $test7 != '1' ]]; then
	echo -n "test7 failed.Expect:'1', Actual:${test7}"
	exit 1
fi

# test7_1
# quote ���X�g
test7_1=`echo '(print (quote (1 2 3)))' | ./pure -e`
if [[ $test7_1 != '(1 2 3)' ]]; then
	echo -n "test7_1 failed.Expect:'(1 2 3)', Actual:${test7_1}"
	exit 1
fi

# test7_2
# quote �󃊃X�g
test7_2=`echo '(print (quote ()))' | ./pure -e`
if [[ $test7_2 != 'nil' ]]; then
	echo -n "test7_2 failed.Expect:'nil', Actual:${test7_2}"
	exit 1
fi

# test8
# car ���X�g�ɓK�p
test8=`echo '(print (car (quote (1 2 3))))' | ./pure -e`
if [[ $test8 != '1' ]]; then
	echo -n "test8 failed.Expect:'1', Actual:${test8}"
	exit 1
fi

# test8_1
# cdr ���X�g�ɓK�p
test8_1=`echo '(print (cdr (quote (1 2 3))))' | ./pure -e`
if [[ $test8_1 != '(2 3)' ]]; then
	echo -n "test8_1 failed.Expect:'(2 3)', Actual:${test8_1}"
	exit 1
fi

# test9
# if true�̏ꍇ
test9=`echo '(print (if t 1 2))' | ./pure -e`
if [[ $test9 != '1' ]]; then
	echo -n "test9 failed.Expect:'1', Actual:${test9}"
	exit 1
fi

# test9_1
# if false�̏ꍇ
test9_1=`echo '(print (if nil 1 2))' | ./pure -e`
if [[ $test9_1 != '2' ]]; then
	echo -n "test9_1 failed.Expect:'2', Actual:${test9_1}"
	exit 1
fi

# test10
# + ���Z
test10=`echo '(print (+ 1 2))' | ./pure -e`
if [[ $test10 != '3' ]]; then
	echo -n "test10 failed.Expect:'3', Actual:${test10}"
	exit 1
fi

# test11
# - ���Z
test11=`echo '(print (- 2 1))' | ./pure -e`
if [[ $test11 != '1' ]]; then
	echo -n "test11 failed.Expect:'1', Actual:${test11}"
	exit 1
fi

# test12
# * ���Z
test12=`echo '(print (* 3 2))' | ./pure -e`
if [[ $test12 != '6' ]]; then
	echo -n "test12 failed.Expect:'6', Actual:${test12}"
	exit 1
fi

# test13
# / ���Z
test13=`echo '(print (/ 4 2))' | ./pure -e`
if [[ $test13 != '2' ]]; then
	echo -n "test13 failed.Expect:'2', Actual:${test13}"
	exit 1
fi

# test14
# % ���Z
test14=`echo '(print (% 10 3))' | ./pure -e`
if [[ $test14 != '1' ]]; then
	echo -n "test14 failed.Expect:'1', Actual:${test14}"
	exit 1
fi

# test15
# < ���Z t
test15=`echo '(print (< 2 3))' | ./pure -e`
if [[ $test15 != 't' ]]; then
	echo -n "test15 failed.Expect:'t', Actual:${test15}"
	exit 1
fi

# test15_1
# < ���Z nil
test15_1=`echo '(print (< 3 3))' | ./pure -e`
if [[ $test15_1 != 'nil' ]]; then
	echo -n "test15_1 failed.Expect:'nil', Actual:${test15_1}"
	exit 1
fi

# test16
# > ���Z t
test16=`echo '(print (> 3 2))' | ./pure -e`
if [[ $test16 != 't' ]]; then
	echo -n "test16 failed.Expect:'t', Actual:${test16}"
	exit 1
fi

# test16_1
# > ���Z nil
test16_1=`echo '(print (> 3 3))' | ./pure -e`
if [[ $test16_1 != 'nil' ]]; then
	echo -n "test16_1 failed.Expect:'nil', Actual:${test16_1}"
	exit 1
fi

# test17
# <= ���Z t
test17=`echo '(print (<= 3 3))' | ./pure -e`
if [[ $test17 != 't' ]]; then
	echo -n "test17 failed.Expect:'t', Actual:${test17}"
	exit 1
fi

# test17_1
# <= ���Z nil
test17_1=`echo '(print (<= 3 2))' | ./pure -e`
if [[ $test17_1 != 'nil' ]]; then
	echo -n "test17_1 failed.Expect:'nil', Actual:${test17_1}"
	exit 1
fi

# test18
# >= ���Z t
test18=`echo '(print (>= 3 3))' | ./pure -e`
if [[ $test18 != 't' ]]; then
	echo -n "test18 failed.Expect:'t', Actual:${test18}"
	exit 1
fi

# test18_1
# >= ���Z nil
test18_1=`echo '(print (>= 3 4))' | ./pure -e`
if [[ $test18_1 != 'nil' ]]; then
	echo -n "test18_1 failed.Expect:'nil', Actual:${test18_1}"
	exit 1
fi

# test19
# tak�̓���m�F(�ċA�ƃX�R�[�v��)
test19=`./pure tests/tak.lisp`
if [[ $test19 != "10" ]]; then
	echo -n "test19 failed.Expect:10, Actual:${test19}"
	exit 1
fi

