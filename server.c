// CMU's csapp.h header file 
// http://csapp.cs.cmu.edu/2e/ics2/code/include/csapp.h

// sometimes it says addrinfo is an incomplete type and this fixes it...
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
//code from file.c
#define MAX_SIZE 300
#define MAX_COLS 7
#define MAX_ROWS 301
struct Data{
    char date[11];
    float price;
};
struct Data msft_data[MAX_SIZE];
struct Data tsla_data[MAX_SIZE];

void read_file(char* filename, struct Data* file_data){
    FILE *file;
    char line[MAXLINE];
    file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    int row = 0;
    int col = 0;
    while (fgets(line, sizeof(line), file) && row < MAX_ROWS) {
        char *token;
        col = 0;
        token = strtok(line, ",");

        while (token != NULL && col < MAX_COLS) {
            //ignore first row
            if (row>0){
                //we only need date and final stock price
                if(col==0){
                    strncpy(file_data[row-1].date, token, 10);
                    file_data[row-1].date[10] = '\0';
                }
                if (col==4){
                    float price = atof(token);
                    //price = roundf(price*100)/100;
                    file_data[row-1].price = price;
                }
            }
            token = strtok(NULL, ",");
            col++;
        }
        row++;
    }
    fclose(file);
    return;
}
void read_price(char* date, struct Data* file_data, char* result){

    for(int i=0; i<MAX_SIZE; i++){
        if(strcmp(date,file_data[i].date)==0){
            snprintf(result, sizeof(result), "%.2f\n", file_data[i].price);
            return;
        }
    }
    result = "Date not found\n";
}
void max_profit(char* start_date, char* end_date, struct Data* file_data, char* result){
    float max_profit = 0;
    for(int i=0; i<MAX_SIZE; i++){
        if(strcmp(file_data[i].date,start_date)>=0 && strcmp(end_date, file_data[i].date)>=0){
            for(int j=i+1; j<MAX_SIZE; j++){
                if(strcmp(file_data[j].date,start_date)>=0 && strcmp(end_date, file_data[j].date)>=0){
                    if(file_data[j].price >= file_data[i].price){
                        float final_difference = file_data[j].price - file_data[i].price;
                        if(final_difference>=max_profit){
                            max_profit = final_difference;
                        }
                    }

                }
                
            }
        }
    }
    snprintf(result, sizeof(result), "%.2f\n", max_profit);
}

//---------------------------------

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
    char* result;
    char* token;
    while((n = read(connfd, buf, MAXLINE)) != 0) {
        printf("server received %d bytes\n", (int)n);
        printf("%s\n", buf);
        buf[strcspn(buf, "\n")] = 0;
        token = strtok(buf, " ");
        if (strcmp(token, "quit") == 0){
            close(connfd);
            return;
        }
        else if(strcmp(token, "List")){
            //we probably shouldn't hardcode this- it's best to split argv but this works for now
            result = "TSLA | MSFT\n";
        }
        else if(strcmp(token, "Prices")){
            token = strtok(NULL, " ");
            char* filename;
            char output[20];
            if (strcmp(token, "MSFT")==0){
                token = strtok(NULL, " ");
                char* date = token;
                read_price(date, msft_data, output);
            }
            else if (strcmp(token, "TSLA")==0){
                token = strtok(NULL, " ");
                char* date = token;
                read_price(date, tsla_data, output);
            }
            strcpy(output,result);
        }
        else if(strcmp(token, "MaxProfit")){
            token = strtok(NULL, " ");
            char* file;
            strcpy(file, token);
            char* filename;
            char output[20];
            char* start_date;
            char* end_date;
            token = strtok(NULL, " ");
            strcpy(start_date, token);
            token = strtok(NULL, " ");
            strcpy(end_date, token);

            if(strcmp(file, "MSFT")==0){
                max_profit(start_date, end_date, msft_data, output);
            }
            else if(strcmp(file, "TSLA")==0){
               max_profit(start_date, end_date, tsla_data, output);
            }
            strcpy(output,result);
        }
        write(connfd, result, sizeof(result));
    }
}
void initial(){
    read_file("MSFT.csv", msft_data);
    read_file("TSLA.csv", tsla_data);
}

int main(int argc, char **argv) {
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr; /* Enough room for any addr */
    char client_hostname[MAXLINE], client_port[MAXLINE];
    listenfd = open_listenfd(argv[1]);
    initial();
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