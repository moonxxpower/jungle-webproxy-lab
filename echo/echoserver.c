#include "csapp.h"
#include "echo.c"

void echo(int connfd);

int main(int argc, char **argv) {

     /*
          socklen_t: 소켓 프로그래밍 라이브러리에 정의된 데이터 타입 -> 소켓 주소 구조의 길이나 크기 지정
          struct sockaddr_storage: 네트워크 프로그래밍에서 사용되는 구조체, 다양한 유형의 소켓 주소를 지정할 수 있도록 설계
     */
     int listenfd, connfd;
     socklen_t clientlen;    // 클라이언트의 주소 길이
     struct sockaddr_storage clientaddr;    // 클라이언트의 주소 정보 저장
     char client_hostname[MAXLINE], client_port[MAXLINE];    // 클라이언트의 호스트 이름과 포트 번호 저장 배열

     /*
        파일 실행 시 인자를 제대로 넘겨주지 않은 경우
        예) ./echoserver <port>
    */
     if (argc != 2) {
          fprintf(stderr, "usage: %s <port>\n", argv[0]);
          exit(0);
     }

     listenfd = Open_listenfd(argv[1]);    // 주어진 port 번호로 listening socket을 열어서 파일 디스크립터를 얻음

     while (1) {

          /*
               클라이언트가 서버에 연결 요청을 하면, 
               서버는 요청을 수락하고 해당 클라이언트와 통신하기 위해 새로운 socket을 생성한다.
               이때, 새로운 소켓의 주소 정보를 클라이언트의 주소로 설정하여 통신을 수행한다. 
          */
          clientlen = sizeof(struct sockaddr_storage);    // 클라이언트의 주소 길이 초기화 (새로운 소켓을 생성할 때, 해당 클라이언트의 주소 정보를 새로 생성된 소켓에 설정하기 위함)
          connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);    // Accept() 함수가 호출될 때,(SA *)&clientaddr은 해당 클라이언트의 주소 정보를 저장하기 위한 곳을 지정한다. (새로운 socket 생성) 이후, 해당 소켓의 파일 디스크립터가 반환된다. (connfd)
          Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);    // 클라이언트의 주소 정보를 얻어와서 호스트 이름과 포트 번호로 변환한다. 
          printf("Connected to (%s, %s)\n", client_hostname, client_port);
          echo(connfd);
          Close(connfd);
     }

     exit(0);
}