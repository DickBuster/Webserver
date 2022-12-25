#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "helper.h"
#include "request.h"
#include "queue.h"

extern char **environ;

const char CONTENTDIR[] = "./contentdir"; // this is the directory where keep all the files for requests

void getFileName(char *buffer, char *method, char *filename, char *version)
{
    char homedir[50];
    strcpy(homedir, CONTENTDIR); // directory where files are stored.
    int i = sscanf(buffer, "%s %s %s", method, filename, version);
    if (strcmp(filename, "/") == 0) // if filename is not provided then we will send index.html
        strcpy(filename, strcat(homedir, "/index.html"));
    else
        strcpy(filename, strcat(homedir, filename));
}
int dynamicChecker(char *fileName, char *returnArgs)
{
    char *args;
    if (strstr(fileName, "cgi"))
    {
        args = strchr(fileName, '?');
        if (args)
        {
            strcpy(returnArgs, args + 1);
            *args = '\0';
        }
        else
        {
            strcpy(returnArgs, "");
        }
        return 1;
    }
    else
    {
        return 0;
    }
}

char *requestClassify(int *sockfd, char *outputbuffer, int* ret)
{
    int newsockfd = *sockfd; // create a local variable for sockfd
    char localbuffer[256];   // we will read the data in this buffer
    bzero(localbuffer, 256); // intialize the buffer data to zero

    char *token; // local variable to split the request to get the filename
    char fileName[50];
    char *type;
    // start reading the message from incoming conenction
    if (read(newsockfd, localbuffer, 255) < 0)
        error("ERROR reading from socket");
    strcpy(outputbuffer, localbuffer);
    // get the requested file part of the request
    printf("The buffer:\n%s \n", localbuffer);

    char token2[20], methd[20], ver[20];
    getFileName(localbuffer, methd, token2, ver);
    printf("The Token: %s \n", token2);

    // token = strtok(localbuffer, " "); // split string into token seperated by " "
    // token = strtok(NULL, " ");   // in this go we read the file name that needs to be sent
    strcpy(fileName, token2);
    char placeholder[50];
    int x = dynamicChecker(fileName, placeholder);
    *ret = x;

    // get the complete filename
    type = fType(fileName); // get file type
    return type;
}

void httpWorker(int *sockfd, char *inputBuffer)
{                            // sockfd contains all the information
    int newsockfd = *sockfd; // create a local variable for sockfd

    char buffer[256];   // we will read the data in this buffer
    bzero(buffer, 256); // intialize the buffer data to zero
    strcpy(buffer, inputBuffer);
    char homedir[50];
    strcpy(homedir, CONTENTDIR); // directory where files are stored.

    // char *token; // local variable to split the request to get the filename
    char *type;
    // start reading the message from incoming conenction
    // get the requested file part of the request
    // token = strtok(buffer, " "); // split string into token seperated by " "
    // token = strtok(NULL, " ");   // in this go we read the file name that needs to be sent
    // strcpy(fileName, token);
    char fileName[50], methd[20], ver[20];
    getFileName(buffer, methd, fileName, ver);
    char dynamicArgs[50];
    int x = dynamicChecker(fileName, dynamicArgs);
    FILE *fp;
    int file_exist = 1;
    fp = fopen(fileName, "r");
    if (fp == NULL)
        file_exist = 0;

    if (x)
    {
        //sleep(4);
        // Dynamic
        char respHeader[256] = "HTTP/1.0 200 OK\r\n";
        printf("Network Sock: %d\n", newsockfd);
        if ((send(newsockfd, respHeader, strlen(respHeader), 0) == -1) || (send(newsockfd, "\r\n", strlen("\r\n"), 0) == -1))
        {
            perror("Failed to send bytes to client");
            return;
        }
        if (file_exist)
        {
            char *placeholderp[2];
            placeholderp[0] = dynamicArgs;
            if (fork() == 0)
            {
                printf("In the child node\n");
                dup2(newsockfd, STDOUT_FILENO);
                execve(fileName, placeholderp, environ);
            }
            wait(NULL);
        }
        else
        {
            if (send(newsockfd, "<html> <HEAD><TITLE>404 Not Found</TITLE></HEAD><BODY>Not Found</BODY></html> \r\n", 100, 0) == -1)
                perror("Failed to send bytes to client");
        }
    }
    else
    {
        // Static

        char *respHeader;       // response header
        type = fType(fileName); // get file type

        // open file and ready to send

        respHeader = responseHeader(file_exist, type);

        if ((send(newsockfd, respHeader, strlen(respHeader), 0) == -1) || (send(newsockfd, "\r\n", strlen("\r\n"), 0) == -1))
            perror("Failed to send bytes to client");

        free(respHeader); // free the allocated memory (note: the memory is allocated in responseheader function)

        if (file_exist)
        {
            char filechar[1];
            while ((filechar[0] = fgetc(fp)) != EOF)
            {
                if (send(newsockfd, filechar, sizeof(char), 0) == -1)
                    perror("Failed to send bytes to client");
            }
        }
        else
        {
            if (send(newsockfd, "<html> <HEAD><TITLE>404 Not Found</TITLE></HEAD><BODY>Not Found</BODY></html> \r\n", 100, 0) == -1)
                perror("Failed to send bytes to client");
        }
    }
    close(newsockfd);
}

// function below find the file type of the file requested
char *fType(char *fileName)
{
    char *type;
    char *filetype = strrchr(fileName, '.'); // This returns a pointer to the first occurrence of some character in the string
    if ((strcmp(filetype, ".htm")) == 0 || (strcmp(filetype, ".html")) == 0)
        type = "text/html";
    else if ((strcmp(filetype, ".jpg")) == 0)
        type = "image/jpeg";
    else if (strcmp(filetype, ".gif") == 0)
        type = "image/gif";
    else if (strcmp(filetype, ".txt") == 0)
        type = "text/plain";
    else
        type = "application/octet-stream";

    return type;
}

// buildresponseheader
char *responseHeader(int filestatus, char *type)
{
    char statuscontent[256] = "HTTP/1.0";
    if (filestatus == 1)
    {
        strcat(statuscontent, " 200 OK\r\n");
        strcat(statuscontent, "Content-Type: ");
        strcat(statuscontent, type);
        strcat(statuscontent, "\r\n");
    }
    else
    {
        strcat(statuscontent, "404 Not Found\r\n");
        // send a blank line to indicate the end of the header lines
        strcat(statuscontent, "Content-Type: ");
        strcat(statuscontent, "NONE\r\n");
    }
    char *returnheader = malloc(strlen(statuscontent) + 1);
    strcpy(returnheader, statuscontent);
    return returnheader;
}