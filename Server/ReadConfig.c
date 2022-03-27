//
// Created by gorets on 3/18/22.
//

#include <bits/types/FILE.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int ReadConfig(char Users[2][10][10], char *Dir, int* serv)
{


    //char Users[2][10][10]={{{""}}};
    int UsersCounter=0;
    //char Dir[50];
    int Port=0;

    FILE *file;

    char str[100];
    char *estr;



    file = fopen("Config.txt", "r");

    if (file == NULL) {
        printf ("Error open file Config.txt\n");
        return -1;
    }

    while (1)
    {
        estr = fgets (str,sizeof(str),file);

        if (estr == NULL) break;


        if (strncmp(str, "Fold=", 5)==0){

            for (int count = 0; count<strlen(str)-6; count++){

                Dir[count]=str[count+5];
            }
        }

        if (strncmp(str, "Port=", 5)==0){

            char tmp[5];
            for (int count = 0; count<strlen(str)-6; count++){
                tmp[count]=str[count+5];
            }

            Port = atoi(tmp);
        }

        if (strncmp(str, "USER=", 5)==0){

            int flag=0;
            int countP=0;
            for (int count = 0; count<strlen(str)-6; count++){
                if (str[count+5]=='-'){
                    flag=1;
                    count++;
                }
                if (flag==0){
                    Users[0][UsersCounter][count]=str[count+5];
                }
                if (flag==1){
                    Users[1][UsersCounter][countP]=str[count+5];
                    countP++;
                }

            }
            UsersCounter++;
        }
    }
    serv[0]=Port;
    serv[1]=UsersCounter;
    fclose(file);
    return 0;
}

