all :
	cc *.c -lucontext -g -Wall -Wextra

clean:
	rm -rf a.out *.o
