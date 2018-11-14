#ifndef NEXUS_SQL_HISTORICAL_DATA_STORE_HPP
#define NEXUS_SQL_HISTORICAL_DATA_STORE_HPP
#include <functional>
#include <thread>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queries/SqlDataStore.hpp>
#include <Beam/Sql/DatabaseConnectionPool.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/Threading/ThreadPool.hpp>
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/MarketDataService/HistoricalDataStore.hpp"
#include "Nexus/MarketDataService/HistoricalDataStoreException.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/SqlDefinitions.hpp"
#include "Nexus/Queries/SqlTranslator.hpp"

namespace Nexus::MarketDataService {

  /** Stores historical market data in an SQL database.
      \tparam C The type of SQL connection.
   */
  template<typename C>
  class SqlHistoricalDataStore : private boost::noncopyable {
    public:

      //! The type of SQL connection.
      using Connection = C;

      //! The callable used to build SQL connections.
      using ConnectionBuilder = std::function<Connection ()>;

      //! Constructs an SqlHistoricalDataStore.
      /*!
        \param connectionBuilder The callable used to build SQL connections.
      */
      SqlHistoricalDataStore(ConnectionBuilder connectionBuilder);

      ~SqlHistoricalDataStore();

      std::vector<SequencedOrderImbalance> LoadOrderImbalances(
        const MarketWideDataQuery& query);

      std::vector<SequencedBboQuote> LoadBboQuotes(
        const SecurityMarketDataQuery& query);

      std::vector<SequencedBookQuote> LoadBookQuotes(
        const SecurityMarketDataQuery& query);

      std::vector<SequencedMarketQuote> LoadMarketQuotes(
        const SecurityMarketDataQuery& query);

      std::vector<SequencedTimeAndSale> LoadTimeAndSales(
        const SecurityMarketDataQuery& query);

      void Store(const SequencedMarketOrderImbalance& orderImbalance);

      void Store(const std::vector<SequencedMarketOrderImbalance>&
        orderImbalances);

      void Store(const SequencedSecurityBboQuote& bboQuote);

      void Store(const std::vector<SequencedSecurityBboQuote>& bboQuotes);

      void Store(const SequencedSecurityMarketQuote& marketQuote);

      void Store(const std::vector<SequencedSecurityMarketQuote>& marketQuotes);

      void Store(const SequencedSecurityBookQuote& bookQuote);

      void Store(const std::vector<SequencedSecurityBookQuote>& bookQuotes);

      void Store(const SequencedSecurityTimeAndSale& timeAndSale);

      void Store(const std::vector<SequencedSecurityTimeAndSale>& timeAndSales);

      void Open();

      void Close();

    private:
      template<typename V, typename I>
      using DataStore = Beam::Queries::SqlDataStore<Connection, V, I,
        Queries::SqlTranslator>;
      ConnectionBuilder m_connectionBuilder;
      Beam::DatabaseConnectionPool<Connection> m_readerPool;
      Beam::DatabaseConnectionPool<Connection> m_writerPool;
      Beam::Threading::ThreadPool m_threadPool;
      DataStore<Viper::Row<OrderImbalance>, Viper::Row<MarketCode>>
        m_orderImbalanceDataStore;
      DataStore<Viper::Row<BboQuote>, Viper::Row<Security>> m_bboQuoteDataStore;
      DataStore<Viper::Row<MarketQuote>, Viper::Row<Security>>
        m_marketQuoteDataStore;
      DataStore<Viper::Row<BookQuote>, Viper::Row<Security>>
        m_bookQuoteDataStore;
      DataStore<Viper::Row<TimeAndSale>, Viper::Row<Security>>
        m_timeAndSaleDataStore;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename C>
  SqlHistoricalDataStore<C>::SqlHistoricalDataStore(
      ConnectionBuilder connectionBuilder)
      : m_connectionBuilder(std::move(connectionBuilder)),
        m_orderImbalanceDataStore("order_imbalances", GetOrderImbalanceRow(),
          GetMarketCodeRow(), Beam::Ref(m_readerPool), Beam::Ref(m_writerPool),
          Beam::Ref(m_threadPool)),
        m_bboQuoteDataStore("bbo_quotes", GetBboQuoteRow(), GetSecurityRow(),
          Beam::Ref(m_readerPool), Beam::Ref(m_writerPool),
          Beam::Ref(m_threadPool)),
        m_marketQuoteDataStore("market_quotes", GetMarketQuoteRow(),
          GetSecurityRow(), Beam::Ref(m_readerPool), Beam::Ref(m_writerPool),
          Beam::Ref(m_threadPool)),
        m_bookQuoteDataStore("book_quotes", GetBookQuoteRow(), GetSecurityRow(),
          Beam::Ref(m_readerPool), Beam::Ref(m_writerPool),
          Beam::Ref(m_threadPool)),
        m_timeAndSaleDataStore("time_and_sales", GetTimeAndSaleRow(),
          GetSecurityRow(), Beam::Ref(m_readerPool), Beam::Ref(m_writerPool),
          Beam::Ref(m_threadPool)) {}

  template<typename C>
  SqlHistoricalDataStore<C>::~SqlHistoricalDataStore() {
    Close();
  }

  template<typename C>
  std::vector<SequencedOrderImbalance> SqlHistoricalDataStore<C>::
      LoadOrderImbalances(const MarketWideDataQuery& query) {
    return m_orderImbalanceDataStore.Load(query);
  }

  template<typename C>
  std::vector<SequencedBboQuote> SqlHistoricalDataStore<C>::LoadBboQuotes(
      const SecurityMarketDataQuery& query) {
    return m_bboQuoteDataStore.Load(query);
  }

  template<typename C>
  std::vector<SequencedBookQuote> SqlHistoricalDataStore<C>::LoadBookQuotes(
      const SecurityMarketDataQuery& query) {
    return m_bookQuoteDataStore.Load(query);
  }

  template<typename C>
  std::vector<SequencedMarketQuote> SqlHistoricalDataStore<C>::LoadMarketQuotes(
      const SecurityMarketDataQuery& query) {
    return m_marketQuoteDataStore.Load(query);
  }

  template<typename C>
  std::vector<SequencedTimeAndSale> SqlHistoricalDataStore<C>::LoadTimeAndSales(
      const SecurityMarketDataQuery& query) {
    return m_timeAndSaleDataStore.Load(query);
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::Store(
      const SequencedMarketOrderImbalance& orderImbalance) {
    return m_orderImbalanceDataStore.Store(orderImbalance);
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::Store(
      const std::vector<SequencedMarketOrderImbalance>& orderImbalances) {
    return m_orderImbalanceDataStore.Store(orderImbalances);
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::Store(
      const SequencedSecurityBboQuote& bboQuote) {
    return m_bboQuoteDataStore.Store(bboQuote);
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::Store(
      const std::vector<SequencedSecurityBboQuote>& bboQuotes) {
    return m_bboQuoteDataStore.Store(bboQuotes);
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::Store(
      const SequencedSecurityMarketQuote& marketQuote) {
    return m_marketQuoteDataStore.Store(marketQuote);
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::Store(
      const std::vector<SequencedSecurityMarketQuote>& marketQuotes) {
    return m_marketQuoteDataStore.Store(marketQuotes);
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::Store(
      const SequencedSecurityBookQuote& bookQuote) {
    return m_bookQuoteDataStore.Store(bookQuote);
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::Store(
      const std::vector<SequencedSecurityBookQuote>& bookQuotes) {
    return m_bookQuoteDataStore.Store(bookQuotes);
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::Store(
      const SequencedSecurityTimeAndSale& timeAndSale) {
    return m_timeAndSaleDataStore.Store(timeAndSale);
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::Store(
      const std::vector<SequencedSecurityTimeAndSale>& timeAndSales) {
    return m_timeAndSaleDataStore.Store(timeAndSales);
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      for(auto i = std::size_t(0);
          i <= std::thread::hardware_concurrency(); ++i) {
        auto readerConnection =
          std::make_unique<Connection>(m_connectionBuilder());
        readerConnection->open();
        m_readerPool.Add(std::move(readerConnection));
      }
      auto writerConnection =
        std::make_unique<Connection>(m_connectionBuilder());
      writerConnection->open();
      m_writerPool.Add(std::move(writerConnection));
      m_orderImbalanceDataStore.Open();
      m_bboQuoteDataStore.Open();
      m_marketQuoteDataStore.Open();
      m_bookQuoteDataStore.Open();
      m_timeAndSaleDataStore.Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::Shutdown() {
    m_timeAndSaleDataStore.Close();
    m_bookQuoteDataStore.Close();
    m_marketQuoteDataStore.Close();
    m_bboQuoteDataStore.Close();
    m_orderImbalanceDataStore.Close();
    m_writerPool.Close();
    m_readerPool.Close();
    m_openState.SetClosed();
  }
}

#endif