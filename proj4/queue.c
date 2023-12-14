typedef struct customer customer;
struct customer {
    struct timeval arrival_time;
    customer *next;
    customer *prev;
};

customer *qHead, *qTail;
unsigned qLength;
pthread_mutex_t qMutex = PTHREAD_MUTEX_INITIALIZER;

void enqueue(customer *c) {
    if (qLength == 0) {
        qHead = c;
        qTail = c;
    } else {
        c->next = qTail;
        c->next->prev = c;
        qTail = c;
        qTail->prev = NULL;
    }

    qLength++;
}

customer *dequeue() {
    customer *ret;
    
    if (qLength == 0) {
        printf("Error: can't dequeue from empty queue\n");
        exit(1);
    }

    ret = qHead;
    qHead = qHead->prev;
    if (qHead) qHead->next = NULL;

    qLength--;

    return ret;
}