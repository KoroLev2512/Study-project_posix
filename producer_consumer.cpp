#include <pthread.h>
#include <iostream>
#include <vector>
#include <queue>
#include <cstdlib>
#include <ctime>


pthread_mutex_t MUTEX = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t CV_CONSUMER = PTHREAD_COND_INITIALIZER;
pthread_cond_t CV_PRODUCER = PTHREAD_COND_INITIALIZER;

std::queue<int> buffer;

static const int BUFFER_SIZE = 5;

int get_tid() {
    static int tid = 1;
    return tid++;
}

void* producer_routine(void* arg) {
    int* data = static_cast<int*>(arg);

    for (int i = 0; i < *data; i++) {
        int value = std::rand() % 100;
        pthread_mutex_lock(&MUTEX);
        while (buffer.size() == BUFFER_SIZE) {
            pthread_cond_wait(&CV_PRODUCER, &MUTEX);
        }
        buffer.push(value);
        pthread_cond_signal(&CV_CONSUMER);
        pthread_mutex_unlock(&MUTEX);
    }

    pthread_mutex_lock(&MUTEX);
    while (buffer.size() != 0) {
        pthread_cond_wait(&CV_PRODUCER, &MUTEX);
    }
    pthread_cond_broadcast(&CV_CONSUMER);
    pthread_mutex_unlock(&MUTEX);

    return nullptr;
}

void* consumer_routine(void* arg) {
    int* result = new int(0);
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);

    while (true) {
        pthread_mutex_lock(&MUTEX);
        while (buffer.empty()) {
            pthread_cond_wait(&CV_CONSUMER, &MUTEX);
            if (buffer.empty()) {
                pthread_mutex_unlock(&MUTEX);
                continue;
            }
        }
        int value = buffer.front();
        buffer.pop();
        *result += value;
        pthread_cond_signal(&CV_PRODUCER);
        pthread_mutex_unlock(&MUTEX);
    }

    return result;
}

void* consumer_interruptor_routine(void* arg) {
    std::vector<pthread_t>* consumers = static_cast<std::vector<pthread_t>*>(arg);
    while (true) {
        usleep(20000);
        if (buffer.empty()) {
            continue;
        }
        int consumer_id = std::rand() % consumers->size();
        pthread_cancel((*consumers)[consumer_id]);
    }
    return nullptr;
}

int run_threads(int n_producers, int n_consumers) {
    std::srand(std::time(nullptr));

    pthread_t producer_thread, interrupt_thread;
    std::vector<pthread_t> consumer_threads(n_consumers);

    int producer_data = n_producers * BUFFER_SIZE;
    pthread_create(&producer_thread, nullptr, producer_routine, &producer_data);

    for (int i = 0; i < n_consumers; i++) {
        pthread_create(&consumer_threads[i], nullptr, consumer_routine, nullptr);
    }

    pthread_create(&interrupt_thread, nullptr, consumer_interruptor_routine, &consumer_threads);

    pthread_join(producer_thread, nullptr);

    void* result;
    int sum = 0;
    for (int i = 0; i < n_consumers; i++) {
        pthread_join(consumer_threads[i], &result);
        sum += *static_cast<int*>(result);
        delete static_cast<int*>(result);
    }

    pthread_join(interrupt_thread, nullptr);

    pthread_cond_destroy(&CV_CONSUMER);
    pthread_cond_destroy(&CV_PRODUCER);
    pthread_mutex_destroy(&MUTEX);

    return sum;
}
