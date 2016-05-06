#include "Nexus/Python/MarketDataService.hpp"
#include <Beam/Codecs/SizeDeclarativeDecoder.hpp>
#include <Beam/Codecs/SizeDeclarativeEncoder.hpp>
#include <Beam/Codecs/ZLibDecoder.hpp>
#include <Beam/Codecs/ZLibEncoder.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Python/BoostPython.hpp>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Python/PythonBindings.hpp>
#include <Beam/Python/PythonQueueWriter.hpp>
#include <Beam/Python/Queries.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClient.hpp>
#include <Beam/ServiceLocator/VirtualServiceLocatorClient.hpp>
#include <Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp>
#include <Beam/Services/ServiceProtocolClientBuilder.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/MarketDataService/MarketWideDataQuery.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/MarketDataService/VirtualMarketDataClient.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestInstance.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Python;
using namespace Beam::Queries;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::python;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Nexus::MarketDataService::Tests;
using namespace Nexus::Python;
using namespace std;

namespace {
  using SessionBuilder = AuthenticatedServiceProtocolClientBuilder<
    VirtualServiceLocatorClient, MessageProtocol<
    std::unique_ptr<Beam::Network::TcpSocketChannel>,
    BinarySender<SharedBuffer>, SizeDeclarativeEncoder<ZLibEncoder>>,
    LiveTimer>;
  using Client = MarketDataClient<SessionBuilder>;

  class PythonMarketDataClient : public WrapperMarketDataClient<
      std::unique_ptr<Client>> {
    public:
      PythonMarketDataClient(std::unique_ptr<Client> client)
          : WrapperMarketDataClient<std::unique_ptr<Client>>(
              std::move(client)) {}

      void QueryOrderImbalances(const MarketWideDataQuery& query,
          const std::shared_ptr<PythonQueueWriter>& queue) {
        WrapperMarketDataClient<std::unique_ptr<Client>>::QueryOrderImbalances(
          query, queue->GetSlot<OrderImbalance>());
      }

      void QuerySequencedOrderImbalances(const MarketWideDataQuery& query,
          const std::shared_ptr<PythonQueueWriter>& queue) {
        WrapperMarketDataClient<std::unique_ptr<Client>>::QueryOrderImbalances(
          query, queue->GetSlot<SequencedOrderImbalance>());
      }

      void QueryBboQuotes(const SecurityMarketDataQuery& query,
          const std::shared_ptr<PythonQueueWriter>& queue) {
        WrapperMarketDataClient<std::unique_ptr<Client>>::QueryBboQuotes(
          query, queue->GetSlot<BboQuote>());
      }

      void QuerySequencedBboQuotes(const SecurityMarketDataQuery& query,
          const std::shared_ptr<PythonQueueWriter>& queue) {
        WrapperMarketDataClient<std::unique_ptr<Client>>::QueryBboQuotes(query,
          queue->GetSlot<SequencedBboQuote>());
      }

      void QueryBookQuotes(const SecurityMarketDataQuery& query,
          const std::shared_ptr<PythonQueueWriter>& queue) {
        WrapperMarketDataClient<std::unique_ptr<Client>>::QueryBookQuotes(
          query, queue->GetSlot<BookQuote>());
      }

      void QuerySequencedBookQuotes(const SecurityMarketDataQuery& query,
          const std::shared_ptr<PythonQueueWriter>& queue) {
        WrapperMarketDataClient<std::unique_ptr<Client>>::QueryBookQuotes(query,
          queue->GetSlot<SequencedBookQuote>());
      }

      void QueryMarketQuotes(const SecurityMarketDataQuery& query,
          const std::shared_ptr<PythonQueueWriter>& queue) {
        WrapperMarketDataClient<std::unique_ptr<Client>>::QueryMarketQuotes(
          query, queue->GetSlot<MarketQuote>());
      }

      void QuerySequencedMarketQuotes(const SecurityMarketDataQuery& query,
          const std::shared_ptr<PythonQueueWriter>& queue) {
        WrapperMarketDataClient<std::unique_ptr<Client>>::QueryMarketQuotes(
          query, queue->GetSlot<SequencedMarketQuote>());
      }

      void QueryTimeAndSales(const SecurityMarketDataQuery& query,
          const std::shared_ptr<PythonQueueWriter>& queue) {
        WrapperMarketDataClient<std::unique_ptr<Client>>::QueryTimeAndSales(
          query, queue->GetSlot<TimeAndSale>());
      }

      void QuerySequencedTimeAndSales(const SecurityMarketDataQuery& query,
          const std::shared_ptr<PythonQueueWriter>& queue) {
        WrapperMarketDataClient<std::unique_ptr<Client>>::QueryTimeAndSales(
          query, queue->GetSlot<SequencedTimeAndSale>());
      }
  };

  PythonMarketDataClient* BuildClient(
      VirtualServiceLocatorClient& serviceLocatorClient) {
    auto addresses = LocateServiceAddresses(serviceLocatorClient,
      MarketDataService::RELAY_SERVICE_NAME);
    auto delay = false;
    SessionBuilder sessionBuilder(Ref(serviceLocatorClient),
      [=] () mutable {
        if(delay) {
          LiveTimer delayTimer(seconds(3), Ref(*GetTimerThreadPool()));
          delayTimer.Start();
          delayTimer.Wait();
        }
        delay = true;
        return std::make_unique<TcpSocketChannel>(addresses,
          Ref(*GetSocketThreadPool()));
      },
      [=] {
        return std::make_unique<LiveTimer>(seconds(10),
          Ref(*GetTimerThreadPool()));
      });
    auto baseClient = std::make_unique<Client>(sessionBuilder);
    return new PythonMarketDataClient(std::move(baseClient));
  }

  MarketDataServiceTestInstance* BuildMarketDataServiceTestInstance(
      std::auto_ptr<VirtualServiceLocatorClient> serviceLocatorClient) {
    std::unique_ptr<VirtualServiceLocatorClient> clientWrapper{
      serviceLocatorClient.release()};
    return new MarketDataServiceTestInstance{std::move(clientWrapper)};
  }

  VirtualMarketDataClient* MarketDataServiceTestInstanceBuildClient(
      MarketDataServiceTestInstance& instance,
      VirtualServiceLocatorClient& serviceLocatorClient) {
    return instance.BuildClient(Ref(serviceLocatorClient)).release();
  }
}

void Nexus::Python::ExportMarketDataClient() {
  class_<PythonMarketDataClient, boost::noncopyable>("MarketDataClient",
      no_init)
    .def("__init__", make_constructor(&BuildClient))
    .def("query_order_imbalances",
      &PythonMarketDataClient::QueryOrderImbalances)
    .def("query_sequenced_order_imbalances",
      &PythonMarketDataClient::QuerySequencedOrderImbalances)
    .def("query_bbo_quotes", &PythonMarketDataClient::QueryBboQuotes)
    .def("query_sequenced_bbo_quotes",
      &PythonMarketDataClient::QuerySequencedBboQuotes)
    .def("query_book_quotes", &PythonMarketDataClient::QueryBookQuotes)
    .def("query_sequenced_book_quotes",
      &PythonMarketDataClient::QuerySequencedBookQuotes)
    .def("query_market_quotes", &PythonMarketDataClient::QueryMarketQuotes)
    .def("query_sequenced_market_quotes",
      &PythonMarketDataClient::QuerySequencedMarketQuotes)
    .def("query_time_and_sales", &PythonMarketDataClient::QueryTimeAndSales)
    .def("query_sequenced_time_and_sales",
      &PythonMarketDataClient::QuerySequencedTimeAndSales)
    .def("load_security_snapshot", BlockingFunction<PythonMarketDataClient>(
      &PythonMarketDataClient::LoadSecuritySnapshot))
    .def("load_security_technicals", BlockingFunction<PythonMarketDataClient>(
      &PythonMarketDataClient::LoadSecurityTechnicals))
    .def("load_security_info_from_prefix",
      BlockingFunction<PythonMarketDataClient>(
      &PythonMarketDataClient::LoadSecurityInfoFromPrefix))
    .def("open", BlockingFunction<PythonMarketDataClient>(
      &PythonMarketDataClient::Open))
    .def("close", BlockingFunction<PythonMarketDataClient>(
      &PythonMarketDataClient::Close));
}

void Nexus::Python::ExportMarketDataService() {
  string nestedName = extract<string>(scope().attr("__name__") +
    ".market_data_service");
  object nestedModule{handle<>(
    borrowed(PyImport_AddModule(nestedName.c_str())))};
  scope().attr("market_data_service") = nestedModule;
  scope parent = nestedModule;
  ExportMarketDataClient();
  ExportMarketWideDataQuery();
  ExportSecurityMarketDataQuery();
  {
    string nestedName = extract<string>(parent.attr("__name__") + ".tests");
    object nestedModule{handle<>(
      borrowed(PyImport_AddModule(nestedName.c_str())))};
    parent.attr("tests") = nestedModule;
    scope child = nestedModule;
    ExportMarketDataServiceTestInstance();
  }
}

void Nexus::Python::ExportMarketDataServiceTestInstance() {
  class_<MarketDataServiceTestInstance, boost::noncopyable>(
      "MarketDataServiceTestInstance", no_init)
    .def("__init__", make_constructor(BuildMarketDataServiceTestInstance))
    .def("open", BlockingFunction(&MarketDataServiceTestInstance::Open))
    .def("close", BlockingFunction(&MarketDataServiceTestInstance::Close))
    .def("set_bbo", &MarketDataServiceTestInstance::SetBbo)
    .def("build_client", &MarketDataServiceTestInstanceBuildClient,
      return_value_policy<manage_new_object>());
}

void Nexus::Python::ExportMarketWideDataQuery() {
  ExportIndexedQuery<MarketCode>("MarketWideDataQuery");
  class_<MarketWideDataQuery, bases<IndexedQuery<MarketCode>, RangedQuery,
    SnapshotLimitedQuery, InterruptableQuery, FilteredQuery>>(
    "MarketWideDataQuery", init<>());
  def("query_real_time_with_snapshot",
    static_cast<MarketWideDataQuery (*)(const MarketCode&)>(
    &QueryRealTimeWithSnapshot));
}

void Nexus::Python::ExportSecurityMarketDataQuery() {
  ExportIndexedQuery<Security>("SecurityIndexedQuery");
  class_<SecurityMarketDataQuery, bases<IndexedQuery<Security>, RangedQuery,
    SnapshotLimitedQuery, InterruptableQuery, FilteredQuery>>(
    "SecurityMarketDataQuery", init<>());
  def("query_real_time_with_snapshot",
    static_cast<SecurityMarketDataQuery (*)(Security)>(
    &QueryRealTimeWithSnapshot));
}
