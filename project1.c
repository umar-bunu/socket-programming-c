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
int ports;
char dirName[100];

void PANIC(char* msg);
#define PANIC(msg)  { perror(msg); exit(-1); }

//my own
int validateuser(char username[], char password[]);

void putFile(int client, char cmd2[], char line[]){
    char message[150];
    FILE *fp;
    struct stat st;
    char buf[8];

     char slash = '/';
         char dir[100];
        strcpy(dir,dirName);
          strncat(dir, &slash, 1);
        strcat(dir, cmd2);
    fp = fopen(dir, "w");
    fputs(line, fp);
    fclose(fp);
    if(stat(dir, &st) == 0)
            sprintf(buf, "%ld", st.st_size);
    strcpy(message,"200 ");
    strcat(message, buf);
    strcat(message," Byte ");
    strcat(message,cmd2);
    strcat(message, " file retrieved by server and was saved.\n");
    int counter = 0;
    for(int i=0;message[i]!='\0';i++){
          counter++;
      }
      counter++;
    send(client, message, counter, 0);
   

}

void openFileAndRead(int client, char cmd2[]){
     
    
    
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
    
    struct stat st;
    DIR *dr;
    struct dirent *files;
    dr = opendir(dirName);
    char tempdata[100];
    while((files = readdir(dr)) != NULL ){
        int counter = 0;
        char buf[8];
        char slash = '/';
        strcpy(tempdata,files->d_name);
        char fileLocation[100];
        strcpy(fileLocation,dirName);
        strncat(fileLocation, &slash, 1);
        strcat(fileLocation,files->d_name);
        
        if(stat(fileLocation, &st) == 0)
            sprintf(buf, "%ld", st.st_size);
        strcat(tempdata," ");
        strcat(tempdata, buf);
        
    
      

      for(int i=0;tempdata[i]!='\0';i++){
          counter++;
      }
      counter++;
       send(client,tempdata, counter, 0);
       send(client, "\n", 2, 0);
    }
    send(client, ".\n", 2, 0);
    closedir(dr);
}

void getCommand(char cmd[], int client, int *isLoggedin){
     // my own
     char message[150];
     char *cmd1 = strtok(cmd, ". \n");
    if(strcmp(cmd,"QUIT") == 0){
        send(client,"Goodbye!\n", 9, 0);
        exit(0);
    }
  
  
    if(strcmp(cmd1, "USER") == 0){
       char *cmd2 = strtok(NULL, " ");
            if(strcmp(cmd2, "umar") == 0){
                
                char *cmd3 = strtok(NULL, " .\n");
               
                if(strcmp(cmd3,"password") == 0){      
                    *isLoggedin = 1;
                    send(client, "200 User umar granted to access\n", 32, 0);
                }
            }
        else {
             send(client, "404 user not found. Please try with another user\n", 19, 0);
                return;
        }
    }
    else if(!*isLoggedin){
           send(client, "user not logged in\n",19, 0);
           return;
       }
    else if (strcmp(cmd,"LIST") == 0){
       
        listFiles(client);
        
    }
    else if(strcmp(cmd1,"GET") == 0){
       char *cmd2 = strtok(NULL, " \n");
        char dir[100];
        strcpy(dir,dirName);
        strcat(dir, cmd2);

        openFileAndRead(client, dir);
    }
    else if(strcmp(cmd,"PUT") == 0){
        char *cmd2 = strtok(NULL, " \n");
        char line[DEFAULT_BUFLEN];
        int bytes_read;
        recv(client, line, sizeof(line), 0);
        putFile(client, cmd2, line);
    }
    else if(strcmp(cmd,"DEL") == 0){
        char *cmd2 = strtok(NULL, " \n");
        char slash = '/';
         char dir[100];
        strcpy(dir,dirName);
          strncat(dir, &slash, 1);
        strcat(dir, cmd2);
        int confirm = remove(dir);
        if(confirm){
            send(client,"404 FILE is not on the server\n", 30, 0);
            return;
        }
        strcpy(message,"200 File ");
        strcat(message,cmd2);
        strcat(message, " deleted\n");
        int counter = 1;
        for(int i = 0;message[i]!='\0';i++)
            counter++;
        
        send(client, message, counter, 0);
    }
   
    else {
        send(client, "Invalid command!\n", 17, 0);
    }
}
/*--------------------------------------------------------------------*/
/*--- Child - echo server                                         ---*/
/*--------------------------------------------------------------------*/

void* Child(void* arg)
{   char line[DEFAULT_BUFLEN];
    int bytes_read;
    int client = *(int *)arg;
    int isLoggedin = 0;
    send(client, "welcome to Umar's file server\n", 30, 0);
    
    
    

    do
    {

        bytes_read = recv(client, line, sizeof(line), 0);
        if (bytes_read > 0) {
            char temp;
            
            getCommand(line, client, &isLoggedin);
                
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
   
  
    for(int i=0;argv[1][i]!='\0';i++)
        strncat(dirName, &argv[1][i], 1);
    ports = atoi(argv[2]);
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
                addr.sin_port = htons(ports);

    addr.sin_addr.s_addr = INADDR_ANY;

   // set SO_REUSEADDR on a socket to true (1):
   optval = 1;
   setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);


    if ( bind(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0 )
        PANIC("Bind");
    if ( listen(sd, SOMAXCONN) != 0 )
        PANIC("Listen");

    printf("File server listening on localhost port %d\n", ports);

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

//MY OWN
int validateuser( char username[], char password[]){
    if(strcmp(password,"password")==0)
        return 1;
    return 0;
}
