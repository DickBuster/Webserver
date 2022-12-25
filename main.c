/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "helper.h"
#include "request.h"
#include "queue.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;

pthread_t *workers = NULL;
Queue q;

void *responseWorker(void *args);

void args(int *portno, int *thread_size, int *pool_size, char **sched, int argc, char *argv[])
{
    if (argc < 5)
    {
        fprintf(stderr, "Usage: %s [portnum] [threads] [buffers] [schedalg]\n", argv[0]);
        exit(1);
    }
    *portno = atoi(argv[1]);
    *thread_size = atoi(argv[2]);
    *pool_size = atoi(argv[3]);
    *sched = argv[4];
}

int main(int argc, char *argv[])
{
    int yes = 1;
    char* sched;
    int thread_size = 0;
    int pool_size = 0;
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;

    args(&portno, &thread_size, &pool_size, &sched, argc, argv);
    printf("%s\n", sched);
    queueInitializer(&q, pool_size);

    workers = (pthread_t *)malloc(sizeof(pthread_t) * thread_size);

    for (int i = 0; i < thread_size; i++)
    {
        pthread_create(&workers[i], NULL, responseWorker, NULL);
    }
    int n;
    if (argc < 2)
    {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    bzero((char *)&serv_addr, sizeof(serv_addr));

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                   sizeof(int)) == -1)
    {
        perror("setsockopt");
        exit(1);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr,
             sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    listen(sockfd, pool_size);
    char buf[256];
    char *type;
    int x = 0;
    while (1)
    {
        Data input;
        bzero(buf, 256);
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0)
            error("ERROR on accept");
        pthread_mutex_lock(&mutex);
        while (q.current == q.max)
        {
            pthread_cond_wait(&full, &mutex);
        }
        pthread_mutex_unlock(&mutex);
        type = requestClassify(&newsockfd, buf, &x);
        printf("Type is: %s\n", type);
        printf("Static 0, Dynamic 1: %d\n", x);

        // Data Encapsulate
        strcpy(input.buff, buf);
        input.clientfd = newsockfd;
        if (x)
            input.priority = x + 4;
        else
            input.priority = x + 2;

        pthread_mutex_lock(&mutex);
        enqueue(&q, input, sched);
        for (int i = 0; i < q.max; i++)
        {
            printf("%d", q.cData[i].priority);
        }
        printf("\n");
        pthread_cond_signal(&empty);
        pthread_mutex_unlock(&mutex);
    }

    close(sockfd);
    return 0;
}

void *responseWorker(void *args)
{
    while (1)
    {
        pthread_mutex_lock(&mutex);
        while (q.current == 0)
        {
            pthread_cond_wait(&empty, &mutex);
        }
        Data d = dequeue(&q);
        pthread_cond_signal(&full);
        pthread_mutex_unlock(&mutex);
        // printf("The data is: %s\n", d.buff);
        httpWorker(&(d.clientfd), d.buff); // worker to fulfill the request
    }
}