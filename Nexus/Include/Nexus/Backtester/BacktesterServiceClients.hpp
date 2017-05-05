#ifndef NEXUS_BACKTESTERSERVICECLIENTS_HPP
#define NEXUS_BACKTESTERSERVICECLIENTS_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/RegistryService/VirtualRegistryClient.hpp>
#include <Beam/ServiceLocator/VirtualServiceLocatorClient.hpp>
#include <Beam/Threading/VirtualTimer.hpp>
#include <Beam/TimeService/VirtualTimeClient.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Backtester/Backtester.hpp"
#include "Nexus/Backtester/BacktesterEnvironment.hpp"
#include "Nexus/Backtester/BacktesterMarketDataClient.hpp"
#include "Nexus/ChartingService/VirtualChartingClient.hpp"
#include "Nexus/Compliance/VirtualComplianceClient.hpp"
#include "Nexus/DefinitionsService/VirtualDefinitionsClient.hpp"
#include "Nexus/MarketDataService/VirtualMarketDataClient.hpp"
#include "Nexus/OrderExecutionService/VirtualOrderExecutionClient.hpp"
#include "Nexus/RiskService/VirtualRiskClient.hpp"

namespace Nexus {

  /*! \class BacktesterServiceClients
      \brief Implements the ServiceClients interface for the purpose of
             backtesting.
   */
  class BacktesterServiceClients : private boost::noncopyable {
    public:
      using ServiceLocatorClient =
        Beam::ServiceLocator::VirtualServiceLocatorClient;

      using RegistryClient = Beam::RegistryService::VirtualRegistryClient;

      using AdministrationClient =
        AdministrationService::VirtualAdministrationClient;

      using DefinitionsClient = DefinitionsService::VirtualDefinitionsClient;

      using MarketDataClient = MarketDataService::VirtualMarketDataClient;

      using ChartingClient = ChartingService::VirtualChartingClient;

      using ComplianceClient = Compliance::VirtualComplianceClient;

      using OrderExecutionClient =
        OrderExecutionService::VirtualOrderExecutionClient;

      using RiskClient = RiskService::VirtualRiskClient;

      using TimeClient = Beam::TimeService::VirtualTimeClient;

      using Timer = Beam::Threading::VirtualTimer;

      //! Constructs a BacktesterServiceClients.
      /*!
        \param environment The BacktesterEnvironment to use.
      */
      BacktesterServiceClients(
        Beam::RefType<BacktesterEnvironment> environment);

      ~BacktesterServiceClients();

      ServiceLocatorClient& GetServiceLocatorClient();

      RegistryClient& GetRegistryClient();

      AdministrationClient& GetAdministrationClient();

      DefinitionsClient& GetDefinitionsClient();

      MarketDataClient& GetMarketDataClient();

      ChartingClient& GetChartingClient();

      ComplianceClient& GetComplianceClient();

      OrderExecutionClient& GetOrderExecutionClient();

      RiskClient& GetRiskClient();

      TimeClient& GetTimeClient();

      std::unique_ptr<Timer> BuildTimer(
        boost::posix_time::time_duration expiry);

      void Open();

      void Close();

    private:
      BacktesterEnvironment* m_environment;
      std::unique_ptr<ServiceLocatorClient> m_serviceLocatorClient;
      std::unique_ptr<RegistryClient> m_registryClient;
      std::unique_ptr<AdministrationClient> m_administrationClient;
      std::unique_ptr<DefinitionsClient> m_definitionsClient;
      std::unique_ptr<MarketDataClient> m_marketDataClient;
      std::unique_ptr<ChartingClient> m_chartingClient;
      std::unique_ptr<ComplianceClient> m_complianceClient;
      std::unique_ptr<OrderExecutionClient> m_orderExecutionClient;
      std::unique_ptr<RiskClient> m_riskClient;
      std::unique_ptr<TimeClient> m_timeClient;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  inline BacktesterServiceClients::BacktesterServiceClients(
      Beam::RefType<BacktesterEnvironment> environment)
      : m_environment{environment.Get()} {}

  inline BacktesterServiceClients::~BacktesterServiceClients() {
    Close();
  }

  inline BacktesterServiceClients::ServiceLocatorClient&
      BacktesterServiceClients::GetServiceLocatorClient() {
    return *m_serviceLocatorClient;
  }

  inline BacktesterServiceClients::RegistryClient&
      BacktesterServiceClients::GetRegistryClient() {
    return *m_registryClient;
  }

  inline BacktesterServiceClients::AdministrationClient&
      BacktesterServiceClients::GetAdministrationClient() {
    return *m_administrationClient;
  }

  inline BacktesterServiceClients::DefinitionsClient&
      BacktesterServiceClients::GetDefinitionsClient() {
    return *m_definitionsClient;
  }

  inline BacktesterServiceClients::MarketDataClient&
      BacktesterServiceClients::GetMarketDataClient() {
    return *m_marketDataClient;
  }

  inline BacktesterServiceClients::ChartingClient&
      BacktesterServiceClients::GetChartingClient() {
    return *m_chartingClient;
  }

  inline BacktesterServiceClients::ComplianceClient&
      BacktesterServiceClients::GetComplianceClient() {
    return *m_complianceClient;
  }

  inline BacktesterServiceClients::OrderExecutionClient&
      BacktesterServiceClients::GetOrderExecutionClient() {
    return *m_orderExecutionClient;
  }

  inline BacktesterServiceClients::RiskClient&
      BacktesterServiceClients::GetRiskClient() {
    return *m_riskClient;
  }

  inline BacktesterServiceClients::TimeClient&
      BacktesterServiceClients::GetTimeClient() {
    return *m_timeClient;
  }

  inline std::unique_ptr<BacktesterServiceClients::Timer>
      BacktesterServiceClients::BuildTimer(
      boost::posix_time::time_duration expiry) {
    return Beam::Threading::MakeVirtualTimer(
      std::make_unique<Beam::TimeService::Tests::TestTimer>(expiry,
      Beam::Ref(m_environment->m_testEnvironment.GetTimeEnvironment())));
  }

  inline void BacktesterServiceClients::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_serviceLocatorClient =
        m_environment->m_testEnvironment.GetServiceLocatorEnvironment().
        BuildClient();
      m_serviceLocatorClient->SetCredentials("root", "");
      m_serviceLocatorClient->Open();
      m_definitionsClient = m_environment->m_testEnvironment.
        GetDefinitionsEnvironment().BuildClient(
        Beam::Ref(*m_serviceLocatorClient));
      m_definitionsClient->Open();
      m_administrationClient =
        m_environment->m_testEnvironment.GetAdministrationEnvironment().
        BuildClient(Beam::Ref(*m_serviceLocatorClient));
      m_administrationClient->Open();
      auto baseMarketDataClient = m_environment->m_testEnvironment.
        GetMarketDataEnvironment().BuildClient(
        Beam::Ref(*m_serviceLocatorClient));
      m_marketDataClient = MarketDataService::MakeVirtualMarketDataClient(
        std::make_unique<BacktesterMarketDataClient>(Beam::Ref(*m_environment),
        std::move(baseMarketDataClient)));
      m_marketDataClient->Open();
      m_orderExecutionClient = m_environment->m_testEnvironment.
        GetOrderExecutionEnvironment().BuildClient(
        Beam::Ref(*m_serviceLocatorClient));
      m_orderExecutionClient->Open();
      m_timeClient = Beam::TimeService::MakeVirtualTimeClient(
        std::make_unique<Beam::TimeService::Tests::TestTimeClient>(
        Beam::Ref(m_environment->m_testEnvironment.GetTimeEnvironment())));
      m_timeClient->Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  inline void BacktesterServiceClients::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void BacktesterServiceClients::Shutdown() {
    if(m_timeClient != nullptr) {
      m_timeClient->Close();
    }
    if(m_riskClient != nullptr) {
      m_riskClient->Close();
    }
    if(m_orderExecutionClient != nullptr) {
      m_orderExecutionClient->Close();
    }
    if(m_complianceClient != nullptr) {
      m_complianceClient->Close();
    }
    if(m_chartingClient != nullptr) {
      m_chartingClient->Close();
    }
    if(m_marketDataClient != nullptr) {
      m_marketDataClient->Close();
    }
    if(m_definitionsClient != nullptr) {
      m_definitionsClient->Close();
    }
    if(m_administrationClient != nullptr) {
      m_administrationClient->Close();
    }
    if(m_registryClient != nullptr) {
      m_registryClient->Close();
    }
    if(m_serviceLocatorClient != nullptr) {
      m_serviceLocatorClient->Close();
    }
    m_openState.SetClosed();
  }
}

#endif