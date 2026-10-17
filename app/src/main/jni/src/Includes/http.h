#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include "Logger.h"

#define BUFFER_SIZE 1024
#define HTTP_POST "POST /%s HTTP/1.1\r\nHOST: %s:%d\r\nAccept: */*\r\n"\
                 "Content-Type:application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n%s"
#define HTTP_GET "GET /%s HTTP/1.0\r\nHOST: %s:%d\r\nAccept: */*\r\n\r\n"
#define MY_HTTP_DEFAULT_PORT 80

static int http_tcpclient_create(const char* host, int port) {
    struct hostent* he;
    struct sockaddr_in server_addr;
    int socket_fd;

    if ((he = gethostbyname(host)) == NULL) {
        perror("gethostbyname failed");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr*)he->h_addr);

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket creation failed");
        return -1;
    }

    if (connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect failed");
        close(socket_fd);
        return -1;
    }

    return socket_fd;
}

static void http_tcpclient_close(int socket) {
    close(socket);
}

static int http_parse_url(const char* url, char* host, char* file, int* port) {
    char* ptr1;
    char* ptr2;
    int len = 0;

    if (!url || !host || !file || !port) {
        return -1;
    }

    ptr1 = (char*)url;

    if (strncmp(ptr1, "http://", strlen("http://")) == 0) {
        ptr1 += strlen("http://");
    } else if (strncmp(ptr1, "https://", strlen("https://")) == 0) {
        ptr1 += strlen("https://");
    } else {
        return -1;
    }

    ptr2 = strchr(ptr1, '/');
    if (ptr2) {
        len = ptr2 - ptr1;
        strncpy(host, ptr1, len);
        host[len] = '\0';
        strcpy(file, ptr2 + 1);
    } else {
        strcpy(host, ptr1);
        file[0] = '\0';
    }

    ptr1 = strchr(host, ':');
    if (ptr1) {
        *ptr1++ = '\0';
        *port = atoi(ptr1);
    } else {
        *port = MY_HTTP_DEFAULT_PORT;
    }

    return 0;
}

static int http_tcpclient_recv(int socket, char* lpbuff) {
    int recvnum = recv(socket, lpbuff, BUFFER_SIZE * 4, 0);
    if (recvnum < 0) {
        perror("recv failed");
        return -1;
    }
    lpbuff[recvnum] = '\0';  // Garantir que a string seja terminada corretamente
    return recvnum;
}

static int http_tcpclient_send(int socket, char* buff, int size) {
    int sent = 0;
    int tmpres;

    while (sent < size) {
        tmpres = send(socket, buff + sent, size - sent, 0);
        if (tmpres == -1) {
            perror("send failed");
            return -1;
        }
        sent += tmpres;
    }
    return sent;
}

static char* http_parse_result(const char* lpbuf) {
    char* ptmp;
    char* response;

    ptmp = (char*)strstr(lpbuf, "HTTP/1.1");
    if (!ptmp) {
        printf("HTTP/1.1 não encontrado na resposta\n");
        return NULL;
    }

    int status_code = atoi(ptmp + 9);
    if (status_code != 200) {
        printf("Erro HTTP: %d\n", status_code);
        return NULL;
    }

    ptmp = (char*)strstr(lpbuf, "\r\n\r\n");
    if (!ptmp) {
        printf("Cabeçalho HTTP não encontrado\n");
        return NULL;
    }

    response = (char*)malloc(strlen(ptmp + 4) + 1);
    if (!response) {
        perror("malloc failed");
        return NULL;
    }
    strcpy(response, ptmp + 4);
    return response;
}

char* http_get(const char* url) {
    int socket_fd = -1;
    char lpbuf[BUFFER_SIZE * 4] = { '\0' };
    char host_addr[BUFFER_SIZE] = { '\0' };
    char file[BUFFER_SIZE] = { '\0' };
    int port = 0;

    if (!url) {
       // LOGI("URL inválida\n");
        return NULL;
    }

    if (http_parse_url(url, host_addr, file, &port)) {
        //LOGI("Erro ao parsear URL\n");
        return NULL;
    }

    socket_fd = http_tcpclient_create(host_addr, port);
    if (socket_fd < 0) {
        return NULL;
    }

    snprintf(lpbuf, sizeof(lpbuf), HTTP_GET, file, host_addr, port);

    if (http_tcpclient_send(socket_fd, lpbuf, strlen(lpbuf)) < 0) {
        http_tcpclient_close(socket_fd);
        return NULL;
    }

    int recv_len = http_tcpclient_recv(socket_fd, lpbuf);
    if (recv_len <= 0) {
        //LOGI("Falha ao receber dados\n");
        http_tcpclient_close(socket_fd);
        return NULL;
    }

    //LOGI("Resposta recebida: %s\n", lpbuf);  // Imprimir o buffer para depuração
    http_tcpclient_close(socket_fd);

    return http_parse_result(lpbuf);
}


char* http_get_follow_redirect(const char* url) {
    int socket_fd = -1;
    char lpbuf[BUFFER_SIZE * 4] = { '\0' };
    char host_addr[BUFFER_SIZE] = { '\0' };
    char file[BUFFER_SIZE] = { '\0' };
    int port = 0;

    if (!url) {
        //LOGI("URL inválida\n");
        return NULL;
    }

    if (http_parse_url(url, host_addr, file, &port)) {
        //LOGI("Erro ao parsear URL\n");
        return NULL;
    }

    socket_fd = http_tcpclient_create(host_addr, port);
    if (socket_fd < 0) {
        return NULL;
    }

    snprintf(lpbuf, sizeof(lpbuf), HTTP_GET, file, host_addr, port);

    if (http_tcpclient_send(socket_fd, lpbuf, strlen(lpbuf)) < 0) {
        http_tcpclient_close(socket_fd);
        return NULL;
    }

    int recv_len = http_tcpclient_recv(socket_fd, lpbuf);
    if (recv_len <= 0) {
       // LOGI("Falha ao receber dados\n");
        http_tcpclient_close(socket_fd);
        return NULL;
    }

    //LOGI("Resposta recebida: %s\n", lpbuf);  // Imprimir o buffer para depuração

    // Verifica se há um redirecionamento
    if (strstr(lpbuf, "HTTP/1.1 301 Moved Permanently")) {
        char* new_url = strstr(lpbuf, "location: ");
        if (new_url) {
            new_url += strlen("location: ");
            char* end_url = strchr(new_url, '\r');
            if (end_url) {
                *end_url = '\0';  // Termina a URL
               //LOGI("Redirecionando para: %s\n", new_url);
                // Chama a função http_get novamente com a nova URL
                return http_get_follow_redirect(new_url);
            }
        }
    }

    http_tcpclient_close(socket_fd);
    return http_parse_result(lpbuf);
}


#endif // UTILS_H
