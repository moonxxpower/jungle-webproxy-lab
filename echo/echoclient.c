#include "csapp.h"

/* 
    표준 입출력, 에러의 파일 디스크립터 번호
    - stdin: 표준 입력 (0)
    - stdout: 표준 출력 (1)
    - stderr: 표준 에러 (2)

*/

int main(int argc, char **argv) {
    int clientfd;
    char *host, *port, buf[MAXLINE];
    rio_t rio;

    /*
        파일 실행 시 인자를 제대로 넘겨주지 않은 경우
        예) ./echoclient <host> <port>
    */
    if(argc != 3) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);    
        exit(0);
    }

    host = argv[1];
    port = argv[2];

    clientfd = Open_clientfd(host, port);    // 서버와의 연결 수립하고, 소켓 식별자를 clientfd에 저장
    Rio_readinitb(&rio, clientfd);    // rio 구조체를 초기화하고, clientfd에 대한 읽기 작업 수행할 수 있도록 설정 

    /*
        표준 입력에서 텍스트 줄을 반복해서 읽는 루프 진입
        EOF(End Of File) 표준 입력을 만나면 종료
    */
    while(Fgets(buf, MAXLINE, stdin) != NULL) {
        Rio_writen(clientfd, buf, strlen(buf));    // buffer에 있는 데이터를 서버에 전송
        Rio_readlineb(&rio, buf, MAXLINE);    // 서버가 전송한 데이터를 읽어와서 buffer에 저장
        Fputs(buf, stdout);    // buffer에 저장된 문자열을 표준 출력으로 출력
    }

    Close(clientfd);
    exit(0);
}