#include <catch.hpp>
#include "spire/spire/qt_promise.hpp"
#include "spire/spire_tester/spire_tester.hpp"

using namespace Spire;

//! Returns a promise that signals the result only when all provided
//! promises have completed, or throws an exception if any provided
//! promise throws an exception.
/*
  \param promises The promises to be executed.
*/
template<typename T>
QtPromise<std::vector<T>> all(std::vector<QtPromise<T>> promises) {
  if(promises.empty()) {
    return QtPromise(
      [] {
        return std::vector<T>();
      });
  }
  auto promises_ptr = std::make_shared<std::vector<QtPromise<T>>>();
  for(auto& promise : promises) {
    promises_ptr->push_back(std::move(promise));
  }
  auto completed_promises = std::make_shared<std::vector<T>>();
  auto promise = std::move(promises_ptr->front());
  for(auto i = 0; i < promises_ptr->size() - 1; ++i) {
    promise = promise.then([=] (auto result) {
      completed_promises->push_back(std::move(result.Get()));
      return std::move(promises_ptr->at(i + 1));
    });
  }
  return promise.then([=] (auto result) {
    completed_promises->push_back(result.Get());
    return *completed_promises;
  });
}

TEST_CASE("test_chaining_promise_then", "[QtPromise]") {
  run_test([] {
    auto p = QtPromise(
      [] {
        return 123;
      }).then([] (auto result) {
        return QtPromise(
          [=] {
            return 2 * result.Get();
          }).then([] (auto result) {
            return 3 * result.Get();
          });
      }).then([] (auto result) {
        return 6 * result.Get();
      });
    auto r = wait(std::move(p));
    REQUIRE(r == 4428);
  }, "test_chaining_promise_then");
}

TEST_CASE("test_empty_promise", "[QtPromise]") {
  run_test([] {
    auto promises = std::vector<QtPromise<std::vector<int>>>();
    auto result = wait(std::move(all(std::move(promises))));
    REQUIRE(result == std::vector<std::vector<int>>());
  }, "test_empty_promise");
}

TEST_CASE("test_single_promise", "[QtPromise]") {
  run_test([] {
    auto promises = std::vector<QtPromise<int>>();
    promises.push_back(QtPromise([] {
      return 1;
    }));
    auto result = wait(std::move(all(std::move(promises))));
    REQUIRE(result == std::vector<int>{1});
  }, "test_single_promise");
}

TEST_CASE("test_multiple_promises", "[QtPromise]") {
  run_test([] {
    auto promises = std::vector<QtPromise<int>>();
    promises.push_back(QtPromise([] {
      return 1;
    }));
    promises.push_back(QtPromise([] {
      return 2;
    }));
    promises.push_back(QtPromise([] {
      return 3;
    }));
    promises.push_back(QtPromise([] {
      return 4;
    }));
    auto all_promise = std::move(all(std::move(promises)));
    auto result = wait(std::move(all_promise));
    REQUIRE(result == std::vector<int>{1, 2, 3, 4});
  }, "test_multiple_promises");
}
