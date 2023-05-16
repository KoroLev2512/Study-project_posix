#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <queue>
#include <cstdlib>
#include <ctime>
#include <memory>

pthread_mutex_t MUTEX = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t CV_CONSUMER = PTHREAD_COND_INITIALIZER;
pthread_cond_t CV_PRODUCER = PTHREAD_COND_INITIALIZER;

std::queue<int> buffer;

static const int BUFFER_SIZE = 5;

bool producer_is_active = true;
bool is_new_number = false;

int get_tid() {
    thread_local std::shared_ptr<int> tid;
    static int tidsCounter = 1;

    if (!tid) {
        tid = std::make_shared<int>(tidsCounter++);
    }
    return *tid;
}

struct consumer_args {
    int* sleep_time;
    int* sum;
    int* add;
    bool debug;
};

void* producer_routine(void* arg) {
    int* add = static_cast<int*>(arg);

    while (true) {
        pthread_mutex_lock(&MUTEX);
        if (!(std::cin >> *add)) {
            producer_is_active = false;
            pthread_mutex_unlock(&MUTEX);
            break;
        }
        is_new_number = true;

        pthread_cond_signal(&CV_CONSUMER);
        pthread_cond_wait(&CV_PRODUCER, &MUTEX);
        pthread_mutex_unlock(&MUTEX);
    }

    usleep(300);

    pthread_mutex_lock(&MUTEX);
    pthread_cond_broadcast(&CV_CONSUMER);
    pthread_mutex_unlock(&MUTEX);

    return nullptr;
}

void* consumer_routine(void* arg) {
    consumer_args* consumerArgs = static_cast<consumer_args*>(arg);
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    thread_local int thread_sum = 0;

    while (true) {
        pthread_mutex_lock(&MUTEX);
        while (!is_new_number && producer_is_active) {
            pthread_cond_wait(&CV_CONSUMER, &MUTEX);
        }

        if (!producer_is_active) {
            pthread_mutex_unlock(&MUTEX);
            break;
        }
        is_new_number = false;
        thread_sum += *consumerArgs->add;
        pthread_cond_signal(&CV_PRODUCER);

        if (consumerArgs->debug) {
            std::cout << "(" << get_tid() << ", " << thread_sum << ")" << std::endl;
        }
        pthread_mutex_unlock(&MUTEX);
        usleep(*consumerArgs->sleep_time);
    }
    pthread_mutex_lock(&MUTEX);
    *consumerArgs->sum += thread_sum;
    pthread_mutex_unlock(&MUTEX);

    return nullptr;
}

void* consumer_interruptor_routine(void* arg) {
    auto* consumer_threads = static_cast<std::vector<pthread_t>*>(arg);
    while (producer_is_active)
        for (auto& thread : *consumer_threads) pthread_cancel(thread);
    return nullptr;
}

int run_threads(int treadsCount, int sleepTime, bool debug) {
    int sum = 0;
    int add = 0;
    pthread_t producer_thread, interrupt_thread;
    std::vector<pthread_t> consumer_threads(treadsCount);
    pthread_create(&producer_thread, nullptr, producer_routine, &add);
    producer_is_active = true;

    consumer_args consumer_args = {&sleepTime, &sum, &add, debug};

    for (int i = 0; i < treadsCount; i++) {
        pthread_create(&consumer_threads[i], nullptr, consumer_routine, &consumer_args);
    }

    pthread_create(&interrupt_thread, nullptr, consumer_interruptor_routine, &consumer_threads);
    pthread_join(producer_thread, nullptr);
    pthread_join(interrupt_thread, nullptr);
    for (int i = 0; i < treadsCount; i++) {
        pthread_join(consumer_threads[i], nullptr);
    }

    return sum;
}
