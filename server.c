#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "http-parser/http_parser.h"
#include <uv.h>

#define DEFAULT_PORT 7000
#define DEFAULT_BACKLOG 128

uv_loop_t* loop;

struct sockaddr_in addr;

static http_parser_settings settings;

static uv_tcp_t server;

#define RESPONSE \
  "HTTP/1.1 200 OK\r\n" \
  "Content-Type: text/plain\r\n" \
  "Content-Length: 12\r\n" \
  "\r\n" \
  "hello world\n"

static uv_buf_t resbuf;

typedef struct {
    uv_tcp_t tcp_handle;
    http_parser parser;
    uv_write_t write_req;
} client_t;

void on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
    buf->base = (char*)malloc(suggested_size);
    buf->len = suggested_size;
}

void echo_write(uv_write_t* req, int status)
{
    if (status) {
        fprintf(stderr, "Write error %s\n", uv_strerror(status));
    }
    free(req);
}

void on_read(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf)
{
    client_t* client = handle->data;
    size_t parsed;
    if (nread > 0) {
        parsed = http_parser_execute(&client->parser, &settings, buf->base, nread);

        if (parsed < nread) {
            fprintf(stderr, "Parser error\n");
        }

        write(1, buf->base, nread);
    }
    else {
    }
    if (buf->base)
        free(buf->base);
}

void on_close(uv_handle_t* handle)
{
    client_t* client = (client_t*)handle->data;
    free(client);
}

void on_new_connection(uv_work_t* req)
{
  client_t* client=(client_t*)req->data;

  int r = uv_accept((uv_stream_t*)&server, (uv_stream_t*)&client->tcp_handle);

  if (r == 0) {
      http_parser_init(&client->parser, HTTP_REQUEST);
      uv_read_start((uv_stream_t*)&client->tcp_handle, on_alloc, on_read);
  }
  else {
      fprintf(stderr, "error on read:%s\n", uv_strerror(r));
      uv_close((uv_handle_t*)&client->tcp_handle, on_close);
  }
}

void on_connection(uv_stream_t* server, int status)
{
    if (status < 0) {
        fprintf(stderr, "New connection error %s\n", uv_strerror(status));
        return;
    }
    client_t* client = malloc(sizeof(client_t));

    client->parser.data = client;
    client->tcp_handle.data = client;

    uv_work_t* work_req = malloc(sizeof(uv_work_t));

    work_req->data = client;

    uv_tcp_init(loop, &client->tcp_handle);

    uv_queue_work(loop,work_req,on_new_connection,NULL);
}

void after_write(uv_write_t* req, int status) { uv_close((uv_handle_t*)req->handle, on_close); }

int on_headers_complete(http_parser* parser)
{
    client_t* client = (client_t*)parser->data;
    uv_write(&client->write_req, (uv_stream_t*)&client->tcp_handle, &resbuf, 1, after_write);
    return 1;
}

int main(int argc, char** argv)
{
    loop = uv_default_loop();

    settings.on_headers_complete = on_headers_complete;

    resbuf.base = RESPONSE;
    resbuf.len = sizeof(RESPONSE);

    uv_tcp_init(loop, &server);

    uv_ip4_addr("0.0.0.0", DEFAULT_PORT, &addr);

    uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);

    int r = uv_listen((uv_stream_t*)&server, DEFAULT_BACKLOG, on_connection);

    if (r) {
        fprintf(stderr, "error on connection:%s\n", uv_strerror(r));
        return 1;
    }

    return uv_run(loop, UV_RUN_DEFAULT);
}
