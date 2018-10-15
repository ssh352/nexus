#ifndef NEXUS_MARKET_DATA_WEB_SERVLET_HPP
#define NEXUS_MARKET_DATA_WEB_SERVLET_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/WebServices/HttpRequestSlot.hpp>
#include <Beam/WebServices/SessionStore.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/ServiceClients/ApplicationServiceClients.hpp"
#include "web_portal/web_portal/web_portal.hpp"
#include "web_portal/web_portal/web_portal_session.hpp"

namespace Nexus::WebPortal {

  //! Provides a web interface to the MarketDataService.
  class MarketDataWebServlet : private boost::noncopyable {
    public:

      //! Constructs a MarketDataWebServlet.
      /*!
        \param sessions The available web sessions.
        \param serviceClients The clients used to access Spire services.
      */
      MarketDataWebServlet(Beam::Ref<
        Beam::WebServices::SessionStore<WebPortalSession>> sessions,
        Beam::Ref<ApplicationServiceClients> serviceClients);

      ~MarketDataWebServlet();

      std::vector<Beam::WebServices::HttpRequestSlot> GetSlots();

      void Open();

      void Close();

    private:
      ApplicationServiceClients* m_serviceClients;
      Beam::WebServices::SessionStore<WebPortalSession>* m_sessions;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      Beam::WebServices::HttpResponse OnLoadSecurityInfoFromPrefix(
        const Beam::WebServices::HttpRequest& request);
  };
}

#endif
