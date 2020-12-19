#include <netinet/in.h>

int serverConnection(int myPort);
int dataConnection ( int connectfd, char* cmdLetter );
int getListener(int myPort);
void readPath (char* path, int connectfd);
void changeServerDir (int connectfd, char* path, char* clientHostName);
