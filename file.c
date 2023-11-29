#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

// use compiler flag -lm 
#define MAXLINE 8192
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


int main(){
    char filename[50];
    
    read_file("MSFT.csv",msft_data);
    char* date = "2021-11-04";
    char result[20];
    read_price(date,msft_data, result);
    printf("MSFT Price: %s",result);
    read_file("TSLA.csv",tsla_data);
    char* date1 = "2021-11-04";
    read_price(date,tsla_data, result);
    printf("TSLA Price: %s",result);
    max_profit("2021-11-04","2021-11-17",msft_data,result);
    printf("Max Profit MSFT %s", result);
    max_profit("2021-11-04","2021-11-17",tsla_data,result);
    printf("Max Profit TSLA %s", result);
    return 0;
}
