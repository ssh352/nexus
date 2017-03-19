#include "ClientWebPortal/ClientWebPortal/DefinitionsWebServlet.hpp"
#include <Beam/WebServices/HttpRequest.hpp>
#include <Beam/WebServices/HttpResponse.hpp>
#include <Beam/WebServices/HttpServerPredicates.hpp>
#include "ClientWebPortal/ClientWebPortal/ClientWebPortalSession.hpp"

using namespace Beam;
using namespace Beam::WebServices;
using namespace boost;
using namespace Nexus;
using namespace Nexus::ClientWebPortal;
using namespace std;

DefinitionsWebServlet::DefinitionsWebServlet(
    RefType<SessionStore<ClientWebPortalSession>> sessions,
    RefType<ApplicationServiceClients> serviceClients)
    : m_sessions{sessions.Get()},
      m_serviceClients{serviceClients.Get()} {}

DefinitionsWebServlet::~DefinitionsWebServlet() {
  Close();
}

vector<HttpRequestSlot> DefinitionsWebServlet::GetSlots() {
  vector<HttpRequestSlot> slots;
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/definitions_service/load_compliance_rule_schemas"),
    std::bind(&DefinitionsWebServlet::OnLoadComplianceRuleSchemas, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/definitions_service/load_country_database"),
    std::bind(&DefinitionsWebServlet::OnLoadCountryDatabase, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/definitions_service/load_currency_database"),
    std::bind(&DefinitionsWebServlet::OnLoadCurrencyDatabase, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/definitions_service/load_market_database"),
    std::bind(&DefinitionsWebServlet::OnLoadMarketDatabase, this,
    std::placeholders::_1));
  return slots;
}

void DefinitionsWebServlet::Open() {
  if(m_openState.SetOpening()) {
    return;
  }
  try {
    m_serviceClients->Open();
  } catch(const std::exception&) {
    m_openState.SetOpenFailure();
    Shutdown();
  }
  m_openState.SetOpen();
}

void DefinitionsWebServlet::Close() {
  if(m_openState.SetClosing()) {
    return;
  }
  Shutdown();
}

void DefinitionsWebServlet::Shutdown() {
  m_openState.SetClosed();
}

HttpResponse DefinitionsWebServlet::OnLoadComplianceRuleSchemas(
    const HttpRequest& request) {
  HttpResponse response;
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto schemas =
    m_serviceClients->GetDefinitionsClient().LoadComplianceRuleSchemas();
  session->ShuttleResponse(schemas, Store(response));
  return response;
}

HttpResponse DefinitionsWebServlet::OnLoadCountryDatabase(
    const HttpRequest& request) {
  HttpResponse response;
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto database =
    m_serviceClients->GetDefinitionsClient().LoadCountryDatabase();
  session->ShuttleResponse(database, Store(response));
  return response;
}

HttpResponse DefinitionsWebServlet::OnLoadCurrencyDatabase(
    const HttpRequest& request) {
  HttpResponse response;
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto database =
    m_serviceClients->GetDefinitionsClient().LoadCurrencyDatabase();
  session->ShuttleResponse(database, Store(response));
  return response;
}

HttpResponse DefinitionsWebServlet::OnLoadMarketDatabase(
    const HttpRequest& request) {
  HttpResponse response;
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto database =
    m_serviceClients->GetDefinitionsClient().LoadMarketDatabase();
  session->ShuttleResponse(database, Store(response));
  return response;
}