test: lexer_test int_print_test symbol_print_test

lexer_test: lexer_test.o pureLisp.o

int_print_test: int_print_test.o pureLisp.o

symbol_print_test: symbol_print_test.o pureLisp.o

clean:
	rm -rf *.o
	rm -rf *.exe
	rm -rf test
