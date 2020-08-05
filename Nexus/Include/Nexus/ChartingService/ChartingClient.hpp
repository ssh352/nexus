#ifndef NEXUS_CHARTINGCLIENT_HPP
#define NEXUS_CHARTINGCLIENT_HPP
#include <vector>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/IO/Connection.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queries/SequencedValuePublisher.hpp>
#include <Beam/Queues/ConverterQueueWriter.hpp>
#include <Beam/Routines/RoutineHandlerGroup.hpp>
#include <Beam/Services/ServiceProtocolClientHandler.hpp>
#include <boost/atomic/atomic.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/ChartingService/ChartingService.hpp"
#include "Nexus/ChartingService/ChartingServices.hpp"
#include "Nexus/ChartingService/SecurityChartingQuery.hpp"
#include "Nexus/Queries/ShuttleQueryTypes.hpp"
#include "Nexus/TechnicalAnalysis/CandlestickTypes.hpp"

namespace Nexus {
namespace ChartingService {

  /*! \class ChartingClient
      \brief Client used to access the charting related services.
      \tparam ServiceProtocolClientBuilderType The type used to build
              ServiceProtocolClients to the server.
   */
  template<typename ServiceProtocolClientBuilderType>
  class ChartingClient : private boost::noncopyable {
    public:

      //! The type used to build ServiceProtocolClients to the server.
      using ServiceProtocolClientBuilder = Beam::GetTryDereferenceType<
        ServiceProtocolClientBuilderType>;

      //! Constructs a ChartingClient.
      /*!
        \param clientBuilder Initializes the ServiceProtocolClientBuilder.
      */
      template<typename ClientBuilderForward>
      explicit ChartingClient(ClientBuilderForward&& clientBuilder);

      ~ChartingClient();

      //! Submits a query for a Security's technical info.
      /*!
        \param query The query to submit.
        \param queue The queue that will store the result of the query.
      */
      void QuerySecurity(const SecurityChartingQuery& query,
        Beam::ScopedQueueWriter<Queries::QueryVariant> queue);

      //! Loads a Security's time/price series.
      /*!
        \param security The Security to load the series for.
        \param startTime The series start time (inclusive).
        \param endTime The series end time (inclusive).
        \param interval The time interval per Candlestick.
        \return The Security's time/price series with the specified parameters.
      */
      TimePriceQueryResult LoadTimePriceSeries(const Security& security,
        const boost::posix_time::ptime& startTime,
        const boost::posix_time::ptime& endTime,
        boost::posix_time::time_duration interval);

      void Open();

      void Close();

    private:
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      using SecurityChartingPublisher = Beam::Queries::SequencedValuePublisher<
        SecurityChartingQuery, Queries::QueryVariant>;
      boost::atomic_int m_nextQueryId;
      Beam::SynchronizedUnorderedMap<int,
        std::shared_ptr<SecurityChartingPublisher>>
        m_securityChartingPublishers;
      Beam::Services::ServiceProtocolClientHandler<
        ServiceProtocolClientBuilderType> m_clientHandler;
      Beam::IO::OpenState m_openState;
      Beam::Routines::RoutineHandlerGroup m_queryRoutines;

      void Shutdown();
      void OnReconnect(const std::shared_ptr<ServiceProtocolClient>& client);
      void OnSecurityQuery(ServiceProtocolClient& client, int queryId,
        const Queries::SequencedQueryVariant& value);
  };

  template<typename ServiceProtocolClientBuilderType>
  template<typename ClientBuilderForward>
  ChartingClient<ServiceProtocolClientBuilderType>::ChartingClient(
      ClientBuilderForward&& clientBuilder)
      : m_nextQueryId(0),
        m_clientHandler(std::forward<ClientBuilderForward>(clientBuilder)) {
    m_clientHandler.SetReconnectHandler(
      std::bind(&ChartingClient::OnReconnect, this, std::placeholders::_1));
    Queries::RegisterQueryTypes(Beam::Store(
      m_clientHandler.GetSlots().GetRegistry()));
    RegisterChartingServices(Beam::Store(m_clientHandler.GetSlots()));
    RegisterChartingMessages(Beam::Store(m_clientHandler.GetSlots()));
    Beam::Services::AddMessageSlot<SecurityQueryMessage>(
      Beam::Store(m_clientHandler.GetSlots()), std::bind(
      &ChartingClient::OnSecurityQuery, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3));
  }

  template<typename ServiceProtocolClientBuilderType>
  ChartingClient<ServiceProtocolClientBuilderType>::~ChartingClient() {
    Close();
  }

  template<typename ServiceProtocolClientBuilderType>
  void ChartingClient<ServiceProtocolClientBuilderType>::QuerySecurity(
      const SecurityChartingQuery& query,
      Beam::ScopedQueueWriter<Queries::QueryVariant> queue) {
    if(query.GetRange().GetEnd() == Beam::Queries::Sequence::Last()) {
      m_queryRoutines.Spawn(
        [=, queue = std::move(queue)] {
          auto filter = Beam::Queries::Translate<Queries::EvaluatorTranslator>(
            query.GetFilter());
          auto conversionQueue = Beam::MakeConverterQueueWriter<
            Queries::SequencedQueryVariant>(std::move(queue),
            [] (auto& value) {
              return *value;
            });
          auto publisher = std::make_shared<SecurityChartingPublisher>(query,
            std::move(filter), std::move(conversionQueue));
          auto id = ++m_nextQueryId;
          try {
            publisher->BeginSnapshot();
            m_securityChartingPublishers.Insert(id, publisher);
            auto client = m_clientHandler.GetClient();
            auto queryResult =
              client->template SendRequest<QuerySecurityService>(query, id);
            publisher->PushSnapshot(queryResult.m_snapshot.begin(),
              queryResult.m_snapshot.end());
            publisher->EndSnapshot(queryResult.m_queryId);
          } catch(const std::exception&) {
            publisher->Break(std::current_exception());
            m_securityChartingPublishers.Erase(id);
          }
        });
    } else {
      m_queryRoutines.Spawn(
        [=, queue = std::move(queue)] {
          try {
            auto client = m_clientHandler.GetClient();
            auto id = ++m_nextQueryId;
            auto queryResult =
              client->template SendRequest<QuerySecurityService>(query, id);
            for(auto& value : queryResult.m_snapshot) {
              queue.Push(std::move(value));
            }
            queue.Break();
          } catch(const std::exception&) {
            queue.Break(std::current_exception());
          }
        });
    }
  }

  template<typename ServiceProtocolClientBuilderType>
  TimePriceQueryResult ChartingClient<ServiceProtocolClientBuilderType>::
      LoadTimePriceSeries(const Security& security,
      const boost::posix_time::ptime& startTime,
      const boost::posix_time::ptime& endTime,
      boost::posix_time::time_duration interval) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadSecurityTimePriceSeriesService>(
      security, startTime, endTime, interval);
  }

  template<typename ServiceProtocolClientBuilderType>
  void ChartingClient<ServiceProtocolClientBuilderType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_clientHandler.Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename ServiceProtocolClientBuilderType>
  void ChartingClient<ServiceProtocolClientBuilderType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ServiceProtocolClientBuilderType>
  void ChartingClient<ServiceProtocolClientBuilderType>::Shutdown() {
    m_clientHandler.Close();
    m_openState.SetClosed();
  }

  template<typename ServiceProtocolClientBuilderType>
  void ChartingClient<ServiceProtocolClientBuilderType>::OnReconnect(
      const std::shared_ptr<ServiceProtocolClient>& client) {}

  template<typename ServiceProtocolClientBuilderType>
  void ChartingClient<ServiceProtocolClientBuilderType>::OnSecurityQuery(
      ServiceProtocolClient& client, int queryId,
      const Queries::SequencedQueryVariant& value) {
    auto checkPublisher = m_securityChartingPublishers.FindValue(queryId);
    if(!checkPublisher.is_initialized()) {
      return;
    }
    const auto& publisher = *checkPublisher;
    try {
      publisher->Push(value);
    } catch(const std::exception&) {
      if(publisher->GetId() != -1) {
        auto client = m_clientHandler.GetClient();
        Beam::Services::SendRecordMessage<EndSecurityQueryMessage>(*client,
          queryId);
      }
    }
  }
}
}

#endif
