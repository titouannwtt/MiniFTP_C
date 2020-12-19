


/* ***** Receive a Connection on the Server Side ***** */
int serverConnection(int myPort){
  int listenfd;
  // Setting Server Speifications
  if ( (listenfd=socket(AF_INET, SOCK_STREAM, 0)) < 0){
    perror("Socket");
    exit(1);
  }
  struct sockaddr_in servAddr;
  memset(&servAddr, 0, sizeof(servAddr));
  servAddr.sin_family = AF_INET;
  servAddr.sin_port = htons(myPort);
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  // Bind the socket to my port
  if ( bind (listenfd, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0){
    perror("bind");
    exit(1);
  }
  return listenfd;
}

/* ***** DATA CONNECTION ***** */
int dataConnection ( int connectfd, char* cmdLetter ){
  int datafd;
  read(connectfd, cmdLetter, 1);
  char datacmd[10] = {0};
  struct sockaddr_in data;
  socklen_t  length = sizeof(struct sockaddr_in);
  memset(&data, 0, length);
  struct sockaddr_in client1Addr;
  int dataListenfd = getListener(0);  // not a real port
  listen(dataListenfd, 1);
  getsockname(dataListenfd,( struct sockaddr* ) &data, &length);
  int myPort = ntohs(data.sin_port);
  sprintf ( &datacmd[0], "A%d\n", myPort );
  write(connectfd, datacmd, strlen(&datacmd[0]));
    if((datafd = accept(dataListenfd,
    (struct sockaddr*) &client1Addr, &length)) < 0){
    perror("Unable to Accept");
    exit(1);
  }
  return datafd;
}

/* ***** Change Server's CWD ***** */
void changeServerDir (int connectfd, char* path, char* clientHostName){
  if(chdir(path) < 0){
    write(connectfd, "EFailed to change the server directory!\n", 40);
    printf("%s failed to changed the CWD to %s\n", clientHostName, path);
  }else{
    write(connectfd, "A\n", 2);
    printf("%s changed the CWD to %s\n", clientHostName, path);
  }
}

/* ***** GET LISTENER ***** */
int getListener(int myPort){
  int listenfd;
  struct sockaddr_in servAddr;
  memset(&servAddr, 0, sizeof(servAddr));
  servAddr.sin_family = AF_INET;
  servAddr.sin_port = htons(myPort);
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

  if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    perror("listenfd");
    exit(1);
  }else if(bind(listenfd, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0){
    perror("bind");
    exit(1);
  }
  return listenfd;
}

/* ***** READ PATH ***** */
void readPath (char* path, int connectfd) {
  int readBlock;  // read 1 byte at a time
  for ( int i=0; (readBlock= read(connectfd,&path[i],1)) >0; i++ ){
    if(path[i] == '\n'){
      path[i] = '\0';
      break;
    }
  }
//printf("path:  '%s'\n", path);
}
