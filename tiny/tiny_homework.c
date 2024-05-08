/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 *
 * Updated 11/2019 droh
 *   - Fixed sprintf() aliasing issue in serve_static(), and clienterror().
 */
#include "csapp.h"

/* 
 * 11장 6번 (c) 
 * Tiny의 출력을 조사해서 HTTP version 통일하기
 *   - Response Header의 HTTP version을 Request Header에 담긴 version으로 통일시켜주기 (원래 'HTTP/1.0'으로 되어 있음)
 */

/*  
 * 11장 7번
 * Tiny를 확장해서 MPG 비디오 파일 처리하기
 *   - filetype에 'video/mp4' 추가하고, HTML에 video 추가
 *   ; HTML5에서 MPG 처리가 어렵기 때문에, MP4 비디오 파일로 처리함
 */

/*  
 * 11장 9번
 * 정적 콘텐츠를 처리할 때, 요청할 파일을 mmap과 malloc을 사용해서 연결 식별자에게 복사하기
 *   - mmap: 파일을 메모리에 매핑하여 파일의 내용을 직접적으로 접근할 수 있게 함 (해제: munmap) 
 *   - malloc: 사용자가 요청한 크기의 메모리 블록을 할당 (해제: free) > 메모리 할당 후, rio_readn으로 할당된 메모리 블록에 파일을 읽어야 함
 */

/*  
 * 11장 10번
 * 실제 브라우저에 두 개의 텍스트 상자를 포함하여 상자에 입력한 내용을 토대로 동적 콘텐츠를 표시하기
 *   - html에 입력 상자를 만들고, 입력 값을 토대로 동적 콘텐츠 표시하기 (adder.c 인자 값 수정)
 */

/*  
 * 11장 11번
 * HTTP HEAD 메소드를 지원하도록 확장하기
 *   - HEAD: Response Header만을 보내기 (Response Body 제외)
 *   ; HEAD는 종종 캐싱을 사용하는 클라이언트가 가장 최근에 접속한 이후로 문서가 바뀌었는지를 보기 위해 사용한다.
 */

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize, char *method, char *version);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs, char *method, char *version);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg, char *version);

int main(int argc, char **argv) {
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  /* Check command line args */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]);
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);  // line:netp:tiny:accept
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    doit(connfd);   // line:netp:tiny:doit
    Close(connfd);  // line:netp:tiny:close
  }
}

void doit(int fd) {
  int is_static;
  struct stat sbuf;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char filename[MAXLINE], cgiargs[MAXLINE];
  rio_t rio;

  /* Read request line and headers */
  Rio_readinitb(&rio, fd);

  if (!(Rio_readlineb(&rio, buf, MAXLINE))) {
    return;
  }
  
  printf("Request headers:\n");
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version);

  if ((strcasecmp(method, "GET")) * (strcasecmp(method, "HEAD"))) {
    clienterror(fd, method, "501", "Not implemented", "Tiny does not implement this method", version);
    return;
  }

  read_requesthdrs(&rio);

  /* Parse URI from GET request */
  is_static = parse_uri(uri, filename, cgiargs);
  if (stat(filename, &sbuf) < 0) {
    clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file", version);
    return;
  }

  /* Serve static content */
  if (is_static) {    
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
      clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file", version);
      return;
    }
    
    serve_static(fd, filename, sbuf.st_size, method, version);
  }

  /* Serve dynamic content */
  else {    
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
      clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run the CGI program", version);
      return;
    }

    serve_dynamic(fd, filename, cgiargs, method, version);
  }
}

void read_requesthdrs(rio_t *rp) {
  char buf[MAXLINE];

  Rio_readlineb(rp, buf, MAXLINE);
  while(strcmp(buf, "\r\n")) {
    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
  }

  return;
}

int parse_uri(char *uri, char *filename, char *cgiargs) {
  char *ptr;

  /* Static Content */
  if (!strstr(uri, "cgi-bin")) {    
    strcpy(cgiargs, "");
    strcpy(filename, ".");
    strcat(filename, uri);
    
    if (uri[strlen(uri) - 1] == '/') {
      strcat(filename, "home_homework.html");
    }

    return 1;
  }

  /* Dynamic Content */
  else {    
    ptr = index(uri, '?');

    if (ptr) {
      strcpy(cgiargs, ptr + 1);
      *ptr = '\0';
    }

    else {
      strcpy(cgiargs, "");
    }

    strcpy(filename, ".");
    strcat(filename, uri);

    return 0;
  }
}

void serve_static(int fd, char *filename, int filesize, char *method, char *version) {
  int srcfd;
  char *srcp, filetype[MAXLINE], buf[MAXBUF];

  /* Send response headers to client */
  get_filetype(filename, filetype);
  sprintf(buf, "%s 200 OK\r\n", version);
  sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
  sprintf(buf, "%sConnection: close\r\n", buf);
  sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
  sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);

  Rio_writen(fd, buf, strlen(buf));
  printf("Response headers:\n");
  printf("%s", buf);

  if (strcasecmp(method, "HEAD") == 0) {
    return;
  }

  /* Send response body to client */
  srcfd = Open(filename, O_RDONLY, 0);
  srcp = (char *) malloc(filesize);
  rio_readn(srcfd, srcp, filesize);
  Close(srcfd);
  Rio_writen(fd, srcp, filesize);
  free(srcp);
}

void get_filetype(char *filename, char *filetype) {
  if (strstr(filename, ".html")) {
    strcpy(filetype, "text/html");
  }

  else if (strstr(filename, ".gif")) {
    strcpy(filetype, "image/gif");
  }

  else if (strstr(filename, ".png")) {
    strcpy(filetype, "image/png");
  }

  else if (strstr(filename, ".jpg")) {
    strcpy(filetype, "image/jpeg");
  }

  else if (strstr(filename, ".mp4")) {
    strcpy(filetype, "video/mp4");
  }

  else {
    strcpy(filetype, "text/plain");
  }
}

void serve_dynamic(int fd, char *filename, char *cgiargs, char *method, char *version) {
  char buf[MAXLINE], *emptylist[] = { NULL };

  /* Return first part of HTTP response */
  sprintf(buf, "%s 200 OK\r\n", version);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Server: Tiny Web Server\r\n");
  Rio_writen(fd, buf, strlen(buf));

  if (Fork() == 0) {    /* Child */
    /* Real server would set alll CGI vars here */
    setenv("QUERY_STRING", cgiargs, 1);
    setenv("REQUEST_METHOD", method, 1);
    Dup2(fd, STDOUT_FILENO);    /* Redirect stdout to client */
    Execve(filename, emptylist, environ);    /* Run CGI program */
  }

  Wait(NULL);    /* Parent waits for and reaps child */
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg, char *version) {
    char buf[MAXLINE], body[MAXBUF];

    /* Build the HTTP response body */
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

    /* Print the HTTP response */
    sprintf(buf, "%s %s %s\r\n", version, errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}