#ifndef REQUEST_H
#define REQUEST_H

void httpWorker(int *, char *); // This function will handle request
char* fType(char *);
char* responseHeader(int, char *); // function that builds response header
char* requestClassify(int *sockfd, char* outputbuffer, int* ret);

#endif