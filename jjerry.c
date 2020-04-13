#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#define _CRT_SECURE_NO_WARNINGS

char buf[256];
int max_len = 256;
char *user_name;
struct passwd *user_pw;
char command[256];
FILE *fp = NULL;

int main(int argc, char *argv[])
{
    int tmp;

    if (argc == 1)
    {

        printf("You can put only SIX commands.\n");
        printf("1. block username filename: It will block user from opening the file.\n");
        printf("2. protect username: It will kill processes which are made by user.\n");
        printf("3. stop_block username: It will stop blocking user to open the file.\n");
        printf("4. stop_protect username: It will stop preventing processes from killing.\n");
        printf("5. show mousehole: It will show up the information given from mousehole module\n");
        printf("6. help: help will print out the instruction\n");

        return -1;
    }
    else
    {
        //Read /proc/mousehole
        if (strcmp(argv[1], "show") == 0)
        {
            fp = fopen("/proc/mousehole", "r");

            if (fp == NULL)
            {
                printf("Access failed from /proc/mousehole \n");
                return -1;
            }

            printf("The information from /proc/mousehole\n\n");
            while (fgets(buf, max_len, fp) != NULL)
            {
                printf("%s", buf);
            }

            fclose(fp);
            printf("\n\n");
        }

        //Print help message
        else if (strcmp(argv[1], "help") == 0)
        {

            printf("You can put only SIX commands.\n");
            printf("block username filename: It will block user from opening the file.\n");
            printf("protect username: It will kill processes which are made by user.\n");
            printf("stop_block: It will stop blocking the user to open the file.(the user)\n");
            printf("stop_protect: It will stop preventing a killing processes made by the user.(The user is the most recent user which you put)\n");
            printf("show mousehole: It will show up the information given from mousehole module\n");
            printf("help: help will print out the instructions\n");

            return -1;
        }
        //Block the user from opening the file
        else if (strcmp(argv[1], "block") == 0)
        {
            if (argv[3] == NULL)
            {
                printf("You didn't put a file name. Do it again with a file name.\n");
                return -1;
            }
            else
            {
                if (argv[2] == NULL)
                {
                    printf("You didn't put a user name. Do it again with a user name.\n");
                    return -1;
                }
                else
                {
                    //Pass data to /proc/mousehole
                     fp = fopen("/proc/mousehole", "w");

            if (fp == NULL)
            {
                printf("Access failed from /proc/mousehole \n");
                return -1;
            }

            user_name = argv[2];           //get user_name
            user_pw = getpwnam(user_name); //get user_info by using user_name

                    printf("%s will open %s file with uid %d\n", user_pw->pw_name, argv[3], user_pw->pw_uid);
                    fprintf(fp, "1 %d %s", user_pw->pw_uid, argv[3]);
                    fclose(fp);
                    return -1;
                }
            }
        }
        //Protect the user from killing
        else if (strcmp(argv[1], "protect") == 0)
        {

            if (argv[2] == NULL)
            {
                printf("You didn't put a user name. Do it again with a user name.\n");
                return -1;
            }
            else
            {
                
            fp = fopen("/proc/mousehole", "w");

            if (fp == NULL)
            {
                printf("Access failed from /proc/mousehole \n");
                return -1;
            }

            user_name = argv[2];           //get user_name
            user_pw = getpwnam(user_name); //get user_info by using user_name

                //Pass data to /proc/mousehole
                printf("It will help to protect %s's processes to kill with uid %d\n", argv[2], user_pw->pw_uid);
                fprintf(fp, "2 %d", user_pw->pw_uid);
                fclose(fp);
                return -1;
            }
        }
        else if (strcmp(argv[1], "stop_block") == 0)
        {

            fp = fopen("/proc/mousehole", "w");

            if (fp == NULL)
            {
                printf("Access failed from /proc/mousehole \n");
                return -1;
            }

            fprintf(fp, "3");
            fclose(fp);
            return -1;
        }
        else if (strcmp(argv[1], "stop_protect") == 0)
        {

            fp = fopen("/proc/mousehole", "w");

            if (fp == NULL)
            {
                printf("Access failed from /proc/mousehole \n");
                return -1;
            }
 
            fprintf(fp, "4");
            fclose(fp);
            return -1;
        }else{
            printf("\n%s Command is not found\n", argv[1]);
            printf("Enter the 'help' command to see commands you can use.\n");
        }
    }
}
