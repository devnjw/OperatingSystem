#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include <errno.h>
#define _CRT_SECURE_NO_WARNINGS

FILE *fd = NULL;

void option();
void blocking();
void preventing();
void stopBlocking();
void stopPreventing();
void displayInfo();

int fileopen();
int getUid(char *name);

int main(void)
{
	if(!fileopen())
		return -1;
	
	option();

	fclose(fd);

	return 0;
}

void option()
{
	int opt;
	printf("---This program provides five options---\n");
	printf("Option 1 - Block Certain User\n");
	printf("Option 2 - Prevent a Killing\n");
	printf("Option 3 - Stop Option 1 Blocking\n");
	printf("Option 4 - Stop Option 2 Preventing\n");
	printf("Option 5 - Display Information from Kurnel\n");
	printf("Select ( 1 / 2 / 3 / 4 / 5 ) : ");
	scanf("%d", &opt);

	switch(opt){
		case 1 :
			blocking();
			break;
		case 2 :
			preventing();
			break;
		case 3 :
			stopBlocking();
			break;
		case 4 :
			stopPreventing();
			break;
		case 5 :
			displayInfo();
			break;
		default :
			printf("Error: Input should be among 1 ~ 4\n\n");
			option();
	}
}

void blocking()
{
	char filepath[100];
	char name[50];
	printf("Enter username and filename to Block Opening : ");
	scanf("%s %s", name, filepath);
	
	int uid = getUid(name);
	if(uid==-1)
		return ;

	fprintf(fd, "1 %d %s", uid, filepath);
	printf("User %s(%d) is Blocked from Opening %s\n",name,uid,filepath);
}	
	
void preventing()
{
	char name[50];
	
	printf("Enter user name to prevent from process killing : ");
	scanf("%s", name);
	
	int uid = getUid(name);
	if(uid==-1)
		return ;
	fprintf(fd, "2 %d", uid);
	printf("Processes created by User %s(%d) will not be killed\n", name, uid);
}

void stopBlocking()
{
	fprintf(fd, "1 -1 -1");
	printf("---Block Opening function just Stopped---\n");
}

void stopPreventing()
{
	fprintf(fd, "2 -1");
	printf("---Prevent Killing function just Stopped---\n");
}

void displayInfo()
{
	/*fclose(fd);
	FILE * fr;
	if((fr = fopen("/proc/mousehole", "r")) == NULL);
	{
		printf("Proc File Open Failed.\n");
		return ;
	}*/
	fclose(fd);
	int fp = open("/proc/mousehole", O_RDWR);
	char buf[512];
	read(fp, buf,500);
	puts(buf);
	fileopen();
	//fclose(fr);
}

int getUid(char *name)
{
	struct passwd *p;
	if((p = getpwnam(name)) == NULL){
		perror(name);
		printf("Error: There is no user name '%s'\n",name);
		return -1;
	}
	return (int)p->pw_uid;
}

int fileopen()
{
	if((fd = fopen("/proc/mousehole", "r+")) == NULL)
	{
		printf("Proc File Open Failed.\n");
		return 0;
	}
	return 1;
}
