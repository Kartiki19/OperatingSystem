#include <iostream>
#include <vector>
#include <pthread.h>
#include <thread>
#include <stdio.h>
using namespace std;


class CircularBuffer {
private:
    vector<int> cBuffer;
    int size_;
    int read_;
    int write_;
    int count_;
    pthread_mutex_t mutex_;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;

public:
    int all_consumed_ = 0;
    int max_ = 0; /// Maximum numbers to be generated (0 to max_-1)
    CircularBuffer(int size) : cBuffer(size), size_(size), read_(0), write_(0), count_(0) {
        pthread_mutex_init(&mutex_, NULL);
        pthread_cond_init(&not_full, NULL);
        pthread_cond_init(&not_empty, NULL);
    }

    ~CircularBuffer() {
        pthread_mutex_destroy(&mutex_);
        pthread_cond_destroy(&not_full);
        pthread_cond_destroy(&not_empty);
    }

    /* Produce integers onto circular buffer and broadcast that the data is available to consume */
    void produce(int value) {
        pthread_mutex_lock(&mutex_);
        while (count_ == size_) {
            pthread_cond_wait(&not_full, &mutex_);
        }
        cBuffer[write_] = value;
        write_ = (write_ + 1) % size_;
        ++count_;
        cout << "Producer wrote " << value << endl;
        pthread_mutex_unlock(&mutex_);
        pthread_cond_broadcast(&not_empty);
    }

    /* Consume the available data on signal */
    int consume()
    {
        pthread_mutex_lock(&mutex_);
        while (count_ == 0) {
            if(all_consumed_ >= max_) /// when reached to max, exit safely
            {
                pthread_mutex_unlock(&mutex_);
                return 0;
            }
            pthread_cond_wait(&not_empty, &mutex_);
        }
        int value = cBuffer[read_];
        read_ = (read_ + 1) % size_;
        --count_;
        ++all_consumed_;
       
        /// Convert pthread_t to integer
        cout << "Consumer " << static_cast<int>(reinterpret_cast<std::uintptr_t>(pthread_self())) 
        << ", read " << value << endl;

        pthread_mutex_unlock(&mutex_);
        pthread_cond_broadcast(&not_full);
        return 0;
    }
};

/* Generate integer data and send it to write onto circular buffer */
void* producer(void* arg) {
    CircularBuffer* buffer = static_cast<CircularBuffer*>(arg);
    for (int i = 0; i < buffer->max_; ++i) {
        buffer->produce(i);
    }
    return NULL;
}

void* consumer(void* arg) {
    CircularBuffer* buffer = static_cast<CircularBuffer*>(arg);
    while(buffer->all_consumed_ <  buffer->max_)
        buffer->consume();
    return NULL;
}

int main(int argc, char **argv)
{
    int num_consumers = 2; /// Specify on cmd line; default is two. 
    int max_number = 10; /// Specify on cmd line; default is ten. 
    CircularBuffer buffer(5); /// Circular Buffer of size 5

    /* Find out how many consumers to create. */
    if (argc > 1)
        num_consumers = atoi(argv[1]);

    /* Find out maximum number upto which producer should generate integer stream */
    if (argc > 2)
    {
        if(atoi(argv[2]) <= 5){
            /// As buffer size is 5, to make utilization of circular buffer, max  number should be greater than 5
            cerr << "Max Number (2nd Argument) should be greater than 5 !" << endl; 
            return 1;
        }
        else max_number = atoi(argv[2]);
    }

    buffer.max_ = max_number;
    
    pthread_t producer_thread, consumer_threads[num_consumers];
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    /* Create Producer Thread */
    if(pthread_create(&producer_thread, &attr, producer, &buffer) != 0)
    {
        cerr << "Could not create producer thread !" << endl;
        return 1;
    }

    /* Create Consumer Threads */
    for (int i = 0; i < num_consumers; i++)
    {
        if (pthread_create(&consumer_threads[i], &attr, consumer, &buffer) != 0)
        {
            cerr << "Could not create consumer " << i << endl;
            return 1;
        }
    }

    /* Wait for termination of all the threads */
    pthread_join(producer_thread, NULL);
    for (int i = 0; i < num_consumers; ++i)
        pthread_join(consumer_threads[i], NULL);


    return 0;
} 
