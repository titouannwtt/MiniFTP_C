#define _BSD_SOURCE                         // to support gethostname() in C99
#include <stdio.h>  
#include <string.h>
#include <sys/unistd.h>                     // for gethostname()
#include <stdlib.h>                         // for exit()
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>                          // open(), close()
#include <sys/stat.h>                       // for stat in isReg()

void cd(char* path);
void clean (char* path);                    // remove \n from the end
int getSocket(char* serverHost, int myPort);// connect to server and return the socket
void rcd(char* path, int socketfd);         // Remote Change Directory
void ls();                                  // ls: List Segments
int myStrCat(char c,char* cmd,char* path);  // cmd = KeyChar + Path
void myExit(int socketfd);
int getPort(int socketfd);                  // get a free port num from server
int rls(int socketfd, char* serverHost);    // RLS
int get(int socketfd, char* path, char* serverHost);        // Get
int show(int socketfd, char* path, char* serverHost);       // Show
int put(int socketfd, char* path, char* serverHost);        // Show
int isReg(const char* filePath);            // if regular and accessible
