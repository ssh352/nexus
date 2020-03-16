#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorModel.hpp"
#include "Spire/Spire/Utility.hpp"

using namespace Beam::Queries;
using namespace boost;
using namespace boost::icl;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

void LocalOrderImbalanceIndicatorModel::publish(
    const OrderImbalance& imbalance) {
  insert_sorted(imbalance);
  m_imbalance_published_signal(imbalance);
}

void LocalOrderImbalanceIndicatorModel::insert(
    const OrderImbalance& imbalance) {
  insert_sorted(imbalance);
}

QtPromise<std::vector<Nexus::OrderImbalance>>
    LocalOrderImbalanceIndicatorModel::load(const TimeInterval& interval) {
  auto [first, last] = get_iterators_from_interval(interval);
  return QtPromise(
    [imbalances = std::vector<OrderImbalance>(first, last)] () mutable {
      return std::move(imbalances);
  });
}

QtPromise<std::vector<Nexus::OrderImbalance>>
    LocalOrderImbalanceIndicatorModel::load(const Security& security,
    const TimeInterval& interval) {
  auto imbalances = std::vector<OrderImbalance>();
  auto [first, last] = get_iterators_from_interval(interval);
  for(auto i = first; i != last; ++i) {
    if(i->m_security == security) {
      imbalances.push_back(*i);
    }
  }
  return QtPromise([imbalances = std::move(imbalances)] () mutable {
    return std::move(imbalances);
  });
}

QtPromise<std::vector<Nexus::OrderImbalance>>
    LocalOrderImbalanceIndicatorModel::load(const Security& security,
    const ptime& timestamp, const SnapshotLimit& limit) {
  if(limit.GetSize() == 0) {
    return QtPromise<std::vector<OrderImbalance>>([] {
        return std::vector<OrderImbalance>();
      });
  }
  auto [lower_iter, upper_iter] = [&] {
      if(limit.GetType() == SnapshotLimit::Type::HEAD) {
        auto lower = std::lower_bound(m_imbalances.begin(),
          m_imbalances.end(), timestamp,
          [] (const auto& imbalance, const auto& timestamp) {
              return imbalance.m_timestamp < timestamp;
            });
        auto lower_index = std::distance(m_imbalances.begin(), lower);
        auto upper = lower + min(std::distance(lower, m_imbalances.end()),
           (lower_index + limit.GetSize()) - lower_index);
        return std::make_tuple(lower, upper);
      }
      auto upper = std::upper_bound(m_imbalances.begin(), m_imbalances.end(),
        timestamp, [] (const auto& timestamp, const auto& imbalance) {
            return imbalance.m_timestamp > timestamp;
          });
      auto lower = m_imbalances.begin() +
        max(0, std::distance(m_imbalances.begin(), upper) - limit.GetSize());
      return std::make_tuple(lower, upper);
    }();
  return QtPromise<std::vector<OrderImbalance>>(
    [imbalances = std::vector<OrderImbalance>(lower_iter, upper_iter)] {
        return std::move(imbalances);
      });
}

SubscriptionResult<optional<Nexus::OrderImbalance>>
    LocalOrderImbalanceIndicatorModel::subscribe(
    const OrderImbalanceSignal::slot_type& slot) {
  auto last_imbalance = [&] () -> optional<Nexus::OrderImbalance> {
    if(m_imbalances.empty()) {
      return none;
    }
    return m_imbalances.back();
  }();
  return {m_imbalance_published_signal.connect(slot),
    QtPromise([imbalance = std::move(last_imbalance)] () mutable {
      return std::move(imbalance); })};
}

void LocalOrderImbalanceIndicatorModel::insert_sorted(
    const OrderImbalance& imbalance) {
  if(m_imbalances.empty() ||
      m_imbalances.back().m_timestamp < imbalance.m_timestamp) {
    m_imbalances.push_back(imbalance);
    return;
  }
  auto index = std::lower_bound(m_imbalances.begin(), m_imbalances.end(),
    imbalance.m_timestamp,
    [] (const auto& stored_imbalance, const auto& timestamp) {
      return stored_imbalance.m_timestamp < timestamp;
    });
  m_imbalances.insert(index, imbalance);
}

std::tuple<std::vector<OrderImbalance>::iterator,
    std::vector<OrderImbalance>::iterator>
    LocalOrderImbalanceIndicatorModel::get_iterators_from_interval(
    const TimeInterval& interval) {
  auto first = [&] {
    if(is_left_closed(interval.bounds())) {
      return std::lower_bound(m_imbalances.begin(), m_imbalances.end(),
        interval.lower(), [] (const auto& imbalance, const auto& timestamp) {
          return imbalance.m_timestamp < timestamp;
        });
    }
    return std::upper_bound(m_imbalances.begin(), m_imbalances.end(),
      interval.lower(), [] (const auto& timestamp, const auto& imbalance) {
        return imbalance.m_timestamp > timestamp;
      });
  }();
  auto last = [&] {
    if(is_right_closed(interval.bounds())) {
     return std::upper_bound(first, m_imbalances.end(),
        interval.upper(), [] (const auto& timestamp, const auto& imbalance) {
          return imbalance.m_timestamp > timestamp;
        });
    }
    return std::lower_bound(first, m_imbalances.end(),
      interval.upper(), [] (const auto& imbalance, const auto& timestamp) {
        return imbalance.m_timestamp < timestamp;
      });
  }();
  return {first, last};
}
