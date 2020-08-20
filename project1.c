#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <pthread.h>

#include <sys/stat.h> 
#include <sys/types.h>
#include <dirent.h>


/* Definations */
#define DEFAULT_BUFLEN 1024
#define PORT 1888
# define dirname "tmp/server/"

void PANIC(char* msg);
#define PANIC(msg)  { perror(msg); exit(-1); }

//my own
int validateuser(char username[], char password[]);

void openFileAndRead(int client, char cmd2[]){
     
    char dir[255] = "tmp/server/";
        strcat(dir,cmd2);
    
    FILE *selectedfile;
    selectedfile = fopen(cmd2, "r");
    char ch[255];

   
    
   
   
    if( selectedfile == NULL)
        send(client, "\n404 file not found\n", 20, 0);

    else{

     while((fgets(ch, sizeof(ch), selectedfile)) != NULL){
     int counter = 0;
      for(int i =0;ch[i]!='\0';i++){
          counter++;
      }
        counter++;
    send(client, ch, counter, 0);
    }
    send(client, "\n.\n", 3, 0);
   fclose(selectedfile);
   }

}

void listFiles(int client){
  
    DIR *dr;
    struct dirent *files;
    dr = opendir("./tmp/server");
    char *tempdata;
    while((files = readdir(dr)) != NULL ){
      tempdata = files->d_name;
       send(client,tempdata, 11, 0);
       send(client, "\n", 2, 0);
    }
    closedir(dr);
}

void getCommand(char cmd[], int client){
     // my own
     char *cmd1 = strtok(cmd, ". \n");
    if(strcmp(cmd1, "USER") == 0){
       char *cmd2 = strtok(NULL, " ");
            if(strcmp(cmd2, "umar") == 0){
                
                char *cmd3 = strtok(NULL, " .\n");
               
                if(strcmp(cmd3,"password") == 0){      
                    send(client, "200 User umar granted to access\n", 32, 0);
                }
            }
        else {
             send(client, "404 user not found", 13, 0);
                exit(1);
        }
    }
   
     if (strcmp(cmd,"LIST") == 0){
       
        listFiles(client);
        
    }
    else if (strcmp(cmd1,"GET") == 0){
       char *cmd2 = strtok(NULL, " \n");
        char dir[100];
        strcpy(dir,"tmp/server/");
        strcat(dir, cmd2);

        openFileAndRead(client, dir);
    }
}
/*--------------------------------------------------------------------*/
/*--- Child - echo server                                         ---*/
/*--------------------------------------------------------------------*/

void* Child(void* arg)
{   char line[DEFAULT_BUFLEN];
    int bytes_read;
    int client = *(int *)arg;

   
    
    

    do
    {

        bytes_read = recv(client, line, sizeof(line), 0);
        if (bytes_read > 0) {
            char temp;
            getCommand(line, client);
                
        } else if (bytes_read == 0 ) {
                printf("Connection closed by client\n");
                break;
        } else {
                printf("Connection has problem\n");
                break;
        }
    } while (bytes_read > 0);
    close(client);
    return arg;
}

/*--------------------------------------------------------------------*/
/*--- main - setup server and await connections (no need to clean  ---*/
/*--- up after terminated children.                                ---*/
/*--------------------------------------------------------------------*/
int main(int argc, char *argv[])
{  
    //my own
    int checkDir;
    char* dir1 = "tmp";
    char* dir2 = "tmp/server";
    mkdir(dir1, 0777);
    mkdir(dir2, 0777);
    


     int sd,opt,optval;
    struct sockaddr_in addr;
    unsigned short port=0;

    
    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
        case 'p':
                port=atoi(optarg);
                break;
        }
    }


    if ( (sd = socket(PF_INET, SOCK_STREAM, 0)) < 0 )
        PANIC("Socket");
    addr.sin_family = AF_INET;

    if ( port > 0 )
                addr.sin_port = htons(port);
    else
                addr.sin_port = htons(PORT);

    addr.sin_addr.s_addr = INADDR_ANY;

   // set SO_REUSEADDR on a socket to true (1):
   optval = 1;
   setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);


    if ( bind(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0 )
        PANIC("Bind");
    if ( listen(sd, SOMAXCONN) != 0 )
        PANIC("Listen");

    printf("welcome to Umar's port server\n");

    while (1)
    {
        int client, addr_size = sizeof(addr);
        pthread_t child;

        client = accept(sd, (struct sockaddr*)&addr, &addr_size);
        printf("Connected: %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        if ( pthread_create(&child, NULL, Child, &client) != 0 )
            perror("Thread creation");
        else
            pthread_detach(child);  /* disassociate from parent */
    }
    return 0;
}


int validateuser( char username[], char password[]){
    if(strcmp(password,"password")==0)
        return 1;
    return 0;
}
