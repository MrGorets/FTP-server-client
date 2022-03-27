#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <pthread.h>
#include <sys/fcntl.h>


#include "ReadConfig.h"

#define SIZE_BUF 512
#define PortDP 55555 //ftp Data channel open
#define HOST "127.0.0.1" //ftp Data channel open

int main(int argc,char *argv[])
{
    char Users[2][10][10]={{{""}}};
    char Dir[50]={""};
    int serv[2];
    ReadConfig (Users, Dir, serv);
    int Port=serv[0];
    int UsersCounter=serv[1];

    char Check[128];
    int LFLAG = 0;
    int AUTFLAG = 0;
    int LoginN;

    char Buffer[SIZE_BUF] = {""};

    int socket_desc, c, client_sock, socketD, sock_data;
    struct sockaddr_in server, client, servedD={0};

    socket_desc=socket(AF_INET,SOCK_STREAM,0);
    if(socket_desc==-1)
    {
        printf("Could not create socket");
        return 1;
    }
    puts("Socket create");


    server.sin_family=AF_INET;
    server.sin_addr.s_addr=INADDR_ANY;
    server.sin_port=htons(Port);
    if(bind(socket_desc,(struct sockaddr*)&server,sizeof(server))<0)
    {
        perror("Bind failed");
        return 1;
    }
    puts("Bind done");
    listen(socket_desc,100);
    puts("Waiting for incoming connections...");



        c = sizeof(struct sockaddr_in);
        client_sock = accept(socket_desc, (struct sockaddr *) &client, (socklen_t *) &c);

        if (client_sock < 0) {
            perror("Accept failed");
            return 1;
        }
        puts("Connection accept");

///  command processing

        memset(Buffer, 0, SIZE_BUF);
        snprintf(Buffer, SIZE_BUF, "220 Welcome to the FTP Server)\n");
        send(client_sock, &Buffer, SIZE_BUF, 0);
        memset(Buffer, 0, SIZE_BUF);
        recv(client_sock, &Buffer, SIZE_BUF, 0);

        if (strstr(Buffer, "USER") != NULL) {
            memset(Check, 0, 128);
            int tmp = strlen("USER") + 1;
            for (int count = 0; count < strlen(Buffer) + 1; count++) {
                Check[count] = Buffer[tmp++];
                if (Buffer[count + 1] == 32) {
                    count = strlen(Buffer) + 10;
                }
            }

            for (int count = 0; count < UsersCounter; count++) {
                if (strcmp(Users[0][count], Check) == 0) {
                    LoginN = count;
                    LFLAG = 1;
                }
            }


            if (LFLAG) {

                memset(Buffer, 0, SIZE_BUF);
                snprintf(Buffer, SIZE_BUF, "331 Login ok. Send your password.");
                send(client_sock, &Buffer, SIZE_BUF, 0);
                memset(Buffer, 0, SIZE_BUF);

                while (strlen(Buffer)==0) {
                    recv(client_sock, &Buffer, SIZE_BUF, 0);
                }

                if (strstr(Buffer, "PASS") != NULL) {
                    memset(Check, 0, 128);
                    tmp = strlen("PASS") + 1;
                    for (int count = 0; count < strlen(Buffer) + 1; count++) {
                        Check[count] = Buffer[tmp++];
                        if (Buffer[count + 1] == 32) {
                            count = strlen(Buffer) + 10;
                        }
                    }

                    if (strcmp(Users[1][LoginN], Check) == 0) {
                        printf("%s%s%s%s\n", "Successful authorization: ", Users[0][LoginN], "-", Users[1][LoginN]);
                        AUTFLAG = 1;
                    } else {
                        snprintf(Buffer, SIZE_BUF, "530 Not logged in");
                        send(client_sock, &Buffer, SIZE_BUF, 0);
                        printf("Authorization fail\n");
                    }
                }
            }
        }
        //Authorization

        if (AUTFLAG == 1) {
            snprintf(Buffer, SIZE_BUF, "230 Guest login ok. Access restrictions apply.\n");
            send(client_sock, &Buffer, SIZE_BUF, 0);

            while (AUTFLAG == 1) {
                memset(Buffer, 0, SIZE_BUF);
                while (strlen(Buffer)==0) {
                    recv(client_sock, &Buffer, SIZE_BUF, 0);
                }
                printf("%s\n", Buffer);

                if (strstr(Buffer, "SYST") != NULL) {
                    snprintf(Buffer, SIZE_BUF, "215 UNIX Type: L8");
                    send(client_sock, &Buffer, SIZE_BUF, 0);
                }
                if (strstr(Buffer, "PASV") != NULL) {
                    fcntl(client_sock, F_SETFL, O_NONBLOCK);
                    int P1, P2;
                    P1 = PortDP / 256;
                    P2 = PortDP - (256 * P1);
                    snprintf(Buffer, SIZE_BUF, "227 Entering Passive Mode (%s,%d,%d)", HOST, P1, P2);


                    socketD = socket(AF_INET, SOCK_STREAM, 0);
                    if (socketD == -1) {
                        printf("Could not create socket Data");
                    }
                    servedD.sin_family = AF_INET;
                    servedD.sin_addr.s_addr = INADDR_ANY;
                    servedD.sin_port = htons(PortDP);
                    if (bind(socketD, (struct sockaddr *) &servedD, sizeof(servedD)) < 0) {
                        perror("Bind failed Data");
                        return 1;
                    }
                    puts("Bind done Data");
                    listen(socketD, 100);
                    puts("Waiting for incoming connections Data...");
                    send(client_sock, &Buffer, SIZE_BUF, 0);
                    c = sizeof(struct sockaddr_in);
                    sock_data = accept(socketD, (struct sockaddr *) &servedD, (socklen_t *) &c);
                    fcntl(sock_data, F_SETFL, O_NONBLOCK);
                    if (sock_data < 0) {
                        perror("Accept failed Data");
                        return 1;
                    }
                    puts("Connection accept Data");
                }
                if (strstr(Buffer, "NOOP") != NULL) {
                    snprintf(Buffer, SIZE_BUF, "250 Request was successful.");
                    send(client_sock, &Buffer, SIZE_BUF, 0);
                }
                if (strstr(Buffer, "PORT") != NULL) {
                    snprintf(Buffer, SIZE_BUF, "201 Command not supported.");
                    send(client_sock, &Buffer, SIZE_BUF, 0);

                }
                if (strstr(Buffer, "TYPE") != NULL) {
                    char tmp = Buffer[5];
                    if (tmp == 'I') {
                        snprintf(Buffer, SIZE_BUF, "200 Switching to Binary mode.");
                    } else if (tmp == 'A') {
                        snprintf(Buffer, SIZE_BUF, "200 Switching to ASCII mode.");
                    } else {
                        snprintf(Buffer, SIZE_BUF, "504 Command not implemented for that parameter.");
                    }
                    send(client_sock, &Buffer, SIZE_BUF, 0);

                }
                if (strstr(Buffer, "PWD") != NULL) {
                    memset(Buffer, 0, SIZE_BUF);
                    snprintf(Buffer, SIZE_BUF, "257 \"%s\"", Dir);
                    send(client_sock, &Buffer, SIZE_BUF, 0);
                }
                if (strstr(Buffer, "LIST") != NULL) {

                    FILE *file;
                    char str[64]="";

                    snprintf(str, 64, "ls -l %s | tail -n+2 > tmp.txt", Dir);
                    int rs = system(str);
                    if (rs < 0) {
                        exit(1);
                    }
                    file = fopen("tmp.txt", "r");


                    fseek(file, SEEK_SET, 0);
                    memset(Buffer, 0, 64);
                    snprintf(Buffer, SIZE_BUF, "150 In progress...\n");
                    send(client_sock, &Buffer, SIZE_BUF, 0);


                    char data[2048] = {""};
                    char *ptr = data;
                    memset(data, 0, 2048);

                    ptr = fgets(data, sizeof(data), file);
                    system("sleep 0.1");

                    while (ptr != NULL) {
                        printf("%s", data);
                        int siZe = strlen(data);
                        system("sleep 0.1");
                        if (send(client_sock, &data, siZe, 0) < 0) {
                            perror("err");
                        }
                        memset(data, 0, 2048);
                        ptr = fgets(data, sizeof(data), file);
                    }
                    fclose(file);

                    system("sleep 0.1");
                    memset(Buffer, 0, SIZE_BUF);
                    snprintf(Buffer, SIZE_BUF, "END");
                    send(client_sock, &Buffer, SIZE_BUF, 0);


                }
                if (strstr(Buffer, "RETR") != NULL) {
                    char data[512]="";
                    FILE *file;
                    char file_name[128];

                    memset(Check, 0, 128);
                    int tmp = strlen("RETR") + 1;
                    for (int count = 0; count < strlen(Buffer) + 1; count++) {
                        Check[count] = Buffer[tmp++];
                    }

                    memset(Buffer, 0, SIZE_BUF);
                    snprintf(Buffer, SIZE_BUF, "150 In progress...\n");
                    send(client_sock, &Buffer, SIZE_BUF, 0);
                    system("sleep 0.1");


                            strcpy(data, Check);
                            send(sock_data, &data, SIZE_BUF, 0);

                            strcpy(file_name, Dir);
                            strcat(file_name, "/");
                            strcat(file_name, Check);

                            file = fopen(file_name, "r");
                            if (file ==NULL){
                                printf("Error open file\n");
                                memset(Buffer, 0, SIZE_BUF);
                                snprintf(Buffer, SIZE_BUF, "404 File not found\n");
                                send(client_sock, &Buffer, SIZE_BUF, 0);
                                break;
                            }

                            int count=0;
                            for (count = 0; (tmp = getc(file)) != EOF; count++) {
                                data[count] = (char) tmp;
                                if (count == SIZE_BUF - 2) {
                                    send(sock_data, &data, SIZE_BUF, 0);
                                    printf("%s", data);
                                    count = 0;
                                }
                            }
                            data[count] = '\0';
                            printf("%s", data);
                            send(sock_data, &data, SIZE_BUF, 0);

                            snprintf(data, SIZE_BUF, "END");
                            send(sock_data, &data, SIZE_BUF, 0);
                            fclose(file);


                }
                if (strstr(Buffer, "STOR") != NULL) {
                    FILE *file;
                    char file_name[128];
                    char data[512]="";
                    int tmp = strlen("STOR") + 1;
                    memset(Check, 0, 128);
                    for (int count = 0; count < strlen(Buffer) + 1; count++) {
                        Check[count] = Buffer[tmp++];
                    }

                    memset(Buffer, 0, SIZE_BUF);
                    snprintf(Buffer, SIZE_BUF, "150 In progress...\n");
                    send(client_sock, &Buffer, SIZE_BUF, 0);
                    system("sleep 0.1");


                        strcpy(file_name, Dir);
                        strcat(file_name, "/");
                        strcat(file_name, Check);

                        file = fopen(file_name, "w");
                        while (recv(sock_data, &data, 512, 0) != 0) {
                            if (strcmp(data, "END") == 0) {
                                printf("Recive is over.\n");
                                break;
                            }
                            fputs(data, file);
                            printf("%s", data);
                        }
                        fclose(file);


                    memset(Buffer, 0, SIZE_BUF);
                    snprintf(Buffer, SIZE_BUF, "226 Transfer complete.\n");
                    send(client_sock, &Buffer, SIZE_BUF, 0);
                }
                if (strstr(Buffer, "DELE") != NULL) {
                    char file_name[128];
                    int tmp = strlen("DELE") + 1;
                    for (int count = 0; count < strlen(Buffer) + 1; count++) {
                        Check[count] = Buffer[tmp++];
                    }
                    printf("REMOVE: %s!!!\n", Check);
                    strcpy(file_name, Dir);
                    strcat(file_name, "/");
                    strcat(file_name, Check);
                    if (remove(file_name)){
                        snprintf(Buffer, SIZE_BUF, "450 File not available.\n");
                        send(client_sock, &Buffer, SIZE_BUF, 0);
                    }else{
                        snprintf(Buffer, SIZE_BUF, "226 REMOVE '%s' complete.\n", Check);
                        send(client_sock, &Buffer, SIZE_BUF, 0);
                    }
                }
                if (strstr(Buffer, "GORETS") != NULL) {
                    snprintf(Buffer, SIZE_BUF, "434 Don't touch Mr! He is angry! Get out, get out of here. That never came back! ARGH.\n");
                    send(client_sock, &Buffer, SIZE_BUF, 0);
                }
                if (strstr(Buffer, "HELP") != NULL) {
                    snprintf(Buffer, SIZE_BUF, "214 The following commands are recognized:\nCWD      QUIT      PASV      PWD      SYST      HELP      LIST\nNOOP     TYPE      RETR      STOR     USER      PASS      DELE\n");
                    send(client_sock, &Buffer, SIZE_BUF, 0);
                    memset(Buffer, 0, SIZE_BUF);
                }
                if (strstr(Buffer, "QUIT") != NULL) {
                    snprintf(Buffer, SIZE_BUF, "221 Quit succesful.");
                    send(client_sock, &Buffer, SIZE_BUF, 0);
                    AUTFLAG = 0;
                    break;
                }

            }
        }//Jest`



    close(sock_data);
    close(socket_desc);
    return 0;
}