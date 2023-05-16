#pragma once

int get_tid();
int run_threads(int thread_count, int sleep_time, bool debug);

struct consumer_args {
  static void* consumer_routine(void* arg);
};
