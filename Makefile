test: lexer_test int_print_test symbol_print_test string_print_test list_print_test nested_list_print_test dot_pair_print_test nil_print_test unget_token_test read_test

lexer_test: lexer_test.o pureLisp.o

int_print_test: int_print_test.o pureLisp.o

symbol_print_test: symbol_print_test.o pureLisp.o

string_print_test: string_print_test.o pureLisp.o

list_print_test: list_print_test.o pureLisp.o

nested_list_print_test: nested_list_print_test.o pureLisp.o

nil_print_test: nil_print_test.o pureLisp.o

dot_pair_print_test: dot_pair_print_test.o pureLisp.o

unget_token_test: unget_token_test.o pureLisp.o

read_test: read_test.o pureLisp.o

clean:
	rm -rf *.o
	rm -rf *.exe
	rm -rf test
	rm -rf *.stackdump
