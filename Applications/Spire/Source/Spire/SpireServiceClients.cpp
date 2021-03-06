#include "Spire/Spire/SpireServiceClients.hpp"
#include <stdexcept>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/ServiceLocator/VirtualServiceLocatorClient.hpp>
#include <Beam/RegistryService/ApplicationDefinitions.hpp>
#include <Beam/RegistryService/VirtualRegistryClient.hpp>
#include <Beam/Threading/VirtualTimer.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
#include <Beam/TimeService/VirtualTimeClient.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/AdministrationService/ApplicationDefinitions.hpp"
#include "Nexus/AdministrationService/VirtualAdministrationClient.hpp"
#include "Nexus/ChartingService/ApplicationDefinitions.hpp"
#include "Nexus/ChartingService/VirtualChartingClient.hpp"
#include "Nexus/Compliance/ApplicationDefinitions.hpp"
#include "Nexus/Compliance/VirtualComplianceClient.hpp"
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/DefinitionsService/VirtualDefinitionsClient.hpp"
#include "Nexus/MarketDataService/ApplicationDefinitions.hpp"
#include "Nexus/MarketDataService/VirtualMarketDataClient.hpp"
#include "Nexus/OrderExecutionService/ApplicationDefinitions.hpp"
#include "Nexus/OrderExecutionService/VirtualOrderExecutionClient.hpp"
#include "Nexus/RiskService/ApplicationDefinitions.hpp"
#include "Nexus/RiskService/VirtualRiskClient.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Parsers;
using namespace Beam::RegistryService;
using namespace Beam::ServiceLocator;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::ChartingService;
using namespace Nexus::Compliance;
using namespace Nexus::DefinitionsService;
using namespace Nexus::MarketDataService;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::RiskService;
using namespace Spire;
using namespace std;

namespace {
  template<typename T>
  struct ByPassPtr {
    using Type = GetDereferenceType<T>;

    std::unique_ptr<T> m_value;

    ByPassPtr(std::unique_ptr<T> value)
        : m_value{std::move(value)} {}

    ByPassPtr(ByPassPtr&& ptr)
        : m_value{std::move(ptr.m_value)} {}

    Type& operator *() const {
      return **m_value;
    }

    Type* operator ->() const {
      return &**m_value;
    }
  };
}

SpireServiceClients::SpireServiceClients(
  std::unique_ptr<ApplicationServiceLocatorClient> serviceLocatorClient,
  Ref<SocketThreadPool> socketThreadPool,
  Ref<TimerThreadPool> timerThreadPool)
  : m_applicationServiceLocatorClient{std::move(serviceLocatorClient)},
    m_serviceLocatorClient{MakeVirtualServiceLocatorClient(
      &**m_applicationServiceLocatorClient)},
    m_socketThreadPool{socketThreadPool.Get()},
    m_timerThreadPool{timerThreadPool.Get()} {
  try {
    auto definitionsClient = std::make_unique<ApplicationDefinitionsClient>();
    definitionsClient->BuildSession(
      Ref(*(m_applicationServiceLocatorClient->Get())),
      Ref(*m_socketThreadPool), Ref(*m_timerThreadPool));
    auto minimumVersion = (*definitionsClient)->LoadMinimumSpireClientVersion();
    if(minimumVersion > string{SPIRE_VERSION}) {
      BOOST_THROW_EXCEPTION(std::runtime_error{
        ("Spire version incompatible.\n"
        "Minimum version required: ") +
        minimumVersion + ("\n"
        "Current version installed: ") + string{SPIRE_VERSION}});
    }
    m_definitionsClient = MakeVirtualDefinitionsClient(ByPassPtr(
      std::move(definitionsClient)));
  } catch(const std::exception&) {
    BOOST_THROW_EXCEPTION(ConnectException{
      "Unable to connect to the definitions service."});
  }
  try {
    auto registryClient = std::make_unique<ApplicationRegistryClient>();
    registryClient->BuildSession(
      Ref(*(m_applicationServiceLocatorClient->Get())),
      Ref(*m_socketThreadPool), Ref(*m_timerThreadPool));
    m_registryClient = MakeVirtualRegistryClient(ByPassPtr(
      std::move(registryClient)));
  } catch(const std::exception&) {
    BOOST_THROW_EXCEPTION(ConnectException{
      "Unable to connect to the registry service."});
  }
  try {
    auto administrationClient =
      std::make_unique<ApplicationAdministrationClient>();
    administrationClient->BuildSession(
      Ref(*(m_applicationServiceLocatorClient->Get())),
      Ref(*m_socketThreadPool), Ref(*m_timerThreadPool));
    m_administrationClient = MakeVirtualAdministrationClient(ByPassPtr(
      std::move(administrationClient)));
  } catch(const std::exception&) {
    BOOST_THROW_EXCEPTION(ConnectException{
      "Unable to connect to the administration service."});
  }
  try {
    auto marketDataClient = std::make_unique<ApplicationMarketDataClient>();
    marketDataClient->BuildSession(
      Ref(*(m_applicationServiceLocatorClient->Get())),
      Ref(*m_socketThreadPool), Ref(*m_timerThreadPool));
    m_marketDataClient = MakeVirtualMarketDataClient(ByPassPtr(
      std::move(marketDataClient)));
  } catch(const std::exception&) {
    BOOST_THROW_EXCEPTION(ConnectException{
      "Unable to connect to the market data service."});
  }
  try {
    auto chartingClient = std::make_unique<ApplicationChartingClient>();
    chartingClient->BuildSession(
      Ref(*(m_applicationServiceLocatorClient->Get())),
      Ref(*m_socketThreadPool), Ref(*m_timerThreadPool));
    m_chartingClient = MakeVirtualChartingClient(ByPassPtr(
      std::move(chartingClient)));
  } catch(const std::exception&) {
    BOOST_THROW_EXCEPTION(ConnectException{
      "Unable to connect to the charting service."});
  }
  try {
    auto complianceClient = std::make_unique<ApplicationComplianceClient>();
    complianceClient->BuildSession(
      Ref(*(m_applicationServiceLocatorClient->Get())),
      Ref(*m_socketThreadPool), Ref(*m_timerThreadPool));
    m_complianceClient = MakeVirtualComplianceClient(ByPassPtr(
      std::move(complianceClient)));
  } catch(const std::exception&) {
    BOOST_THROW_EXCEPTION(ConnectException{
      "Unable to connect to the compliance service."});
  }
  try {
    auto timeServices = m_serviceLocatorClient->Locate(
      TimeService::SERVICE_NAME);
    if(timeServices.empty()) {
      BOOST_THROW_EXCEPTION(ConnectException{"No time services available."});
    }
    auto& timeService = timeServices.front();
    auto ntpPool = Parse<vector<IpAddress>>(get<string>(
      timeService.GetProperties().At("addresses")));
    auto timeClient = MakeLiveNtpTimeClient(ntpPool, Ref(*m_socketThreadPool),
      Ref(*m_timerThreadPool));
    m_timeClient = MakeVirtualTimeClient(std::move(timeClient));
  } catch(std::exception&) {
    BOOST_THROW_EXCEPTION(ConnectException{
      "Unable to connect to the time service."});
  }
  try {
    auto orderExecutionClient =
      std::make_unique<ApplicationOrderExecutionClient>();
    orderExecutionClient->BuildSession(
      Ref(*(m_applicationServiceLocatorClient->Get())),
      Ref(*m_socketThreadPool), Ref(*m_timerThreadPool));
    m_orderExecutionClient = MakeVirtualOrderExecutionClient(ByPassPtr(
      std::move(orderExecutionClient)));
  } catch(const std::exception&) {
    BOOST_THROW_EXCEPTION(ConnectException{
      "Unable to connect to the order execution service."});
  }
  try {
    auto riskClient = std::make_unique<ApplicationRiskClient>();
    riskClient->BuildSession(
      Ref(*(m_applicationServiceLocatorClient->Get())),
      Ref(*m_socketThreadPool), Ref(*m_timerThreadPool));
    m_riskClient = MakeVirtualRiskClient(ByPassPtr(std::move(riskClient)));
  } catch(const std::exception&) {
    BOOST_THROW_EXCEPTION(ConnectException{
      "Unable to connect to the risk service."});
  }
}

SpireServiceClients::~SpireServiceClients() {
  Close();
}

VirtualServiceLocatorClient&
    SpireServiceClients::GetServiceLocatorClient() const {
  return *m_serviceLocatorClient;
}

VirtualRegistryClient& SpireServiceClients::GetRegistryClient() const {
  return *m_registryClient;
}

VirtualAdministrationClient&
    SpireServiceClients::GetAdministrationClient() const {
  return *m_administrationClient;
}

VirtualDefinitionsClient& SpireServiceClients::GetDefinitionsClient() const {
  return *m_definitionsClient;
}

VirtualChartingClient& SpireServiceClients::GetChartingClient() const {
  return *m_chartingClient;
}

VirtualComplianceClient& SpireServiceClients::GetComplianceClient() const {
  return *m_complianceClient;
}

VirtualMarketDataClient& SpireServiceClients::GetMarketDataClient() const {
  return *m_marketDataClient;
}

VirtualOrderExecutionClient&
    SpireServiceClients::GetOrderExecutionClient() const {
  return *m_orderExecutionClient;
}

VirtualRiskClient& SpireServiceClients::GetRiskClient() const {
  return *m_riskClient;
}

VirtualTimeClient& SpireServiceClients::GetTimeClient() const {
  return *m_timeClient;
}

std::unique_ptr<VirtualTimer> SpireServiceClients::BuildTimer(
    time_duration expiry) {
  return MakeVirtualTimer(
    std::make_unique<LiveTimer>(expiry, Ref(*m_timerThreadPool)));
}

void SpireServiceClients::Close() {
  m_timeClient->Close();
  m_riskClient->Close();
  m_orderExecutionClient->Close();
  m_marketDataClient->Close();
  m_complianceClient->Close();
  m_chartingClient->Close();
  m_administrationClient->Close();
  m_registryClient->Close();
  m_definitionsClient->Close();
  m_serviceLocatorClient->Close();
}
