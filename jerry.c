#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
  char buf[256];
  int fd = open("/proc/mousehole", O_RDWR);
  char pass[256];

  /*When the user put "open", block uname opening fname 
    - open file If it's not from someone else
    - block opening dontopen If it's from notyou 
    
    argv[2]: uname // notyou
    argv[3]: fname // dontopen
    uid: 1004

    Pass_option: 1
*/
  if(strcmp(argv[1], "open") == 0){
    if(argv[3] == NULL){
        printf("You didn't put a file name. Do it again with a file name.\n");
    }else{
        if(argv[2] == NULL){
          printf("You didn't put a user name. Do it again with a user name.\n");
          read(fd, buf, 300);
          puts(buf);
        }else{
          strcpy(pass, argv[2]);
          strcat(pass, " one "); 
          strcat(pass, argv[3]);
          //Pass data to /proc/mousehole
	  printf("%s will open %s file\n", argv[2], argv[3]);
          write(fd, pass, strlen(pass));
        }
    }
  }

  /*When the user put "kill", kill uname
    - Don't kill If the process is made by notyou
    - Kill if the process is made by someone else

    argv[2]: uname // notyou
    uid: 1004

    Pass_option: 2
*/
  else if(strcmp(argv[1], "kill") == 0){
    if(argv[2] == NULL){ 
      printf("You didn't put a user name. Do it again with a user name.\n");
    }else{
      strcpy(pass, argv[2]);
      strcat(pass, " two");
      //Pass data to /proc/mousehole
      printf("It will help to protect %s's processes to kill\n", argv[2]); 
      write(fd, pass, strlen(pass));
    }
  }

  //When the user put "help"
  else{
    printf("\n%s Command is not found\n", argv[1]);
    printf("You can put only TWO commands.\n1. open (file_name) (user_name): It will help user_name to open the file\n2. kill (user_name): It will kill processes which are made by user_name");
  }

  close(fd);

  return 0;
}

