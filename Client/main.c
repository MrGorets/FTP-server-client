#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include <fcntl.h>


#define SIZE_BUF 512
#define HOST "127.0.0.1" //ftp server COM
#define PORT 8888 //ftp server COM

int main(int argc, char *arvg[])
{

    char Buffer[SIZE_BUF];
    int sockCom = 0, sockData = 0;
    char check[128];
    int NoDebil = 0;
    int PortData;
    int AUTFLAG = 0;
    char HostData[15]="";
    struct sockaddr_in server, serverData;

    FILE *file;

    sockCom=socket(AF_INET,SOCK_STREAM,0);

    if(sockCom==-1)
        printf("Could not create socket");
    puts("Socket create");

    server.sin_family=AF_INET;
    server.sin_addr.s_addr=inet_addr(HOST);
    server.sin_port=htons(PORT);

    if(connect(sockCom,(struct sockaddr*)&server,sizeof(server))<0)
    {
        perror("Connect failed");
        return 1;
    }
    puts("Connect done");


    recv(sockCom, &Buffer, SIZE_BUF, 0);
    printf("%s\n", Buffer);

    memset(Buffer, 0, SIZE_BUF);
    gets( Buffer);
    if( strstr(Buffer, "USER") == NULL ){
        printf("Invalid command\n");
        return 1;
    }
    send(sockCom, &Buffer, SIZE_BUF, 0);

    recv(sockCom, &Buffer, SIZE_BUF, 0);
    printf("%s\n", Buffer);

    memset(Buffer, 0, SIZE_BUF);
    gets( Buffer);
    if( strstr(Buffer, "PASS") == NULL ){
        printf("Invalid command\n");
        return 1;
    }
    send(sockCom, &Buffer, SIZE_BUF, 0);

    recv(sockCom, &Buffer, SIZE_BUF, 0); //ANSWER PASS
    printf("%s\n", Buffer);

    if( strstr(Buffer, "230") != NULL ) //processing ANSWER PASS
    {
        AUTFLAG = 1;
    }
    int tmp = strlen("USER") + 1;
    for( int count = 0; count < strlen(Buffer) + 1; count++) {
        check[count] = Buffer[tmp++];
    }

    while(AUTFLAG == 1)
    {
        memset(Buffer, 0, SIZE_BUF);
        gets( Buffer);
        if( strstr(Buffer, "PWD") != NULL ){
            send(sockCom, &Buffer, SIZE_BUF, 0);

            recv(sockCom, &Buffer, SIZE_BUF, 0);
            printf("%s\n", Buffer);
        }
        if( strstr(Buffer, "SYST") != NULL ){
            send(sockCom, &Buffer, SIZE_BUF, 0);

            recv(sockCom, &Buffer, SIZE_BUF, 0);
            printf("%s\n", Buffer);
        }
        if( strstr(Buffer, "NOOP") != NULL ){
            send(sockCom, &Buffer, SIZE_BUF, 0);

            recv(sockCom, &Buffer, SIZE_BUF, 0);
            printf("%s\n", Buffer);
        }
        if( strstr(Buffer, "TYPE I") != NULL ){
            send(sockCom, &Buffer, SIZE_BUF, 0);

            recv(sockCom, &Buffer, SIZE_BUF, 0);
            printf("%s\n", Buffer);
        }
        if( strstr(Buffer, "TYPE A") != NULL ){
            send(sockCom, &Buffer, SIZE_BUF, 0);

            recv(sockCom, &Buffer, SIZE_BUF, 0);
            printf("%s\n", Buffer);
        }
        if( strstr(Buffer, "PASV") != NULL ){
            if (NoDebil==1){
                printf("Data channel has already\n");
                break;
            }
            send(sockCom, &Buffer, SIZE_BUF, 0);
            fcntl(sockCom, F_SETFL, O_NONBLOCK);
            system("sleep 1");
            recv(sockCom, &Buffer, SIZE_BUF, 0);
            printf("%s\n", Buffer);

            int f = 0, i=0;
            for (int count = 0; count<strlen(Buffer); count++){
                if (Buffer[count]=='('){
                    f = 1;
                }else if (Buffer[count]==','){
                    f = 0;
                }else if (f == 1){
                    HostData[i]=Buffer[count];
                    i-=-1;
                }
            }
            char Po[3];
            int len = strlen(Buffer), F=1, start=0, P1=0, P2=0;
            for (int count = len-3; count>0; count--){
                if (Buffer[count]==','){
                    if (F==0){
                        start=count+1;
                        count=-10;
                    }
                    F--;
                }
            }
            for (int count = start, i=0; count<len; count++, i++){
                if(count==len-1){
                    P2 = atoi(Po);
                }else if (Buffer[count]!=','){
                    Po[i]=Buffer[count];
                }else if (Buffer[count]==','){
                    i=-1;
                    P1 = atoi(Po);
                    memset(Po, 0, 3);
                }
            }
            PortData=(P1*256)+P2;

            sockData=socket(AF_INET,SOCK_STREAM,0);

            if(sockData==-1)
                printf("Could not create socket");
            puts("Socket create");

            serverData.sin_family=AF_INET;
            serverData.sin_addr.s_addr=inet_addr(HostData);
            serverData.sin_port=htons(PortData);

            if(connect(sockData,(struct sockaddr*)&serverData,sizeof(serverData))<0)
            {
                perror("Connect failed");
                return 1;
            }
            puts("Connect done");
            fcntl(sockData, F_SETFL, O_NONBLOCK);
            NoDebil=1;
        }
        if( strstr(Buffer, "LIST") != NULL ){
            send(sockCom, &Buffer, SIZE_BUF, 0);
            system("sleep 0.1");
            while(recv(sockCom, &Buffer, SIZE_BUF, 0) != 0){
                if( strcmp(Buffer, "END") == 0 ){
                    printf("226 Transfer complete.\n");
                    break;
                }
                printf("%s", Buffer);
                memset(Buffer, 0, SIZE_BUF);
            }
        }
        if( strstr(Buffer, "RETR") != NULL ){
            if (NoDebil==1) {
                send(sockCom, &Buffer, SIZE_BUF, 0);
                system("sleep 0.2");
                memset(Buffer, 0, SIZE_BUF);
                recv(sockCom, &Buffer, SIZE_BUF, 0);
                printf("%s", Buffer);


                char data[512] = "";

                while (strlen(data) == 0) {
                    recv(sockData, &data, 512, 0);
                }
                file = fopen(data, "w");
                while (recv(sockData, &data, 512, 0) != 0) {
                    if (strcmp(data, "END") == 0) {
                        printf("226 Transfer complete.\n");
                        break;
                    }
                    fputs(data, file);
                }
                fclose(file);
            }else{
                printf("No open data channel\n");
            }
        }

        if( strstr(Buffer, "STOR") != NULL ){
            if (NoDebil==1) {

            char data[512] = "";
            tmp = strlen("STOR") + 1;
            for( int count = 0; count < strlen(Buffer) + 1; count++) {
                check[count] = Buffer[tmp++];
            }


                file = fopen( check, "r" );
                if (file ==NULL){
                    printf("Error open file\n");
                    break;
                }
                send(sockCom, &Buffer, SIZE_BUF, 0);
                system("sleep 0.1");
                recv(sockCom, &Buffer, SIZE_BUF, 0);
                printf("%s\n", Buffer);
                int count = 0;
                for(count = 0; (tmp = getc(file)) != EOF; count++ ){
                    data[count] = (char)tmp;
                    if(count == 510){
                        data[count+1] = '\0';
                        send(sockData, &data, 512, 0);
                       count = 0;
                   }
                }
                data[count] = '\0';
                for (count = 0; count< strlen(data)+1; count++ ){
                    data[count]=data[count+1];
                }
                send(sockData, &data, 512, 0);

                snprintf(data, SIZE_BUF, "END");
                send(sockData, &data, SIZE_BUF, 0);
                fclose(file);

            system("sleep 0.1");
            recv(sockCom, &Buffer, SIZE_BUF, 0);
            printf("%s\n", Buffer);

            }else{
                printf("No open data channel\n");
            }
        }
        if( strstr(Buffer, "DELE") != NULL ){
            send(sockCom, &Buffer, SIZE_BUF, 0);
            system("sleep 0.1");
            recv(sockCom, &Buffer, SIZE_BUF, 0);
            printf("%s\n", Buffer);
        }
        if( strstr(Buffer, "HELP") != NULL ){
            send(sockCom, &Buffer, SIZE_BUF, 0);
            recv(sockCom, &Buffer, SIZE_BUF, 0);
            printf("%s\n", Buffer);
            memset(Buffer, 0, SIZE_BUF);
        }
        if( strstr(Buffer, "GORETS") != NULL ){
            send(sockCom, &Buffer, SIZE_BUF, 0);
            recv(sockCom, &Buffer, SIZE_BUF, 0);
            printf("%s\n", Buffer);
        }
        if( strstr(Buffer, "QUIT") != NULL ){
            send(sockCom, &Buffer, SIZE_BUF, 0);
            AUTFLAG = 0;
            NoDebil = 0;
            recv(sockCom, &Buffer, SIZE_BUF, 0); //ANSWER QUIT
            printf("%s\n", Buffer);
            break;
        }

    }

    close(sockCom);
    close(sockData);
    return 0;
}
