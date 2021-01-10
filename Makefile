server:
	gcc -Wall -O3 src/tcpserv.c -o build/espresso

debug:
	gcc -Wall -g src/tcpserv.c -o build/espresso

hash:
	gcc -Wall -O3 spike/hash.c -o build/hash

clean:
	rm -rf build/*
