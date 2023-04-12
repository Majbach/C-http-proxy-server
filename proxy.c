#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "proxy.h"

char* data_in(int fd,int* len) //wrapper around read()
{
    char* content;
    int i=0,y=0;
    content=calloc(BUFFER_SIZE+1,sizeof(char));
    *len=read(fd,content,BUFFER_SIZE);
    if(*len<=0)
    {
        free(content);
        return NULL;
    }
    else
        return content;
}
int data_out(int fd, char* data, int len) //wrapper around write()
{
    switch(write(fd,data,len))
    {
        case -1:
            return -1;
        default:
            return 1;
    }
}
struct request_info* get_request_info(char* req, int len) //parsing information about request to struct request info, probably could be done better
{
    int y=0; //helper variable for while()
    struct request_info *ret;
    ret=malloc(sizeof(struct request_info));
    //getting method
    ret->method=calloc(16,sizeof(char));
    for(int i=0;i<16;i++)
    {
        ret->method[i]=req[i];
        if(ret->method[i]==' ')
        {
            ret->method[i]='\0';
            req+=i+1;
            break;
        }
    }
    //skipping protocol check, either way https does not work
    if(strncmp(req,"http://",strlen("http://"))==0)
        req+=strlen("http://");
    else
        return NULL;
    //getting host
    ret->host=calloc(1024,sizeof(char));
    for(int i=0;i<1024;i++)
    {
        switch(req[i])
        {
            case '\0':
                return NULL;
            case '/':
                ret->host[i]='\0';
                ret->port_in=0;
                ret->port=calloc(3,sizeof(char));
                strcpy(ret->port,"80");
                req+=i;
                i=1024;
                break;
            case ':':
                ret->host[i]='\0';
                req+=i+1;
                ret->port_in=1;
                i=1024;
                break;
            default: 
                ret->host[i]=req[i];
                break;
        }
    }
    //getting port number (depends on previous loop)
    if(ret->port_in==1)
    {
        ret->port=calloc(6,sizeof(char));
        for(int i=0;i<6;i++)
        {
            if(req[i]>='0'&&req[i]<='9')
                ret->port[i]=req[i];
        }
        req+=strlen(ret->port);
    }
    //getting URI
    while(req[y]!=' ') //determining how much space to allocate for URI
        y++;
    ret->resource=calloc(y+1,sizeof(char));
    for(int i=0;i<y+1;i++)
    {
        ret->resource[i]=req[i];
        if(ret->resource[i]==' ')
        {
            ret->resource[i]='\0';
            req+=i;
            break;
        }
    }
    ret->headers=calloc(strlen(req),sizeof(char));
    strcpy(ret->headers,req);
    return ret;
}
void free_request_info(struct request_info* r) //function to free memory allocated by get_request_info()
{
    free(r->method);
    free(r->host);
    free(r->port);
    free(r->resource);
    free(r->headers);
    free(r);
}
//probably dirty way to do a proxy it but it works
