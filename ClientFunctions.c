
/* ***** Remove \n from the end ***** */
void clean (char *path){
  for(int i=0; i<strlen(path); i++)
    if (path[i] == '\n')
      path[i] = '\0';
}

/* ***** Connect to server and return the socket ***** */
int getSocket(char *serverHost, int myPort){
  struct sockaddr_in servAddr;
  int socketfd = socket( AF_INET, SOCK_STREAM, 0);
  if (socketfd < 0){
    perror("socketfd");
    return -1;
  }

  memset(&servAddr, 0, sizeof(servAddr)+1);
  servAddr.sin_family = AF_INET;
  servAddr.sin_port = htons(myPort);

  struct hostent* hostEntry;
  struct in_addr **pptr;
  hostEntry = gethostbyname(serverHost);
  /* this is magic, unless you want to dig into the man pages */
  pptr = (struct in_addr**) hostEntry->h_addr_list;
  memcpy(&servAddr.sin_addr, *pptr, sizeof(struct in_addr));

  if ((connect(socketfd, (struct sockaddr*) &servAddr, sizeof(servAddr)+1)) < 0){
    printf("Unable to connect to Server\n");
    return -1;
  }
  return socketfd;
}

/* ***** cmd = KeyChar + Path ***** */
int myStrCat(char c, char* cmd, char* path){
  cmd [0] = c;
  int i;
  for (i=1; i<=strlen(path); i++)
    cmd[i] = path[i-1];
  cmd[i] = '\n';
  return i+1;     // i is strlen
}

/* ***** Exit ***** */
void myExit(int socketfd){
  write(socketfd, "Q\n", 2);
  exit(0);
}

/* ***** rls ***** */
int rls(int socketfd, char* serverHost){
  char response[RESP_MAX] = {0};
  int myPort, rlsSock, pid;
  if((myPort = getPort(socketfd)) < 0 ){  
    printf("Unable to get a port.\n");
    return -1;
  }
  if((rlsSock = getSocket(serverHost, myPort)) < 0){
    printf("Unable to make a network socket.\n");
    return -1;
  }
  write(socketfd, "L\n", 2);
  read(socketfd, response, RESP_MAX);
  if(response[0] == 'E'){
    printf("%s\n", &response[1]);
    return -1;
  }
  if((pid = fork())){
    wait(NULL);
  }else{
    close(0); dup2(rlsSock, 0); close(rlsSock);
    execlp("more", "more", "-20", NULL);
  }
  close(rlsSock);
  return 1;
}

/* ***** Get Port ***** */
int getPort(int socketfd){
  int myPort = -1;
  char* cmd = "D\n";
  write(socketfd, cmd, 2);
  char response[RESP_MAX] = {0};
  read(socketfd, response, RESP_MAX);
  if(response[0] == 'E')
    printf("%s", &response[1]);
  else
    sscanf(response, "A%d\n", &myPort);
  return myPort;
}

/* ***** Get ***** */
int get(int socketfd, char* path, char* serverHost){
  char* fileName;
  int nameOnlyFlg = 1;      // if just a file name
  for(int i=0; i<strlen(path); i++)
    if (path[i] == '/')     // if a full path
      nameOnlyFlg = 0;
  if (nameOnlyFlg)
    fileName= strtok(path, "\0");
  else
    fileName = strrchr(path, '/')+1;

  int myPort, getSock, filefd;
  if((myPort = getPort(socketfd)) < 0 ){  
    printf("Unable to get a port.\n");
    return -1;
  }
  if((getSock = getSocket(serverHost, myPort)) < 0){
    printf("Unable to make a network socket.\n");
    return -1;
  }
  if((filefd = open(fileName, O_CREAT | O_EXCL | O_WRONLY, S_IXUSR, S_IRUSR 
  | S_IRGRP | S_IWGRP | S_IWUSR | S_IROTH)) < 0){
    perror("Error! Unable to open the file");
    return -1;
  }
  int readBlock;     // reading in 1 byte blocks
  char* buffer = (char*)malloc(sizeof(char) * CMD_MAX);
  char response[RESP_MAX] = {0};
  char* cmd = malloc( sizeof(char) * CMD_MAX );
  int cmdLen = myStrCat ('G', cmd, path);
  write(socketfd, cmd, cmdLen);
  read(socketfd, response, RESP_MAX);
  if(response[0] == 'E'){
    printf("%s", &response[1]);
    return -1;
  }
  while((readBlock = read(getSock, buffer, CMD_MAX)) > 0)
    write(filefd, buffer, readBlock);

  close (getSock);
  close (filefd);
  free(buffer);
  free (cmd);
  return 0;
}
// NOTE: I should make these ugly functions modular if I finish early.
/* ***** Show ***** */
int show(int socketfd, char* path, char* serverHost){
  int myPort, showSock;
  if((myPort = getPort(socketfd)) < 0 ){  
    printf("Unable to get a port.\n");
    return -1;
  }
  if((showSock = getSocket(serverHost, myPort)) < 0){
    printf("Unable to make a network socket.\n");
    return -1;
  }
  char response[RESP_MAX] = {0};
  char* cmd = malloc( sizeof(char) * CMD_MAX );
  int cmdLen = myStrCat ('G', cmd, path);
  write(socketfd, cmd, cmdLen);
  read(socketfd, response, RESP_MAX);
  if(response[0] == 'E'){
    printf("%s", &response[1]);
    return -1;
  }
  int pid;
  if((pid = fork())){
    wait(NULL);
  }else{
    close(0); dup2(showSock, 0); close(showSock);
    execlp("more", "more", "-20", NULL);
  }
  close (showSock);
  free (cmd);
  return 0;
}

/* ***** Put ***** */
int put(int socketfd, char* path, char* serverHost){
  int myPort, putSock, filefd;
  char* fileName;
  int nameOnlyFlg = 1;      // if just a file name
  for(int i=0; i<strlen(path); i++)
    if (path[i] == '/')     // if a full path
      nameOnlyFlg = 0;
  if (nameOnlyFlg)
    fileName= strtok(path, "\0");
  else
    fileName = strrchr(path, '/')+1;
  if(isReg(path) < 0){
    printf("Error! '%s' is not a regular file, or not accessible\n", path);
    return -1;
  } //Done with file check here
  if((filefd = open(fileName, O_RDONLY)) < 0){
    printf("Unable to open the file!\n");
    return -1;
  }
  if((myPort = getPort(socketfd)) < 0 ){  
    printf("Unable to get a port.\n");
    return -1;
  }
  if((putSock = getSocket(serverHost, myPort)) < 0){
    printf("Unable to make a network socket.\n");
    return -1;
  }

  int readBlock;     // reading in 1 byte blocks
  char* buffer = (char*)malloc(sizeof(char) * CMD_MAX);
  char response[RESP_MAX] = {0};
  char* cmd = malloc( sizeof(char) * CMD_MAX );
  int cmdLen = myStrCat ('P', cmd, path);
  write(socketfd, cmd, cmdLen);
  read(socketfd, response, RESP_MAX);
  if(response[0] == 'E'){
    printf("%s", &response[1]);
    return -1;
  }
  while((readBlock = read(filefd, buffer, CMD_MAX)) > 0)
    write(putSock, buffer, readBlock);

  close(filefd);
  close(putSock);
	free(buffer);
	return 1;
}

/* ***** if the file is a REGULAR FILE ***** */
int isReg(const char* filePath){
  struct stat temp;
  struct stat* tempPtr = &temp;
  if ( access(filePath, R_OK)==-1 || lstat(filePath, tempPtr)==-1 )
    return -1;
  if ( S_ISREG(tempPtr->st_mode) )
    return 1;
  return -1;
}
