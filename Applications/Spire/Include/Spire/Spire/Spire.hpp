#ifndef SPIRE_HPP
#define SPIRE_HPP
#include <Beam/Queues/AggregatePublisher.hpp>
#include <Beam/Queues/FilteredPublisher.hpp>
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/Accounting/PortfolioMonitor.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Spire {
  class ServiceClients;
  using SpirePosition = Nexus::Accounting::Position<Nexus::Security>;
  using SpireInventory = Nexus::Accounting::Inventory<SpirePosition>;
  using SpireBookkeeper = Nexus::Accounting::TrueAverageBookkeeper<
    SpireInventory>;
  using SpirePortfolio = Nexus::Accounting::Portfolio<SpireBookkeeper>;
  using SpirePortfolioMonitor = Nexus::Accounting::PortfolioMonitor<
    SpirePortfolio, Nexus::MarketDataService::VirtualMarketDataClient*>;
  using SpireAggregateOrderExecutionPublisher = Beam::AggregatePublisher<
    Beam::FilteredPublisher<Beam::SequencePublisher<
    const Nexus::OrderExecutionService::Order*>>>;
  class UserProfile;
}

#endif