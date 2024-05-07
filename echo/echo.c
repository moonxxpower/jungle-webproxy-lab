#include "csapp.h"

void echo(int connfd) {
    size_t n;    // 데이터 크기 저장 변수 (바이트 단위)
    char buf[MAXLINE];    // 데이터를 실제로 받을 버퍼
    rio_t rio;

    Rio_readinitb(&rio, connfd);    // rio 구조체를 초기화하고, connfd에 대한 읽기 작업 수행할 수 있도록 설정 

    // buffer에 저장된 데이터의 크기가 0이 아닐 때까지 반복하여 클라이언트로부터 읽은 데이터(Rio_readlineb)를 다시 클라이언트로 전송하는(Rio_writen) 역할을 수행
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        printf("server received %d bytes\n", (int)n);
        Rio_writen(connfd, buf, n);
    }
}