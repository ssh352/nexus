#ifndef NEXUS_ACCOUNTORDERSUBMISSIONENTRY_HPP
#define NEXUS_ACCOUNTORDERSUBMISSIONENTRY_HPP
#include <Beam/Queries/Sequence.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <boost/atomic/atomic.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus {
namespace OrderExecutionService {

  /*! \class AccountOrderSubmissionEntry
      \brief Keeps and updates a registry of Order submissions.
   */
  class AccountOrderSubmissionEntry : private boost::noncopyable {
    public:

      /*! \struct InitialSequences
          \brief Stores the next Sequence to use.
       */
      struct InitialSequences {

        //! The next Sequence to use for an OrderInfo.
        Beam::Queries::Sequence m_nextOrderInfoSequence;

        //! The next Sequence to use for an ExecutionReport.
        Beam::Queries::Sequence m_nextExecutionReportSequence;
      };

      //! Constructs an AccountOrderSubmissionEntry.
      /*!
        \param account The account.
        \param initialSequences The initial Sequences to use.
      */
      AccountOrderSubmissionEntry(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const InitialSequences& initialSequences);

      //! Publishes an OrderInfo.
      /*!
        \param orderInfo The OrderInfo to publish.
      */
      SequencedAccountOrderInfo Publish(const OrderInfo& orderInfo);

      //! Publishes an ExecutionReport.
      /*!
        \param executionReport The ExecutionReport to publish.
      */
      SequencedAccountExecutionReport Publish(
        const ExecutionReport& executionReport);

    private:
      Beam::ServiceLocator::DirectoryEntry m_account;
      boost::atomic<Beam::Queries::Sequence::Ordinal> m_orderSequence;
      boost::atomic<Beam::Queries::Sequence::Ordinal> m_executionReportSequence;
  };

  //! Returns the InitialSequences for a AccountOrderSubmissionEntry.
  /*!
    \param dataStore The DataStore to load the InitialSequences from.
    \param account The account to load the InitialSequences for.
    \return The set of InitialSequences for the specified <i>account</i>.
  */
  template<typename DataStore>
  AccountOrderSubmissionEntry::InitialSequences LoadInitialSequences(
      DataStore& dataStore,
      const Beam::ServiceLocator::DirectoryEntry& account) {
    AccountQuery query;
    query.SetIndex(account);
    query.SetRange(Beam::Queries::Range::Total());
    query.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Type::TAIL, 1);
    AccountOrderSubmissionEntry::InitialSequences initialSequences;
    {
      auto results = dataStore.LoadOrderSubmissions(query);
      if(results.empty()) {
        initialSequences.m_nextOrderInfoSequence =
          Beam::Queries::Sequence::First();
      } else {
        initialSequences.m_nextOrderInfoSequence =
          Beam::Queries::Increment(results.back().GetSequence());
      }
    }
    {
      auto results = dataStore.LoadExecutionReports(query);
      if(results.empty()) {
        initialSequences.m_nextExecutionReportSequence =
          Beam::Queries::Sequence::First();
      } else {
        initialSequences.m_nextExecutionReportSequence =
          Beam::Queries::Increment(results.back().GetSequence());
      }
    }
    return initialSequences;
  }

  inline AccountOrderSubmissionEntry::AccountOrderSubmissionEntry(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const InitialSequences& initialSequences)
      : m_account(account),
        m_orderSequence(
          initialSequences.m_nextOrderInfoSequence.GetOrdinal()),
        m_executionReportSequence(
          initialSequences.m_nextExecutionReportSequence.GetOrdinal()) {}

  inline SequencedAccountOrderInfo AccountOrderSubmissionEntry::Publish(
      const OrderInfo& orderInfo) {
    auto sequence = ++m_orderSequence;
    auto sequencedOrderInfo = Beam::Queries::SequencedValue(
      Beam::Queries::IndexedValue(orderInfo, m_account),
      Beam::Queries::Sequence(sequence));
    return sequencedOrderInfo;
  }

  inline SequencedAccountExecutionReport AccountOrderSubmissionEntry::Publish(
      const ExecutionReport& executionReport) {
    auto sequence = ++m_executionReportSequence;
    auto sequencedExecutionReport = Beam::Queries::SequencedValue(
      Beam::Queries::IndexedValue(executionReport, m_account),
      Beam::Queries::Sequence(sequence));
    return sequencedExecutionReport;
  }
}
}

#endif
