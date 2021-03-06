#ifndef NEXUS_ADMINISTRATION_SERVICE_TEST_ENVIRONMENT_HPP
#define NEXUS_ADMINISTRATION_SERVICE_TEST_ENVIRONMENT_HPP
#include <Beam/IO/LocalClientChannel.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <boost/functional/factory.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "Nexus/AdministrationService/AdministrationServlet.hpp"
#include "Nexus/AdministrationService/LocalAdministrationDataStore.hpp"
#include "Nexus/AdministrationService/VirtualAdministrationClient.hpp"
#include "Nexus/AdministrationServiceTests/AdministrationServiceTests.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/MarketDataService/EntitlementSet.hpp"

namespace Nexus::AdministrationService::Tests {

  /**
   * Wraps most components needed to run an instance of the
   * AdministrationService with helper functions.
   */
  class AdministrationServiceTestEnvironment {
    public:

      /**
       * Constructs an AdministrationServiceTestEnvironment.
       * @param serviceLocatorClient The ServiceLocatorClient to use.
       */
      AdministrationServiceTestEnvironment(
        std::shared_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>
        serviceLocatorClient);

      /**
       * Constructs an AdministrationServiceTestEnvironment.
       * @param serviceLocatorClient The ServiceLocatorClient to use.
       * @param entitlements The entitlement database to use.
       */
      AdministrationServiceTestEnvironment(
        std::shared_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>
        serviceLocatorClient,
        MarketDataService::EntitlementDatabase entitlements);

      ~AdministrationServiceTestEnvironment();

      /**
       * Grants an account administrative privileges.
       * @param account The account to grant administrative privileges to.
       */
      void MakeAdministrator(
        const Beam::ServiceLocator::DirectoryEntry& account);

      /**
       * Builds a new AdministrationClient.
       * @param serviceLocatorClient The ServiceLocatorClient used to
       *        authenticate the AdministrationClient.
       */
      std::unique_ptr<VirtualAdministrationClient> BuildClient(
        Beam::Ref<Beam::ServiceLocator::VirtualServiceLocatorClient>
        serviceLocatorClient);

      void Close();

    private:
      using ServerConnection =
        Beam::IO::LocalServerConnection<Beam::IO::SharedBuffer>;
      using ClientChannel =
        Beam::IO::LocalClientChannel<Beam::IO::SharedBuffer>;
      using ServiceLocatorClient =
        Beam::ServiceLocator::VirtualServiceLocatorClient;
      using ServiceProtocolServletContainer =
        Beam::Services::ServiceProtocolServletContainer<
        Beam::ServiceLocator::MetaAuthenticationServletAdapter<
        MetaAdministrationServlet<std::shared_ptr<ServiceLocatorClient>,
        LocalAdministrationDataStore*>, std::shared_ptr<ServiceLocatorClient>>,
        ServerConnection*, Beam::Serialization::BinarySender<
        Beam::IO::SharedBuffer>, Beam::Codecs::NullEncoder,
        std::shared_ptr<Beam::Threading::TriggerTimer>>;
      using ServiceProtocolClientBuilder =
        Beam::Services::AuthenticatedServiceProtocolClientBuilder<
        ServiceLocatorClient, Beam::Services::MessageProtocol<
        std::unique_ptr<ClientChannel>,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder>, Beam::Threading::TriggerTimer>;
      std::shared_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>
        m_serviceLocatorClient;
      LocalAdministrationDataStore m_dataStore;
      ServerConnection m_serverConnection;
      ServiceProtocolServletContainer m_container;

      static MarketDataService::EntitlementDatabase MakeDefaultEntitlements(
        ServiceLocatorClient& client);
      AdministrationServiceTestEnvironment(
        const AdministrationServiceTestEnvironment&) = delete;
      AdministrationServiceTestEnvironment& operator =(
        const AdministrationServiceTestEnvironment&) = delete;
  };

  inline AdministrationServiceTestEnvironment::
    AdministrationServiceTestEnvironment(
    std::shared_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>
    serviceLocatorClient)
    : AdministrationServiceTestEnvironment(serviceLocatorClient,
        MakeDefaultEntitlements(*serviceLocatorClient)) {}

  inline AdministrationServiceTestEnvironment::
    AdministrationServiceTestEnvironment(
    std::shared_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>
    serviceLocatorClient, MarketDataService::EntitlementDatabase entitlements)
    : m_serviceLocatorClient(std::move(serviceLocatorClient)),
      m_container(Beam::Initialize(m_serviceLocatorClient, Beam::Initialize(
        m_serviceLocatorClient, std::move(entitlements), &m_dataStore)),
        &m_serverConnection,
        boost::factory<std::shared_ptr<Beam::Threading::TriggerTimer>>()) {}

  inline AdministrationServiceTestEnvironment::
      ~AdministrationServiceTestEnvironment() {
    Close();
  }

  inline void AdministrationServiceTestEnvironment::MakeAdministrator(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto administrators = m_serviceLocatorClient->LoadDirectoryEntry(
      Beam::ServiceLocator::DirectoryEntry::GetStarDirectory(),
      "administrators");
    m_serviceLocatorClient->Associate(account, administrators);
  }

  inline std::unique_ptr<VirtualAdministrationClient>
      AdministrationServiceTestEnvironment::BuildClient(
      Beam::Ref<Beam::ServiceLocator::VirtualServiceLocatorClient>
      serviceLocatorClient) {
    auto builder = ServiceProtocolClientBuilder(Beam::Ref(serviceLocatorClient),
      [=] {
        return std::make_unique<ServiceProtocolClientBuilder::Channel>(
          "test_administration_client", m_serverConnection);
      },
      [] {
        return std::make_unique<ServiceProtocolClientBuilder::Timer>();
      });
    return MakeVirtualAdministrationClient(
      std::make_unique<AdministrationClient<ServiceProtocolClientBuilder>>(
      builder));
  }

  inline void AdministrationServiceTestEnvironment::Close() {
    m_container.Close();
  }

  inline MarketDataService::EntitlementDatabase
      AdministrationServiceTestEnvironment::MakeDefaultEntitlements(
      ServiceLocatorClient& client) {
    auto entitlementsDirectory = client.MakeDirectory("entitlements",
      Beam::ServiceLocator::DirectoryEntry::GetStarDirectory());
    auto globalEntitlementGroup = client.MakeDirectory("global",
      entitlementsDirectory);
    client.Associate(client.GetAccount(), globalEntitlementGroup);
    auto globalEntitlement = MarketDataService::EntitlementDatabase::Entry();
    globalEntitlement.m_name = "global";
    globalEntitlement.m_groupEntry = globalEntitlementGroup;
    auto& marketDatabase = GetDefaultMarketDatabase();
    auto markets = std::vector<MarketCode>();
    for(auto& entry : marketDatabase.GetEntries()) {
      markets.push_back(entry.m_code);
    }
    markets.push_back(MarketCode());
    for(auto& market : markets) {
      globalEntitlement.m_applicability[
        MarketDataService::EntitlementKey(market)].Set(
        MarketDataService::MarketDataType::TIME_AND_SALE);
      globalEntitlement.m_applicability[
        MarketDataService::EntitlementKey(market)].Set(
        MarketDataService::MarketDataType::BOOK_QUOTE);
      globalEntitlement.m_applicability[
        MarketDataService::EntitlementKey(market)].Set(
        MarketDataService::MarketDataType::MARKET_QUOTE);
      globalEntitlement.m_applicability[
        MarketDataService::EntitlementKey(market)].Set(
        MarketDataService::MarketDataType::BBO_QUOTE);
      globalEntitlement.m_applicability[
        MarketDataService::EntitlementKey(market)].Set(
        MarketDataService::MarketDataType::ORDER_IMBALANCE);
    }
    auto entitlements = MarketDataService::EntitlementDatabase();
    entitlements.Add(globalEntitlement);
    return entitlements;
  }
}

#endif
