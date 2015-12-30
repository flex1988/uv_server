server:server.c http-parser/http_parser.o
	 gcc -g -Wall -Ilibuv/include -framework CoreServices  -o uv_server server.c  libuv/.libs/libuv.a http-parser/http_parser.o

http-parser/http_parser.o:
	$(MAKE) -C http_parser http_parser.o
