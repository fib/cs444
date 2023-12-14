#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/time.h>
#include <math.h>
#include <pthread.h>

#include "queue.c"
#include <linux/time.h>

void avgAndStdDev(double *x, double *avg, double *stddev, int size);

pthread_cond_t customer_signal = PTHREAD_COND_INITIALIZER;
pthread_mutex_t server_signal_mutex = PTHREAD_MUTEX_INITIALIZER;

int q_not_empty = 0;

struct queue_stats {
    double x_sum;
    double x2_sum;
    int size;
};

struct customer_generator_args {
    double lambda;
    int num_customer;
    double *arrival_times;
};

int num_customer;
int customer_server_flag = 0;

struct server_args {
    double mu;
    int num_customer;
    double *service_times;
    double *wait_times;
};

struct observer_args {
    int *queue_lengths;
    struct queue_stats *q_stats;
};



double rnd_exp(struct drand48_data *randData, double lambda) {
    double tmp;

    drand48_r(randData, &tmp);

    return -log(1.0 - tmp) / lambda;
}

/* sleep for the given amount of time */
void do_sleep(double time) {
    double x;
    struct timespec sleep_time;

    sleep_time.tv_sec = (int)time;
    sleep_time.tv_nsec = modf(time, &x) * 1000000000;

    nanosleep(&sleep_time, NULL);
}

/* inserts customers into a queue based on an exponential distribution */
void *customer_generator(void *cg_args) {
    struct customer_generator_args *args = cg_args;
    double lambda = args->lambda;
    int num_customer = args->num_customer;
    double *arrival_times = args->arrival_times;

    struct drand48_data randData;
    struct timeval tv;
    double result;

    int prev_q_length;

    gettimeofday(&tv, NULL);
    srand48_r(tv.tv_sec + tv.tv_usec, &randData);

    double time;
    int i = 0;

    while (!customer_server_flag) {
        time = rnd_exp(&randData, lambda);
        if (i < num_customer) {
            arrival_times[i] = time;
        }

        do_sleep(time);
        
        customer *new_customer = (customer*)malloc(sizeof(customer));
        gettimeofday(&tv, NULL);
        new_customer->arrival_time = tv;


        pthread_mutex_lock(&qMutex);

        prev_q_length = qLength;

        enqueue(new_customer);

        if (prev_q_length == 0) {
            pthread_cond_signal(&customer_signal);
        }
        
        pthread_mutex_unlock(&qMutex);
        i++;
    }
}

void *server(void *s_args) {
    struct server_args *args = s_args;
    double mu = args->mu;
    double *service_times = args->service_times;
    double *wait_times = args->wait_times;
    
    customer* c;
    
    double time;
    struct drand48_data randData; 
    struct timeval tv;

    gettimeofday(&tv, NULL);
    srand48_r(tv.tv_sec + tv.tv_usec, &randData);

    for (int i = 0; i < num_customer; i++) {
        // instead of using signals, we simply wait until the queue is not empty
        while (qLength == 0) {
            continue;
        }

        pthread_mutex_lock(&qMutex);

        c = dequeue();

        pthread_mutex_unlock(&qMutex);

        gettimeofday(&tv, NULL);

        wait_times[i] = (tv.tv_sec - c->arrival_time.tv_sec) + (tv.tv_usec - c->arrival_time.tv_usec) / 1000000.0;

        time = rnd_exp(&randData, mu);

        service_times[i] = time;
        do_sleep(time);
    }

    customer_server_flag = 1;
}

void *observer(void *o_args) {
    struct observer_args *args = o_args;
    int *queue_lengths = args->queue_lengths;
    struct queue_stats *q_stats = args->q_stats;

    double time = 0.005;
    customer *c;
    int i, prev_len;

    
    while (!customer_server_flag) {
        if (qLength != prev_len)  {
            prev_len = qLength;
        }

        q_stats->x_sum += qLength;
        q_stats->x2_sum += qLength * qLength;
        q_stats->size++;

        printf("\33[2K\r");
        printf("%d", qLength);
        fflush(stdout);

        do_sleep(time);
    }
}

int main(int argc, char **argv)
{
    struct timespec start, finish;
    double run_time;

    clock_gettime(CLOCK_MONOTONIC, &start);


    double lambda = 5.0, mu = 7.0;
    int num_server = 1;
    num_customer = 1000;
    int c;


    pthread_t customer_generator_thread, server_thread, observer_thread;

    while ((c = getopt(argc, argv, "l:m:c:s:")) != -1) {
        switch (c) {
            case 'l':
                lambda = atof(optarg);
                break;
            case 'm':
                mu = atof(optarg);
                break;
            case 'c':
                num_customer = atoi(optarg);
                break;
            case 's':
                num_server = atoi(optarg);
                break;
        }
    }

    if (lambda > (mu * num_server)) {
        printf("Error: this system is unstable (lambda < mu * numServer must hold): %f > %f * %d\n", lambda, mu, num_server);
        exit(1);
    }

    printf("lambda %.1f, mu %.1f, num customer %d, num server %d\n", lambda, mu, num_customer, num_server);

    double *arrival_times = (double*)malloc(num_customer * sizeof(double));
    double *wait_times = (double*)malloc(num_customer * sizeof(double));
    double *service_times = (double*)malloc(num_customer * sizeof(double));
    int *queue_lengths = (int*)malloc(num_customer * sizeof(int));

    struct queue_stats q_stats = {0};

    memset(arrival_times, 0, num_customer);
    memset(wait_times, 0, num_customer);
    memset(service_times, 0, num_customer);

    struct customer_generator_args cg_args = {lambda, num_customer, arrival_times};
    struct server_args server_args = {mu, num_customer, service_times, wait_times};
    struct observer_args observer_args = {queue_lengths, &q_stats};

    pthread_create(&server_thread, NULL, server, (void*) &server_args);
    pthread_create(&observer_thread, NULL, observer, (void*) &observer_args);
    pthread_create(&customer_generator_thread, NULL, customer_generator, (void*) &cg_args);

    pthread_join(server_thread, NULL);

    clock_gettime(CLOCK_MONOTONIC, &finish);
    run_time = (finish.tv_sec - start.tv_sec) + (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

    double avg = 0, stddev = 0;

    printf("\n\nStatistics\n");
    printf("%25s %-15s %s\n", "", "average", "standard deviation");

    avgAndStdDev(arrival_times, &avg, &stddev, num_customer);
    printf("%-25s %-15f %f\n", "inter-arrival time", avg, stddev);

    avgAndStdDev(wait_times, &avg, &stddev, num_customer);
    printf("%-25s %-15f %f\n", "customer waiting time", avg, stddev);

    avgAndStdDev(service_times, &avg, &stddev, num_customer);
    printf("%-25s %-15f %f\n", "service time", avg, stddev);

    avg = q_stats.x_sum / q_stats.size;
    stddev = sqrt((q_stats.x2_sum - avg * avg * q_stats.size) / (q_stats.size - 1));
    printf("%-25s %-15f %f\n", "queue length", q_stats.x_sum/q_stats.size, stddev);

    double total_service_time = 0;
    for (int i = 0; i < num_customer; i++) total_service_time += service_times[i];
    
    printf("------utilization:%.1f%%------\n", (total_service_time/run_time) * 100);

    return 0;
}

void avgAndStdDev(double *x, double *avg, double *stddev, int size) {
    double total = 0;
    *avg = *stddev = 0;
    
    for (int i = 0; i < size; i++) {
        total += x[i];
    }

    *avg = total / size;

    for (int i = 0; i < size; i++) {
        *stddev += pow(x[i] - *avg, 2);
    }

    *stddev = sqrt(*stddev/size);
}
