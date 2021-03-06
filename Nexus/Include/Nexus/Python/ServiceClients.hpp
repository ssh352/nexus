#ifndef NEXUS_PYTHON_SERVICE_CLIENTS_HPP
#define NEXUS_PYTHON_SERVICE_CLIENTS_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Python/ToPythonServiceLocatorClient.hpp>
#include <Beam/Python/ToPythonTimeClient.hpp>
#include <Beam/Python/ToPythonTimer.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/Python/AdministrationClient.hpp"
#include "Nexus/Python/ComplianceClient.hpp"
#include "Nexus/Python/DefinitionsClient.hpp"
#include "Nexus/Python/MarketDataClient.hpp"
#include "Nexus/Python/OrderExecutionClient.hpp"
#include "Nexus/Python/RiskClient.hpp"
#include "Nexus/ServiceClients/VirtualServiceClients.hpp"

namespace Nexus {
namespace Python {

  /**
   * Exports the ApplicationServiceClients class.
   * @param module The module to export to.
   */
  void ExportApplicationServiceClients(pybind11::module& module);

  /**
   * Exports the ServiceClients classes.
   * @param module The module to export to.
   */
  void ExportServiceClients(pybind11::module& module);

  /**
   * Exports the TestEnvironment class.
   * @param module The module to export to.
   */
  void ExportTestEnvironment(pybind11::module& module);

  /**
   * Exports the TestServiceClients class.
   * @param module The module to export to.
   */
  void ExportTestServiceClients(pybind11::module& module);

  /**
   * Exports the VirtualServiceClients class.
   * @param module The module to export to.
   */
  void ExportVirtualServiceClients(pybind11::module& module);
}

  /**
   * Wraps a ServiceClients instance for use within Python.
   * @param <C> The type of ServiceClients to wrap.
   */
  template<typename C>
  class ToPythonServiceClients : public VirtualServiceClients {
    public:

      /** The type of ServiceClients to wrap. */
      using Client = C;

      /**
       * Constructs a ToPythonServiceClients instance.
       * @param client The ServiceClients to wrap.
       */
      ToPythonServiceClients(std::unique_ptr<Client> client);

      ~ToPythonServiceClients() override;

      ServiceLocatorClient& GetServiceLocatorClient() override final;

      RegistryClient& GetRegistryClient() override final;

      AdministrationClient& GetAdministrationClient() override final;

      DefinitionsClient& GetDefinitionsClient() override final;

      MarketDataClient& GetMarketDataClient() override final;

      ChartingClient& GetChartingClient() override final;

      ComplianceClient& GetComplianceClient() override final;

      OrderExecutionClient& GetOrderExecutionClient() override final;

      RiskClient& GetRiskClient() override final;

      TimeClient& GetTimeClient() override final;

      std::unique_ptr<Timer> BuildTimer(
        boost::posix_time::time_duration expiry);

      void Close() override;

    private:
      std::unique_ptr<Client> m_client;
      std::unique_ptr<ServiceLocatorClient> m_serviceLocatorClient;
      std::unique_ptr<AdministrationClient> m_administrationClient;
      std::unique_ptr<DefinitionsClient> m_definitionsClient;
      std::unique_ptr<MarketDataClient> m_marketDataClient;
      std::unique_ptr<ComplianceClient> m_complianceClient;
      std::unique_ptr<OrderExecutionClient> m_orderExecutionClient;
      std::unique_ptr<RiskClient> m_riskClient;
      std::unique_ptr<TimeClient> m_timeClient;
      Beam::IO::OpenState m_openState;
  };

  /**
   * Builds a ToPythonServiceClients instance.
   * @param client The ServiceClients instance to wrap.
   */
  template<typename Client>
  auto MakeToPythonServiceClients(std::unique_ptr<Client> client) {
    return std::make_shared<ToPythonServiceClients<Client>>(std::move(client));
  }

  template<typename C>
  ToPythonServiceClients<C>::ToPythonServiceClients(
    std::unique_ptr<Client> client)
    : m_client(std::move(client)),
      m_serviceLocatorClient(
        Beam::ServiceLocator::MakeToPythonServiceLocatorClient(
        Beam::ServiceLocator::MakeVirtualServiceLocatorClient(
        &m_client->GetServiceLocatorClient()))),
      m_administrationClient(
        AdministrationService::MakeToPythonAdministrationClient(
        AdministrationService::MakeVirtualAdministrationClient(
        &m_client->GetAdministrationClient()))),
      m_definitionsClient(DefinitionsService::MakeToPythonDefinitionsClient(
        DefinitionsService::MakeVirtualDefinitionsClient(
        &m_client->GetDefinitionsClient()))),
      m_marketDataClient(MarketDataService::MakeToPythonMarketDataClient(
        MarketDataService::MakeVirtualMarketDataClient(
        &m_client->GetMarketDataClient()))),
      m_complianceClient(Compliance::MakeToPythonComplianceClient(
        Compliance::MakeVirtualComplianceClient(
        &m_client->GetComplianceClient()))),
      m_orderExecutionClient(
        OrderExecutionService::MakeToPythonOrderExecutionClient(
        OrderExecutionService::MakeVirtualOrderExecutionClient(
        &m_client->GetOrderExecutionClient()))),
      m_riskClient(RiskService::MakeToPythonRiskClient(
        RiskService::MakeVirtualRiskClient(&m_client->GetRiskClient()))),
      m_timeClient(Beam::TimeService::MakeToPythonTimeClient(
        Beam::TimeService::MakeVirtualTimeClient(
        &m_client->GetTimeClient()))) {}

  template<typename C>
  ToPythonServiceClients<C>::~ToPythonServiceClients() {
    Close();
    auto release = Beam::Python::GilRelease();
    m_timeClient.reset();
    m_riskClient.reset();
    m_orderExecutionClient.reset();
    m_complianceClient.reset();
    m_marketDataClient.reset();
    m_definitionsClient.reset();
    m_administrationClient.reset();
    m_serviceLocatorClient.reset();
    m_client.reset();
  }

  template<typename C>
  typename ToPythonServiceClients<C>::ServiceLocatorClient&
      ToPythonServiceClients<C>::GetServiceLocatorClient() {
    return *m_serviceLocatorClient;
  }

  template<typename C>
  typename ToPythonServiceClients<C>::RegistryClient&
      ToPythonServiceClients<C>::GetRegistryClient() {
    throw std::runtime_error{"Not implemented"};
  }

  template<typename C>
  typename ToPythonServiceClients<C>::AdministrationClient&
      ToPythonServiceClients<C>::GetAdministrationClient() {
    return *m_administrationClient;
  }

  template<typename C>
  typename ToPythonServiceClients<C>::DefinitionsClient&
      ToPythonServiceClients<C>::GetDefinitionsClient() {
    return *m_definitionsClient;
  }

  template<typename C>
  typename ToPythonServiceClients<C>::MarketDataClient&
      ToPythonServiceClients<C>::GetMarketDataClient() {
    return *m_marketDataClient;
  }

  template<typename C>
  typename ToPythonServiceClients<C>::ChartingClient&
      ToPythonServiceClients<C>::GetChartingClient() {
    throw std::runtime_error("Not implemented");
  }

  template<typename C>
  typename ToPythonServiceClients<C>::ComplianceClient&
      ToPythonServiceClients<C>::GetComplianceClient() {
    return *m_complianceClient;
  }

  template<typename C>
  typename ToPythonServiceClients<C>::OrderExecutionClient&
      ToPythonServiceClients<C>::GetOrderExecutionClient() {
    return *m_orderExecutionClient;
  }

  template<typename C>
  typename ToPythonServiceClients<C>::RiskClient&
      ToPythonServiceClients<C>::GetRiskClient() {
    return *m_riskClient;
  }

  template<typename C>
  typename ToPythonServiceClients<C>::TimeClient&
      ToPythonServiceClients<C>::GetTimeClient() {
    return *m_timeClient;
  }

  template<typename C>
  std::unique_ptr<typename ToPythonServiceClients<C>::Timer>
      ToPythonServiceClients<C>::BuildTimer(
      boost::posix_time::time_duration expiry) {
    auto release = Beam::Python::GilRelease();
    return Beam::Threading::MakeToPythonTimer(m_client->BuildTimer(expiry));
  }

  template<typename C>
  void ToPythonServiceClients<C>::Close() {
    auto release = Beam::Python::GilRelease();
    if(m_openState.SetClosing()) {
      return;
    }
    m_timeClient->Close();
    m_riskClient->Close();
    m_orderExecutionClient->Close();
    m_complianceClient->Close();
    m_marketDataClient->Close();
    m_definitionsClient->Close();
    m_administrationClient->Close();
    m_serviceLocatorClient->Close();
    m_client->Close();
    m_openState.Close();
  }
}

#endif
