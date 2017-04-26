all:
	bison -d parser.y
	flex lexer.l
	gcc -o processor_simulator -g parser.tab.c lex.yy.c -lfl processor.c -pthread cache.c main.c -lm
	./processor_simulator input in.cfg out.svg out.res
	
clean:
	rm processor_simulator lex.yy.c parser.tab.h parser.tab.c