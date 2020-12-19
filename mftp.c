/* * * * * * * * * * * * * * * * *
 * Vargha Hokmran  SID: 11543295 *
 * CS-360    Systems Programming *
 * Final Assignment         MFTP *
 * * * * * * * * * * * * * * * * */
#define PORT 49999
#define CMD_MAX 4096
#define RESP_MAX 256
#include "ClientHeader.h"
#include "ServerHeader.h"
#include "ClientFunctions.c"
#include "ServerFunctions.c"

int main(int argc, char* argv[])
{
  char* firstTok;                           // The first token of the cmd
  char* path;                               // The second token of the cmd
  firstTok =(char*)malloc(sizeof(char)*CMD_MAX);
  path =(char*)malloc(sizeof(char)*CMD_MAX);

  /* ********** ********** **************** ********** ********** */
  /* ********** **********      CLIENT      ********** ********** */
  /* ********** ********** **************** ********** ********** */
  if (argv[0][strlen(argv[0]) - 1]  == 't')   // the last character of "client"
  {
    if (argc < 2){
      printf("  Usage: ./mftp [-d] <hostname | IP address>\n");
      exit(1);
    }
    printf ("Running MFTP in \"Client\" mode\n");
    int socketfd = getSocket(argv[1], PORT);  // The main connection
    if (socketfd < 0)
      exit(1);
    char cmd[CMD_MAX] = "";

    while (strcmp(cmd, "exit\n") != 0)
    {
      printf("-> ");
      fgets(cmd, sizeof(char) * CMD_MAX, stdin);
      firstTok = strtok(cmd, " ");
      if (strcmp(firstTok, "cd\n") == 0) {
        printf ("  Usage: cd <directory>\n");
      } else if (strcmp(firstTok, "cd") == 0) {   /* cd */
        path = strtok(NULL, "");  
        clean (path);
        cd(path);
      } else if (strcmp(firstTok, "rcd\n") == 0) {/* rcd */
        printf ("  Usage: rcd <directory>\n");
      } else if (strcmp(firstTok, "rcd") == 0) {
        path = strtok(NULL, "");
        clean (path);
        rcd(path, socketfd);
      } else if (strcmp(firstTok, "ls\n") == 0) { /* ls */
        ls();
      } else if (strcmp(firstTok, "rls\n") == 0) {/* rls */
        if (rls(socketfd, argv[1]) < 0)
          printf("rls cmd Failure!\n");
      } else if ((strcmp(firstTok, "get\n")) == 0){
        printf ("  Usage: get <path>\n");
      } else if (strcmp(firstTok, "get") == 0){     /* get */
        path = strtok(NULL, "");
        clean (path);
        if (get(socketfd, path, argv[1]) < 0)
          printf("get cmd Failure!\n");
      } else if ((strcmp(firstTok, "show\n")) == 0){
        printf ("  Usage: show <path>\n");
      } else if (strcmp(firstTok, "show") == 0){  /* show */
        path = strtok(NULL, "");
        clean (path);
        if (show(socketfd, path, argv[1]) < 0)
          printf("show cmd Failure!\n");
      } else if (strcmp(firstTok, "put\n") == 0) {
        printf ("  Usage: put <path>\n");
      } else if (strcmp(firstTok, "put") == 0) {  /* put */
        path = strtok(NULL, "");
        clean (path);
        put (socketfd, path, argv[1]);
      } else if (strcmp(firstTok, "exit\n") == 0) {
        myExit(socketfd);
      } else
        printf ("  Improper command use!\n");
    }
  }
  /* ********** ********** **************** ********** ********** */
  /* ********** **********      SERVER      ********** ********** */
  /* ********** ********** **************** ********** ********** */
  else if (argv[0][strlen(argv[0]) - 1]  == 'r') // the last char of "server"
  {
    int datafd;
    printf ("Running MFTP in \"Server\" mode\n");
    char serverHostName[CMD_MAX] = "\0";
    char* clientHostName;
    gethostname(serverHostName, CMD_MAX-1);
    printf ("Server Host Name:  \"%s\"\n", serverHostName);

    int connectfd, listenfd;
    socklen_t length = sizeof(struct sockaddr_in);
    struct sockaddr_in clientAddr;
    struct hostent* hostEntry;
    listenfd = serverConnection(PORT);
    listen(listenfd, 4);

    while (1){                      // wait for a control connection request
      int status;
      waitpid(-1,&status ,0);       // Wait for 1 child die, per loop

      if ((connectfd = accept(listenfd,
          (struct sockaddr*) &clientAddr, &length)) < 0){
      perror("Problem in connection");
      exit(1);
      }
      if( fork() ){
        close(connectfd);
      } else{
        close(listenfd);
        break;
      } // end of forks
    }  // end of while

    if((hostEntry = gethostbyaddr(&(clientAddr.sin_addr),
        sizeof(struct in_addr), AF_INET)) == NULL)
      fprintf(stderr, "Error getting hostname");
    else{
      clientHostName = hostEntry->h_name;
      printf("Connection stablished to '%s'\n", clientHostName);
    }
    /* ***** Underlying Connection for Command Handling ***** */
    char cmdLetter[1];
    while(1){   // wait for client's port request
    read(connectfd, cmdLetter, 1);
//printf("Leader Char: '%c'\n", cmdLetter[0]);
      if(cmdLetter[0] == 'Q'){
        write(connectfd, "A\n", 2); // Acknowledge
        close(connectfd);
        printf("Quitting '%s'\n", clientHostName);
        exit(0);
      }
      else if (cmdLetter[0] == 'D'){// Establish data connection, return port
        datafd = dataConnection ( connectfd, cmdLetter );
        if( fork() ){
          close(datafd);
          wait(NULL);
        }else{
          break;
        }
      }
      else if (cmdLetter[0] == 'C'){// Change Directory
        readPath (path, connectfd);
        changeServerDir(connectfd, path, clientHostName);
      }
      else {    // Data connection needs to be stablished first
        write(connectfd, "EProblem with Data Connection\n", 30);
      }
    }
    /* ***** END of Underlying Connection for Commands ***** */

  /* ***** Command Handling ***** */
    while(1){
      int filefd, readBlock;
      char fileBuffer[CMD_MAX] = {0};
      char cmdLetter[1];
      memset (path, 0, CMD_MAX-1);// set all elements to 0
      char response[RESP_MAX] = {0};
      read(connectfd, cmdLetter, 1);

      if (cmdLetter[0] == 'L'){    // List Directory
        write(connectfd, "A\n", 2);
        read(connectfd, cmdLetter, 1);
          close(1); dup2(datafd, 1); close(datafd);  close(connectfd); // PIPE
          execlp("ls", "ls", "-l",  NULL);
      }
      else if(cmdLetter[0] == 'G'){    // Get a FIle
        readPath (path, connectfd);
        if((filefd = open(path, O_RDONLY)) < 0){
          sprintf(&response[0], "E%s\n", strerror(errno));
          write(  connectfd, response, strlen(response));
          exit(0);
        }
        if(isReg(path)){
          write(  connectfd, "A\n", 2);
          printf("Transferring '%s' to client\n", path);
          int writeBlocks;
          while((writeBlocks = read(filefd, fileBuffer, CMD_MAX)) > 0){
            write(datafd, fileBuffer, writeBlocks);
          }
          close(datafd);
          close(filefd);
          exit(0);
        }else{
          write(connectfd, "ENot a Regular File!\n", 21);
          close(datafd);
          close(filefd);
          exit(1);
        }  
      }
      else if(cmdLetter[0] == 'P'){
        readPath (path, connectfd);
        printf("Writing file %s\n", path);
        if((filefd = open(path, O_CREAT | O_EXCL | O_WRONLY, S_IXUSR, S_IRUSR 
    | S_IRGRP | S_IWGRP | S_IWUSR | S_IROTH)) < 0){
          sprintf(&response[0], "E%s\n", strerror(errno));
          write(  connectfd, response, strlen(response));
          close(datafd);
          exit(0);
        }
        write(connectfd, "A\n", 2);
        printf("Receiving file %s\n", path);
        while((readBlock = read(datafd, fileBuffer, CMD_MAX)) > 0){
          write(filefd, fileBuffer, readBlock);
        }
        close(filefd);
        close(connectfd);
        exit(0);
      }else{
        printf("Bad Command!\n");
        close(datafd);
        exit(0);
      }
    }    /* ***** END of Command Handling Structure ***** */

  }
  else
    printf("Please check the \"Readme.txt\" file about file names.\n");

  /* END OF THE PROGRAM (for both Server and CLient) */
  free (firstTok);// free memories before closing the program
  free (path);
}
