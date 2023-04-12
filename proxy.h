//header file containing all definitions and functions for proxy.c and main.c
#ifndef PROXY
    #define PROXY
    #define PORT 8080
    #define BUFFER_SIZE 4096
    struct request_info{ //contains data returned by get_request_info();
        int port_in;
        char* method;
        char* host;
        char* port; 
        char* resource;
        char* headers; //just passing headers, i don't care what those are
    };
    char* data_in(int fd,int* len); //wrapper around read();
    int data_out(int fd,char* data,int len); //wrapper around write();
    struct request_info* get_request_info(char *req,int len); //identifies request data, returns NULL ptr on failure
    void free_request_info(struct request_info*);
#endif