typedef struct Data {
    int priority;
    int clientfd;
    char buff[256];
} Data;


typedef struct Queue{
    int current;
    int max;
    Data* cData;
} Queue;

void enqueue(Queue* q, Data d, char*);
Data dequeue(Queue* q);
void queueInitializer(Queue* q, int buffers);
