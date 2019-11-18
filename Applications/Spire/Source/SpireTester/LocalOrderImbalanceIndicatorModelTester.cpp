#include <catch.hpp>
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicator.hpp"
#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorModel.hpp"
#include "Spire/Spire/QtPromise.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using boost::posix_time::ptime;
using boost::posix_time::from_time_t;
using namespace Nexus;
using namespace Spire;

namespace {
  auto make_imbalance(const std::string& symbol, const ptime& timestamp) {
    return OrderImbalance(Security(symbol, 0), Side::BID, 100,
      Money(Money::ONE), timestamp);
  }

  const auto A = make_imbalance("A", from_time_t(100));
  const auto B = make_imbalance("B", from_time_t(200));
  const auto C = make_imbalance("C", from_time_t(300));
  const auto D = make_imbalance("D", from_time_t(400));
  const auto E = make_imbalance("E", from_time_t(500));
}

TEST_CASE("test_basic_inserting_subscribing",
    "[LocalOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model = LocalOrderImbalanceIndicatorModel();
    model.insert(A);
    model.insert(B);
    model.insert(C);
    auto [connection, promise] = model.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto data = wait(std::move(promise));
    auto expected = std::vector<OrderImbalance>({A, B, C});
    REQUIRE(std::is_permutation(data.begin(), data.end(), expected.begin(),
      expected.end()));
  }, "test_basic_inserting_subscribing");
}

TEST_CASE("test_subscribing_with_specific_range",
    "[LocalOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model = LocalOrderImbalanceIndicatorModel();
    model.insert(A);
    model.insert(B);
    model.insert(C);
    model.insert(D);
    model.insert(E);
    auto [connection1, promise1] = model.subscribe(from_time_t(100),
      from_time_t(300), [] (auto& i) {});
    auto data1 = wait(std::move(promise1));
    auto expected1 = std::vector<OrderImbalance>({A, B, C});
    REQUIRE(std::is_permutation(data1.begin(), data1.end(), expected1.begin(),
      expected1.end()));
    auto [connection2, promise2] = model.subscribe(from_time_t(200),
      from_time_t(400), [] (auto& i) {});
    auto data2 = wait(std::move(promise2));
    auto expected2 = std::vector<OrderImbalance>({B, C, D});
    REQUIRE(std::is_permutation(data2.begin(), data2.end(), expected2.begin(),
      expected2.end()));
    auto [connection3, promise3] = model.subscribe(from_time_t(300),
      from_time_t(500), [] (auto& i) {});
    auto data3 = wait(std::move(promise3));
    auto expected3 = std::vector<OrderImbalance>({C, D, E});
    REQUIRE(std::is_permutation(data3.begin(), data3.end(), expected3.begin(),
      expected3.end()));
  }, "test_subscribing_with_specific_range");
}

TEST_CASE("test_order_imbalance_signals",
    "[LocalOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model = LocalOrderImbalanceIndicatorModel();
    auto signal_data1 = OrderImbalance();
    auto signal_data2 = OrderImbalance();
    auto signal_data3 = OrderImbalance();
    auto [connection1, promise1] = model.subscribe(from_time_t(100),
      from_time_t(300), [&] (auto& i) { signal_data1 = i; });
    wait(std::move(promise1));
    auto [connection2, promise2] = model.subscribe(from_time_t(200),
      from_time_t(400), [&] (auto& i) { signal_data2 = i; });
    wait(std::move(promise2));
    auto [connection3, promise3] = model.subscribe(from_time_t(300),
      from_time_t(500), [&] (auto& i) { signal_data3 = i; });
    wait(std::move(promise3));
    model.insert(A);
    REQUIRE(signal_data1 == A);
    REQUIRE(signal_data2 == OrderImbalance());
    REQUIRE(signal_data3 == OrderImbalance());
    model.insert(B);
    REQUIRE(signal_data1 == B);
    REQUIRE(signal_data2 == B);
    REQUIRE(signal_data3 == OrderImbalance());
    model.insert(C);
    REQUIRE(signal_data1 == C);
    REQUIRE(signal_data2 == C);
    REQUIRE(signal_data3 == C);
    model.insert(D);
    REQUIRE(signal_data1 == C);
    REQUIRE(signal_data2 == D);
    REQUIRE(signal_data3 == D);
    model.insert(E);
    REQUIRE(signal_data1 == C);
    REQUIRE(signal_data2 == D);
    REQUIRE(signal_data3 == E);
  }, "test_order_imbalance_signals");
}

TEST_CASE("test_basic_order_imbalance_signals",
    "[LocalOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model = LocalOrderImbalanceIndicatorModel();
    auto signal_data1 = OrderImbalance();
    auto signal_data2 = OrderImbalance();
    auto signal_data3 = OrderImbalance();
    auto [connection1, promise1] = model.subscribe(from_time_t(100),
      from_time_t(300), [&] (auto& i) { signal_data1 = i; });
    wait(std::move(promise1));
    auto [connection2, promise2] = model.subscribe(from_time_t(200),
      from_time_t(400), [&] (auto& i) { signal_data2 = i; });
    wait(std::move(promise2));
    auto [connection3, promise3] = model.subscribe(from_time_t(300),
      from_time_t(500), [&] (auto& i) { signal_data3 = i; });
    wait(std::move(promise3));
    model.insert(A);
    REQUIRE(signal_data1 == A);
    REQUIRE(signal_data2 == OrderImbalance());
    REQUIRE(signal_data3 == OrderImbalance());
    model.insert(B);
    REQUIRE(signal_data1 == B);
    REQUIRE(signal_data2 == B);
    REQUIRE(signal_data3 == OrderImbalance());
    model.insert(C);
    REQUIRE(signal_data1 == C);
    REQUIRE(signal_data2 == C);
    REQUIRE(signal_data3 == C);
    model.insert(D);
    REQUIRE(signal_data1 == C);
    REQUIRE(signal_data2 == D);
    REQUIRE(signal_data3 == D);
    model.insert(E);
    REQUIRE(signal_data1 == C);
    REQUIRE(signal_data2 == D);
    REQUIRE(signal_data3 == E);
  }, "test_basic_order_imbalance_signals");
}

TEST_CASE("test_callback_is_not_reentrant",
    "[LocalOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model = LocalOrderImbalanceIndicatorModel();
    auto call_count = 0;
    auto slot = std::function<void (const OrderImbalance&)>();
    slot = [&] (const auto& imbalance) {
      ++call_count;
      if(call_count < 2) {
        model.subscribe(from_time_t(0), from_time_t(1000), slot);
      }
    };
    auto [connection, promise] = model.subscribe(from_time_t(0),
      from_time_t(1000), slot);
    wait(std::move(promise));
    model.insert(A);
    REQUIRE(call_count == 1);
  }, "test_callback_is_not_reentrant");
}

TEST_CASE("test_no_connections",
    "[LocalOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model = LocalOrderImbalanceIndicatorModel();
    auto signal_data = OrderImbalance();
    auto [connection, promise] = model.subscribe(from_time_t(0),
      from_time_t(1000), [&] (auto& i) { signal_data = i; });
    wait(std::move(promise));
    connection.disconnect();
    model.insert(A);
    REQUIRE(signal_data == OrderImbalance());
  }, "test_no_connections");
}

TEST_CASE("test_first_signal_disconnected",
    "[LocalOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model = LocalOrderImbalanceIndicatorModel();
    auto signal_data1 = OrderImbalance();
    auto signal_data2 = OrderImbalance();
    auto signal_data3 = OrderImbalance();
    auto [connection1, promise1] = model.subscribe(from_time_t(0),
      from_time_t(1000), [&] (auto& i) { signal_data1 = i; });
    wait(std::move(promise1));
    auto [connection2, promise2] = model.subscribe(from_time_t(0),
      from_time_t(1000), [&] (auto& i) { signal_data2 = i; });
    wait(std::move(promise2));
    connection1.disconnect();
    model.insert(A);
    REQUIRE(signal_data1 == OrderImbalance());
    REQUIRE(signal_data2 == A);
  }, "test_first_signal_disconnected");
}

TEST_CASE("test_last_signal_disconnected",
    "[LocalOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model = LocalOrderImbalanceIndicatorModel();
    auto signal_data1 = OrderImbalance();
    auto signal_data2 = OrderImbalance();
    auto signal_data3 = OrderImbalance();
    auto [connection1, promise1] = model.subscribe(from_time_t(0),
      from_time_t(1000), [&] (auto& i) { signal_data1 = i; });
    wait(std::move(promise1));
    auto [connection2, promise2] = model.subscribe(from_time_t(0),
      from_time_t(1000), [&] (auto& i) { signal_data2 = i; });
    wait(std::move(promise2));
    connection2.disconnect();
    model.insert(A);
    REQUIRE(signal_data1 == A);
    REQUIRE(signal_data2 == OrderImbalance());
  }, "test_last_signal_disconnected");
}

TEST_CASE("test_mixed_connections_with_disconnections",
    "[LocalOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model = LocalOrderImbalanceIndicatorModel();
    auto signal_data1 = OrderImbalance();
    auto signal_data2 = OrderImbalance();
    auto signal_data3 = OrderImbalance();
    auto signal_data4 = OrderImbalance();
    auto signal_data5 = OrderImbalance();
    auto [connection1, promise1] = model.subscribe(from_time_t(0),
      from_time_t(200), [&] (auto& i) { signal_data1 = i; });
    wait(std::move(promise1));
    auto [connection2, promise2] = model.subscribe(from_time_t(200),
      from_time_t(400), [&] (auto& i) { signal_data2 = i; });
    wait(std::move(promise2));
    auto [connection3, promise3] = model.subscribe(from_time_t(300),
      from_time_t(500), [&] (auto& i) { signal_data3 = i; });
    wait(std::move(promise3));
    auto [connection4, promise4] = model.subscribe(from_time_t(400),
      from_time_t(600), [&] (auto& i) { signal_data4 = i; });
    wait(std::move(promise4));
    auto [connection5, promise5] = model.subscribe(from_time_t(500),
      from_time_t(700), [&] (auto& i) { signal_data5 = i; });
    wait(std::move(promise5));
    connection2.disconnect();
    connection4.disconnect();
    model.insert(A);
    REQUIRE(signal_data1 == A);
    REQUIRE(signal_data2 == OrderImbalance());
    REQUIRE(signal_data3 == OrderImbalance());
    REQUIRE(signal_data4 == OrderImbalance());
    REQUIRE(signal_data5 == OrderImbalance());
    model.insert(B);
    REQUIRE(signal_data1 == B);
    REQUIRE(signal_data2 == OrderImbalance());
    REQUIRE(signal_data3 == OrderImbalance());
    REQUIRE(signal_data4 == OrderImbalance());
    REQUIRE(signal_data5 == OrderImbalance());
    model.insert(C);
    REQUIRE(signal_data1 == B);
    REQUIRE(signal_data2 == OrderImbalance());
    REQUIRE(signal_data3 == C);
    REQUIRE(signal_data4 == OrderImbalance());
    REQUIRE(signal_data5 == OrderImbalance());
    model.insert(D);
    REQUIRE(signal_data1 == B);
    REQUIRE(signal_data2 == OrderImbalance());
    REQUIRE(signal_data3 == D);
    REQUIRE(signal_data4 == OrderImbalance());
    REQUIRE(signal_data5 == OrderImbalance());
    model.insert(E);
    REQUIRE(signal_data1 == B);
    REQUIRE(signal_data2 == OrderImbalance());
    REQUIRE(signal_data3 == E);
    REQUIRE(signal_data4 == OrderImbalance());
    REQUIRE(signal_data5 == E);
  }, "test_mixed_connections_with_disconnections");
}

TEST_CASE("test_all_disconnected",
    "[LocalOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model = LocalOrderImbalanceIndicatorModel();
    auto signal_data1 = OrderImbalance();
    auto signal_data2 = OrderImbalance();
    auto signal_data3 = OrderImbalance();
    auto signal_data4 = OrderImbalance();
    auto signal_data5 = OrderImbalance();
    auto [connection1, promise1] = model.subscribe(from_time_t(0),
      from_time_t(200), [&] (auto& i) { signal_data1 = i; });
    wait(std::move(promise1));
    auto [connection2, promise2] = model.subscribe(from_time_t(200),
      from_time_t(400), [&] (auto& i) { signal_data2 = i; });
    wait(std::move(promise2));
    auto [connection3, promise3] = model.subscribe(from_time_t(300),
      from_time_t(500), [&] (auto& i) { signal_data3 = i; });
    wait(std::move(promise3));
    auto [connection4, promise4] = model.subscribe(from_time_t(400),
      from_time_t(600), [&] (auto& i) { signal_data4 = i; });
    wait(std::move(promise4));
    auto [connection5, promise5] = model.subscribe(from_time_t(500),
      from_time_t(700), [&] (auto& i) { signal_data5 = i; });
    wait(std::move(promise5));
    connection1.disconnect();
    connection2.disconnect();
    connection3.disconnect();
    connection4.disconnect();
    connection5.disconnect();
    model.insert(A);
    model.insert(B);
    model.insert(C);
    model.insert(D);
    model.insert(E);
    REQUIRE(signal_data1 == OrderImbalance());
    REQUIRE(signal_data2 == OrderImbalance());
    REQUIRE(signal_data3 == OrderImbalance());
    REQUIRE(signal_data4 == OrderImbalance());
    REQUIRE(signal_data5 == OrderImbalance());
  }, "test_all_disconnected");
}
