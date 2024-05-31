cc = tcc

all :
	$(cc) jmp.S *.c -ggdb -Wall -Wextra

clean:
	rm -rf a.out *.o
