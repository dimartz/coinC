coinc: coinc.o
	gcc -std=c11 -fmax-errors=10 -Wall -Wextra coinc.o -lm -lcurl -o coinc
coinc.o: coinc.c
	gcc -std=c11 -fmax-errors=10 -Wall -Wextra -c coinc.c -lm -lcurl -o coinc.o

install:
	sudo mv -fv coinc /usr/local/bin
clean:
	rm -fv coinc.o
uninstall:
	sudo rm -fv /usr/local/bin/coinc
