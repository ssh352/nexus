#ifndef NEXUS_ADMINISTRATIONSERVICE_HPP
#define NEXUS_ADMINISTRATIONSERVICE_HPP
#include <string>

namespace Nexus {
namespace AdministrationService {
  struct AccountIdentity;
  class AccountModificationRequest;
  template<typename ServiceProtocolClientBuilderType>
    class AdministrationClient;
  class AdministrationDataStore;
  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationDataStoreType> class AdministrationServlet;
  class AdministrationSession;
  class ApplicationAdministrationClient;
  template<typename DataStoreType> class CachedAdministrationDataStore;
  class EntitlementModification;
  class LocalAdministrationDataStore;
  class Message;
  class MySqlAdministrationDataStore;
  class TradingGroup;
  class VirtualAdministrationClient;
  template<typename ClientType> class WrapperAdministrationClient;

  // Standard name for the administration service.
  static const std::string SERVICE_NAME = "administration_service";
}
}

#endif
