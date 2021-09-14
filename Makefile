cert:
	openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -days 10000 \
		-subj "/C=US/ST=Oregon/L=Portland/O=Company Name/OU=Org/CN=www.example.com" \
		-nodes

server: cert
	gcc -Wall -O3 src/tcpserv.c -lssl -lcrypto -o build/espresso

clean:
	rm -rf build/*
	rm -rf cert.pem key.pem
