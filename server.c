#define _POSIX_C_SOURCE 201712L
#define MAXLINE 8192
typedef struct sockaddr SA;
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


int open_listenfd(char *port) {
    struct addrinfo hints, *listp, *p;
    int listenfd, optval = 1;

    /* Get a list of potential server addresses */
    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_socktype = SOCK_STREAM; /* Accept connect. */
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG; /* …on any IP addr */
    hints.ai_flags |= AI_NUMERICSERV; /* …using numeric port no. */
    getaddrinfo(NULL, port, &hints, &listp);

    /* Walk the list for one that we can bind to */
    for(p = listp; p; p = p->ai_next) {
        /* Createa socketdescriptor*/
        if((listenfd= socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
            continue; /* Socket failed, try the next*/
        }

        /* Eliminates "Address already in use" errorfrom bind */
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));

        /* Bindthe descriptor to the address*/
        if(bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) {
            break; /* Success */
        }
        
        close(listenfd); /* Bind failed, try the next */
    }

    // clean up
    freeaddrinfo(listp);
    if (!p) {
        /* No address worked */
        return-1;
    }

    /* Make it a listening socket ready to accept conn. requests */
    // changed backlog argument from LISTENQ constant to 1
    if (listen(listenfd, 1) < 0) {
        // error occurred at listen() call
        close(listenfd);
        return -1;
    }

    return listenfd;
}

void echo(int connfd) {
    size_t n;
    char buf[MAXLINE];
    while((n = read(connfd, buf, MAXLINE)) != 0) {
        printf("server received %d bytes\n", (int)n);
        write(connfd, buf, n);
    }
}

int main(int argc, char **argv) {
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr; /* Enough room for any addr */
    char client_hostname[MAXLINE], client_port[MAXLINE];
    listenfd = open_listenfd(argv[1]);
    while (1) {
        clientlen = sizeof(struct sockaddr_storage); /* Important! */
        connfd = accept(listenfd, (SA *)&clientaddr, &clientlen);
        getnameinfo((SA *) &clientaddr, clientlen,
        client_hostname, MAXLINE, client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
        echo(connfd);
        close(connfd);
    }
    exit(0);
}