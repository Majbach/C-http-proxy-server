/*this is just a prototype, if i feel like it i will improve it
gcc -c main.c proxy.c;gcc main.o proxy.o -o proxy*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
#include "proxy.h"
int proxy_handle(int fd);
int main() //program driver, handles listening and incoming requests
{
    int sock_opt=1;
    int cli_fd,srv_fd;//file descriptors for listening and client socket
    struct sockaddr_in server,client;
    socklen_t len;
    //setting up listening socket
    server.sin_family=AF_INET;
    server.sin_addr.s_addr=htons(INADDR_ANY);
    server.sin_port=htons(PORT);
    srv_fd=socket(AF_INET, SOCK_STREAM, 0);
    signal(SIGCHLD,SIG_IGN);
    if(bind(srv_fd,(struct sockaddr*)&server,(socklen_t)sizeof(server))<0)
    {
        fputs("BIND FAILED!\n",stderr);
        return -1;
    }
    else
        setsockopt(srv_fd,SOL_SOCKET,SO_REUSEADDR,&sock_opt,(socklen_t)sizeof(int));
    if(listen(srv_fd, 0)==0)
    {
        while(1)
        {
                cli_fd=accept(srv_fd,(struct sockaddr*)&client,&len);
                switch(fork()){ //fork() to allow multiple requests at the same time
                    case 0:
                        proxy_handle(cli_fd);
                        close(cli_fd);
                        close(srv_fd);
                        return 0;
                    case -1:
                        close(cli_fd);
                        close(srv_fd);
                        return -1;
                    default:
                        break;
                }
        }
    }
}
int proxy_handle(int cli_fd)
{
    char *data;
    int lenght,conn_fd; //conn_fd describes socket used to connect to desired host
    struct request_info *req;
    struct addrinfo *srv;
    struct addrinfo crit;
    //getting request and parsing it
    data=data_in(cli_fd,&lenght);
    req=get_request_info(data,lenght);
    if(req==NULL)
    {
        free(data);
        free_request_info(req);
        return -1;
    }
    //resolving host
    crit.ai_family=AF_INET;
    crit.ai_socktype=SOCK_STREAM;
    crit.ai_protocol=0;
    crit.ai_flags=(AI_V4MAPPED | AI_ADDRCONFIG);
    crit.ai_addrlen=0;
    crit.ai_addr=NULL;
    crit.ai_canonname=NULL;
    crit.ai_next=NULL;
    if(getaddrinfo(req->host,req->port,&crit,&srv)!=0) //any errors are just meant to stop the proxy
    {
        return -1;
    }
    conn_fd=socket(AF_INET,SOCK_STREAM,0);
    if(connect(conn_fd,srv->ai_addr,sizeof(struct sockaddr))==-1) //connecting to desired host
    {
        free(data);
        return -1;
    }
    strcpy(data,req->method);
    strcat(data," ");
    strncat(data,req->resource,BUFFER_SIZE-strlen(data));
    strncat(data,req->headers,BUFFER_SIZE-strlen(data));
    free_request_info(req);
    data_out(conn_fd,data,strlen(data));
    while(data!=NULL)
    {
        data=data_in(conn_fd,&lenght);
        if(data_out(cli_fd,data,lenght)==-1)
            break;
    }
    free(data);
    close(conn_fd);
    return 1;
}
