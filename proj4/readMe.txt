Sample output:

josh@h20:~/cs444/proj4$ ./q
lambda 5.0, mu 7.0, num customer 1000, num server 1
Queue length: 5

Statistics
                          average         standard deviation
inter-arrival time        0.214931        0.209528
customer waiting time     0.229148        0.291508
service time              0.139330        0.134051
queue length              1.068935        1.658628
------utilization:64.6%------

I implementd the queue in a separate C file, providing an API that interacts with the contents
of the queue in a thread-safe way. This means both `enqueue` and `dequeue` acquire `q_mutex` prior
to modifying any data. Also, `enqueue` fires the `q_cond_nonempty` signal whenever a new customer
is added to an empty queue--`dequeue` relies on this mechanism to wait for a new customer to arrive 
when the queue is empty.

Since all of the thread-safety is ensured inside of the queue API, the code for the threads is relatively
simple, as it doesn't need to deal with acquiring the correct mutexes or firing the appropriate signals.