// CMU's csapp.h header file 
// http://csapp.cs.cmu.edu/2e/ics2/code/include/csapp.h

// sometimes it says addrinfo is an incomplete type and this fixes it...
#define _POSIX_C_SOURCE 201712L
#define MAXLINE 8192
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


int open_clientfd(char *hostname, char *port) {
    int clientfd;
    struct addrinfo hints, *listp, *p;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM; // Open a connection 
    hints.ai_flags = AI_NUMERICSERV; // …using numeric port arg. 
    hints.ai_flags |= AI_ADDRCONFIG; // Recommended for connections 
    getaddrinfo(hostname, port, &hints, &listp);


    for(p = listp; p; p = p->ai_next) {
        //try socket
        if((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
            continue; 
        }

        //server connect
        if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1) {
            break; 
        }
            
        close(clientfd); //connect failed
    }

    //clean up
    freeaddrinfo(listp);
    if (!p){
         //all connects failed
         return-1;
    } else {
        //connect success
        return clientfd;
    }
}

int main(int argc, char **argv) {
    int clientfd;

    char *host, *port, buf[MAXLINE];
    host = argv[1];
    port = argv[2];

    clientfd= open_clientfd(host, port);

    while(fgets(buf, MAXLINE, stdin) != NULL) {
        write(clientfd, buf, strlen(buf));
        if (strcmp(buf, "quit") == 0){
            break;
        }
        read(clientfd, buf, MAXLINE);
        fputs(buf, stdout);
    }

    close(clientfd);
    exit(0);
}