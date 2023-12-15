/* a thread-safe customer queue */

typedef struct customer customer;
struct customer {
    struct timeval arrival_time;
    customer *next;
    customer *prev;
};

customer *q_head, *q_tail;
unsigned q_length;
pthread_mutex_t q_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t q_cond_nonempty = PTHREAD_COND_INITIALIZER;

/* thread safe enqueue */
void enqueue(customer *c) {
    pthread_mutex_lock(&q_mutex);

    if (q_length == 0) {
        q_head = c;
        q_tail = c;

        pthread_cond_signal(&q_cond_nonempty);
    } else {
        c->next = q_tail;
        c->next->prev = c;
        q_tail = c;
        q_tail->prev = NULL;
    }

    q_length++;

    pthread_mutex_unlock(&q_mutex);
}

/* thread safe dequeue */
customer *dequeue() {
    pthread_mutex_lock(&q_mutex);

    customer *ret;
    
    /* if the queue is empty, wait for a signal from enqueue */
    while (q_length == 0) {
        pthread_cond_wait(&q_cond_nonempty, &q_mutex);
    }

    ret = q_head;
    q_head = q_head->prev;
    if (q_head) q_head->next = NULL;

    q_length--;

    pthread_mutex_unlock(&q_mutex);

    return ret;
}