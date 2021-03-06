#ifndef NEXUS_ORDEREXECUTIONSERVICETESTENVIRONMENT_HPP
#define NEXUS_ORDEREXECUTIONSERVICETESTENVIRONMENT_HPP
#include <Beam/IO/LocalClientChannel.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/UniquePointerPolicy.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/ServiceLocator/SessionAuthenticator.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <Beam/TimeService/IncrementalTimeClient.hpp>
#include <Beam/TimeService/VirtualTimeClient.hpp>
#include <Beam/UidServiceTests/UidServiceTestEnvironment.hpp>
#include <boost/functional/factory.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/Definitions/Destination.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionClient.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionServlet.hpp"
#include "Nexus/OrderExecutionService/LocalOrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionService/VirtualOrderExecutionClient.hpp"
#include "Nexus/OrderExecutionService/VirtualOrderExecutionDriver.hpp"
#include "Nexus/OrderExecutionServiceTests/MockOrderExecutionDriver.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceTests.hpp"

namespace Nexus::OrderExecutionService::Tests {

  /*! \class OrderExecutionServiceTestEnvironment
      \brief Wraps most components needed to run an instance of the
             OrderExecutionService with helper functions.
   */
  class OrderExecutionServiceTestEnvironment : private boost::noncopyable {
    public:

      //! Constructs an OrderExecutionServiceTestEnvironment.
      /*!
        \param marketDatabase The MarketDatabase to use.
        \param destinationDatabase The DestinationDatabase to use.
        \param serviceLocatorClient The ServiceLocatorClient to use.
        \param uidClient The UidClient to use.
        \param administrationClient The AdministrationClient to use.
      */
      OrderExecutionServiceTestEnvironment(MarketDatabase marketDatabase,
        DestinationDatabase destinationDatabase,
        std::shared_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>
        serviceLocatorClient,
        std::shared_ptr<Beam::UidService::VirtualUidClient> uidClient,
        std::shared_ptr<AdministrationService::VirtualAdministrationClient>
        administrationClient);

      //! Constructs an OrderExecutionServiceTestEnvironment.
      /*!
        \param marketDatabase The MarketDatabase to use.
        \param destinationDatabase The DestinationDatabase to use.
        \param serviceLocatorClient The ServiceLocatorClient to use.
        \param uidClient The UidClient to use.
        \param administrationClient The AdministrationClient to use.
        \param timeClient The TimeClient to use.
        \param driver The OrderExecutionDriver to use.
      */
      OrderExecutionServiceTestEnvironment(MarketDatabase marketDatabase,
        DestinationDatabase destinationDatabase,
        std::shared_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>
        serviceLocatorClient,
        std::shared_ptr<Beam::UidService::VirtualUidClient> uidClient,
        std::shared_ptr<AdministrationService::VirtualAdministrationClient>
        administrationClient,
        std::shared_ptr<Beam::TimeService::VirtualTimeClient> timeClient,
        std::unique_ptr<VirtualOrderExecutionDriver> driver);

      ~OrderExecutionServiceTestEnvironment();

      //! Returns the driver used to manage submitted Orders.
      VirtualOrderExecutionDriver& GetDriver();

      //! Builds a new OrderExecutionClient.
      /*!
        \param serviceLocatorClient The ServiceLocatorClient used to
               authenticate the OrderExecutionClient.
      */
      std::unique_ptr<VirtualOrderExecutionClient> BuildClient(
        Beam::Ref<Beam::ServiceLocator::VirtualServiceLocatorClient>
        serviceLocatorClient);

      //! Closes the servlet.
      void Close();

    private:
      using ServerConnection =
        Beam::IO::LocalServerConnection<Beam::IO::SharedBuffer>;
      using ClientChannel =
        Beam::IO::LocalClientChannel<Beam::IO::SharedBuffer>;
      using ServiceLocatorClient =
        Beam::ServiceLocator::VirtualServiceLocatorClient;
      using UidClient = Beam::UidService::VirtualUidClient;
      using AdministrationClient =
        AdministrationService::VirtualAdministrationClient;
      using ServiceProtocolServletContainer =
        Beam::Services::ServiceProtocolServletContainer<
        Beam::ServiceLocator::MetaAuthenticationServletAdapter<
        MetaOrderExecutionServlet<
        std::shared_ptr<Beam::TimeService::VirtualTimeClient>,
        std::shared_ptr<ServiceLocatorClient>, std::shared_ptr<UidClient>,
        std::shared_ptr<AdministrationClient>, VirtualOrderExecutionDriver*,
        LocalOrderExecutionDataStore*>, std::shared_ptr<ServiceLocatorClient>>,
        ServerConnection*,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder,
        std::shared_ptr<Beam::Threading::TriggerTimer>>;
      using ServiceProtocolClientBuilder =
        Beam::Services::AuthenticatedServiceProtocolClientBuilder<
        ServiceLocatorClient, Beam::Services::MessageProtocol<
        std::unique_ptr<ClientChannel>,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder>, Beam::Threading::TriggerTimer>;
      LocalOrderExecutionDataStore m_dataStore;
      std::unique_ptr<VirtualOrderExecutionDriver> m_driver;
      ServerConnection m_serverConnection;
      ServiceProtocolServletContainer m_container;
  };

  inline OrderExecutionServiceTestEnvironment::
    OrderExecutionServiceTestEnvironment(MarketDatabase marketDatabase,
    DestinationDatabase destinationDatabase,
    std::shared_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>
    serviceLocatorClient,
    std::shared_ptr<Beam::UidService::VirtualUidClient> uidClient,
    std::shared_ptr<AdministrationService::VirtualAdministrationClient>
    administrationClient)
    : OrderExecutionServiceTestEnvironment(std::move(marketDatabase),
        std::move(destinationDatabase), std::move(serviceLocatorClient),
        std::move(uidClient), std::move(administrationClient),
        Beam::TimeService::MakeVirtualTimeClient<
        Beam::TimeService::IncrementalTimeClient>(Beam::Initialize()),
        MakeVirtualOrderExecutionDriver<MockOrderExecutionDriver>(
        Beam::Initialize())) {}

  inline OrderExecutionServiceTestEnvironment::
    OrderExecutionServiceTestEnvironment(MarketDatabase marketDatabase,
    DestinationDatabase destinationDatabase,
    std::shared_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>
    serviceLocatorClient,
    std::shared_ptr<Beam::UidService::VirtualUidClient> uidClient,
    std::shared_ptr<AdministrationService::VirtualAdministrationClient>
    administrationClient,
    std::shared_ptr<Beam::TimeService::VirtualTimeClient> timeClient,
    std::unique_ptr<VirtualOrderExecutionDriver> driver)
    : m_driver(std::move(driver)),
      m_container(Beam::Initialize(serviceLocatorClient,
        Beam::Initialize(boost::posix_time::pos_infin,
        std::move(marketDatabase), std::move(destinationDatabase),
        std::move(timeClient), serviceLocatorClient, std::move(uidClient),
        std::move(administrationClient), &*m_driver, &m_dataStore)),
        &m_serverConnection,
        boost::factory<std::shared_ptr<Beam::Threading::TriggerTimer>>()) {}

  inline OrderExecutionServiceTestEnvironment::
      ~OrderExecutionServiceTestEnvironment() {
    Close();
  }

  inline VirtualOrderExecutionDriver& OrderExecutionServiceTestEnvironment::
      GetDriver() {
    return *m_driver;
  }

  inline std::unique_ptr<VirtualOrderExecutionClient>
      OrderExecutionServiceTestEnvironment::BuildClient(
      Beam::Ref<Beam::ServiceLocator::VirtualServiceLocatorClient>
      serviceLocatorClient) {
    ServiceProtocolClientBuilder builder{Beam::Ref(serviceLocatorClient),
      [=] {
        return std::make_unique<ServiceProtocolClientBuilder::Channel>(
          "test_order_execution_client", m_serverConnection);
      },
      [] {
        return std::make_unique<ServiceProtocolClientBuilder::Timer>();
      }};
    return MakeVirtualOrderExecutionClient(
      std::make_unique<OrderExecutionClient<ServiceProtocolClientBuilder>>(
      builder));
  }

  inline void OrderExecutionServiceTestEnvironment::Close() {
    m_container.Close();
  }
}

#endif
