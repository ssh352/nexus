#ifndef NEXUS_CACHEDADMINISTRATIONDATASTORE_HPP
#define NEXUS_CACHEDADMINISTRATIONDATASTORE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include "Nexus/AdministrationService/AdministrationDataStore.hpp"
#include "Nexus/AdministrationService/AdministrationService.hpp"
#include "Nexus/AdministrationService/LocalAdministrationDataStore.hpp"

namespace Nexus {
namespace AdministrationService {

  /*! \class CachedAdministrationDataStore
      \brief Caches an AdministratorDataStore in memory.
      \tparam DataStoreType The type of AdministrationDataStore to cache.
   */
  template<typename DataStoreType>
  class CachedAdministrationDataStore : public AdministrationDataStore {
    public:

      //! The type of ComplianceRuleDataStore to cache.
      using DataStore = Beam::GetTryDereferenceType<DataStoreType>;

      //! Constructs a CachedAdministrationDataStore.
      /*!
        \param dataStore The AdministrationDataStore to cache.
      */
      template<typename DataStoreForward>
      CachedAdministrationDataStore(DataStoreForward&& dataStore);

      virtual ~CachedAdministrationDataStore() override;

      virtual std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
        AccountIdentity>> LoadAllAccountIdentities() override;

      virtual AccountIdentity LoadIdentity(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      virtual void Store(const Beam::ServiceLocator::DirectoryEntry& account,
        const AccountIdentity& identity) override;

      virtual std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
        RiskService::RiskParameters>> LoadAllRiskParameters() override;

      virtual RiskService::RiskParameters LoadRiskParameters(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      virtual void Store(const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskParameters& riskParameters) override;

      virtual std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
        RiskService::RiskState>> LoadAllRiskStates() override;

      virtual RiskService::RiskState LoadRiskState(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      virtual void Store(const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskState& riskState) override;

      virtual void WithTransaction(
        const std::function<void ()>& transaction) override;

      virtual void Open() override;

      virtual void Close() override;

    private:
      Beam::GetOptionalLocalPtr<DataStoreType> m_dataStore;
      LocalAdministrationDataStore m_cache;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename DataStoreType>
  template<typename DataStoreForward>
  CachedAdministrationDataStore<DataStoreType>::CachedAdministrationDataStore(
      DataStoreForward&& dataStore)
      : m_dataStore{std::forward<DataStoreForward>(dataStore)} {}

  template<typename DataStoreType>
  CachedAdministrationDataStore<DataStoreType>::
      ~CachedAdministrationDataStore() {
    Close();
  }

  template<typename DataStoreType>
  std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry, AccountIdentity>>
      CachedAdministrationDataStore<DataStoreType>::LoadAllAccountIdentities() {
    return m_cache.LoadAllAccountIdentities();
  }

  template<typename DataStoreType>
  AccountIdentity CachedAdministrationDataStore<DataStoreType>::LoadIdentity(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_cache.LoadIdentity(account);
  }

  template<typename DataStoreType>
  void CachedAdministrationDataStore<DataStoreType>::Store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const AccountIdentity& identity) {
    m_dataStore->Store(account, identity);
    m_cache.Store(account, identity);
  }

  template<typename DataStoreType>
  std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
      RiskService::RiskParameters>>
      CachedAdministrationDataStore<DataStoreType>::LoadAllRiskParameters() {
    return m_cache.LoadAllRiskParameters();
  }

  template<typename DataStoreType>
  RiskService::RiskParameters CachedAdministrationDataStore<DataStoreType>::
      LoadRiskParameters(const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_cache.LoadRiskParameters(account);
  }

  template<typename DataStoreType>
  void CachedAdministrationDataStore<DataStoreType>::Store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskParameters& riskParameters) {
    m_dataStore->Store(account, riskParameters);
    m_cache.Store(account, riskParameters);
  }

  template<typename DataStoreType>
  std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
      RiskService::RiskState>> CachedAdministrationDataStore<DataStoreType>::
      LoadAllRiskStates() {
    return m_cache.LoadAllRiskStates();
  }

  template<typename DataStoreType>
  RiskService::RiskState CachedAdministrationDataStore<DataStoreType>::
      LoadRiskState(const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_cache.LoadRiskState(account);
  }

  template<typename DataStoreType>
  void CachedAdministrationDataStore<DataStoreType>::Store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskState& riskState) {
    m_dataStore->Store(account, riskState);
    m_cache.Store(account, riskState);
  }

  template<typename DataStoreType>
  void CachedAdministrationDataStore<DataStoreType>::WithTransaction(
      const std::function<void ()>& transaction) {
    m_dataStore->WithTransaction(
      [&] {
        m_cache.WithTransaction(transaction);
      });
  }

  template<typename DataStoreType>
  void CachedAdministrationDataStore<DataStoreType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_dataStore->Open();
      m_cache.Open();
      auto identities = m_dataStore->LoadAllAccountIdentities();
      for(auto& identity : identities) {
        m_cache.Store(std::get<0>(identity), std::get<1>(identity));
      }
      auto riskParameters = m_dataStore->LoadAllRiskParameters();
      for(auto& riskParameter : riskParameters) {
        m_cache.Store(std::get<0>(riskParameter), std::get<1>(riskParameter));
      }
      auto riskStates = m_dataStore->LoadAllRiskStates();
      for(auto& riskState : riskStates) {
        m_cache.Store(std::get<0>(riskState), std::get<1>(riskState));
      }
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename DataStoreType>
  void CachedAdministrationDataStore<DataStoreType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename DataStoreType>
  void CachedAdministrationDataStore<DataStoreType>::Shutdown() {
    m_dataStore->Close();
    m_cache.Close();
    m_openState.SetClosed();
  }
}
}

#endif