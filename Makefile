server:
	gcc -Wall -O3 src/tcpserv.c -o build/espresso

debug:
	gcc -Wall -g src/tcpserv.c -o build/espresso

hash:
	gcc -Wall -O3 spike/hash.c -o build/hash

request_test:
	gcc -Wall -g src/request_driver.c src/echo_request_handler.c src/request_handler.h -o test/request_test

test: request_test
	test/request_test

lexer:
	gcc -Wall -O2 src/lexer.c -o build/lexer

read_test:
	gcc -Wall src/read_test.c -o test/read_test

r_test: read_test
	cat test/SampleTextFile_1000kb.txt | test/read_test

clean:
	rm -rf build/*
	rm -rf test/*
