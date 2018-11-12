#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

typedef struct users {
    char username[50];
    char password[50];
} Users;

void removeNewLine(char *text){
    int len = 0;
    len=strlen(text);
    if(text[len-1]=='\n')
        text[len-1]='\0';
}


bool login (){
    Users u;

    system("clear");
    printf("\n*********************[LOGIN]**************************\n");

    printf("Username: ");
    fgets(u.username, sizeof(u.username), stdin);
    removeNewLine(u.username);

    printf("Password: ");
    fgets(u.password, sizeof(u.password), stdin);
    removeNewLine(u.password);
    
    if (strcmp(u.username, "admin") == 0 && strcmp(u.password, "12345") == 0){
        printf("*******************[ACCESS GRANTED]********************\n");
        sleep(2);
    }else {
        printf("Username or password invalid!\n");
        sleep(2);
        login();

    }

}