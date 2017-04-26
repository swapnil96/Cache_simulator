all: 
	gcc -o sim main.c cache.c -lm
	./sim -bs 16 -us 8192 -a 1 -wb -wa traces/public-block.trace
	