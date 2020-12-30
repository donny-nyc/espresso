server:
	gcc -Wall -O3 src/tcpserv.c -o build/espresso

clean:
	rm -rf build/*
