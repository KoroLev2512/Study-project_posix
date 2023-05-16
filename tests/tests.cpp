#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <producer_consumer.h>

TEST_CASE("FinalTest") {
  std::set<int> tids;
  for (int i = 1; i <= 20; ++i) {
    pthread_t thread;

    auto lambda = [](void *arg) -> void * {
      auto *tids = static_cast<std::set<int8_t> *>(arg);
      tids->insert(get_tid());
      return nullptr;
    };

    pthread_create(&thread, nullptr, lambda, &tids);
    pthread_join(thread, nullptr);
  }

  std::set<int> tidsReference = {1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
                                 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};

  CHECK_EQ(*tids.begin(), *tidsReference.begin());
  CHECK_EQ(*tids.rbegin(), *tidsReference.rbegin());
  CHECK_EQ(tids.size(), tidsReference.size());
}
