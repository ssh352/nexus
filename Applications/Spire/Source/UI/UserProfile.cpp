#include "Spire/UI/UserProfile.hpp"
#include <QStandardPaths>
#include "Nexus/ServiceClients/VirtualServiceClients.hpp"
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Blotter/BlotterSettings.hpp"
#include "Spire/Blotter/OpenPositionsModel.hpp"
#include "Spire/UI/WindowSettings.hpp"

using namespace Beam;
using namespace Beam::Routines;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::local_time;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Spire;
using namespace Spire::UI;
using namespace std;
using namespace std::filesystem;

UserProfile::UserProfile(const string& username, bool isAdministrator,
    bool isManager, const CountryDatabase& countryDatabase,
    const tz_database& timeZoneDatabase,
    const CurrencyDatabase& currencyDatabase,
    const vector<ExchangeRate>& exchangeRates,
    const MarketDatabase& marketDatabase,
    const DestinationDatabase& destinationDatabase,
    const EntitlementDatabase& entitlementDatabase,
    Ref<TimerThreadPool> timerThreadPool,
    Ref<VirtualServiceClients> serviceClients)
    : m_username(username),
      m_isAdministrator(isAdministrator),
      m_isManager(isManager),
      m_countryDatabase(countryDatabase),
      m_timeZoneDatabase(timeZoneDatabase),
      m_currencyDatabase(currencyDatabase),
      m_marketDatabase(marketDatabase),
      m_destinationDatabase(destinationDatabase),
      m_entitlementDatabase(entitlementDatabase),
      m_timerThreadPool(timerThreadPool.Get()),
      m_serviceClients(serviceClients.Get()),
      m_profilePath(path(QStandardPaths::writableLocation(
        QStandardPaths::DataLocation).toStdString()) / "Profiles" / m_username),
      m_catalogSettings(m_profilePath / "Catalog", isAdministrator),
      m_interactionProperties(InteractionsProperties::GetDefaultProperties()) {
  for(auto& exchangeRate : exchangeRates) {
    m_exchangeRates.Add(exchangeRate);
  }
  m_blotterSettings = std::make_unique<BlotterSettings>(Ref(*this));
}

UserProfile::~UserProfile() = default;

const string& UserProfile::GetUsername() const {
  return m_username;
}

bool UserProfile::IsAdministrator() const {
  return m_isAdministrator;
}

bool UserProfile::IsManager() const {
  return m_isAdministrator || m_isManager;
}

const CountryDatabase& UserProfile::GetCountryDatabase() const {
  return m_countryDatabase;
}

const tz_database& UserProfile::GetTimeZoneDatabase() const {
  return m_timeZoneDatabase;
}

const CurrencyDatabase& UserProfile::GetCurrencyDatabase() const {
  return m_currencyDatabase;
}

const ExchangeRateTable& UserProfile::GetExchangeRates() const {
  return m_exchangeRates;
}

const MarketDatabase& UserProfile::GetMarketDatabase() const {
  return m_marketDatabase;
}

const DestinationDatabase& UserProfile::GetDestinationDatabase() const {
  return m_destinationDatabase;
}

const EntitlementDatabase& UserProfile::GetEntitlementDatabase() const {
  return m_entitlementDatabase;
}

TimerThreadPool& UserProfile::GetTimerThreadPool() {
  return *m_timerThreadPool;
}

VirtualServiceClients& UserProfile::GetServiceClients() const {
  return *m_serviceClients;
}

void UserProfile::CreateProfilePath() const {
  if(is_directory(m_profilePath)) {
    return;
  }
  if(exists(m_profilePath)) {
    remove(m_profilePath);
  }
  create_directories(m_profilePath);
}

const path& UserProfile::GetProfilePath() const {
  return m_profilePath;
}

const vector<unique_ptr<WindowSettings>>&
    UserProfile::GetRecentlyClosedWindows() const {
  return m_recentlyClosedWindows;
}

void UserProfile::AddRecentlyClosedWindow(unique_ptr<WindowSettings> window) {
  m_recentlyClosedWindows.push_back(std::move(window));
}

void UserProfile::RemoveRecentlyClosedWindow(const WindowSettings& window) {
  for(auto i = m_recentlyClosedWindows.begin();
      i != m_recentlyClosedWindows.end(); ++i) {
    if(i->get() == &window) {
      m_recentlyClosedWindows.erase(i);
      return;
    }
  }
}

const BlotterSettings& UserProfile::GetBlotterSettings() const {
  return *m_blotterSettings;
}

BlotterSettings& UserProfile::GetBlotterSettings() {
  return *m_blotterSettings;
}

SavedDashboards& UserProfile::GetSavedDashboards() {
  return m_savedDashboards;
}

const SavedDashboards& UserProfile::GetSavedDashboards() const {
  return m_savedDashboards;
}

const KeyBindings& UserProfile::GetKeyBindings() const {
  return m_keyBindings;
}

KeyBindings& UserProfile::GetKeyBindings() {
  return m_keyBindings;
}

void UserProfile::SetKeyBindings(const KeyBindings& keyBindings) {
  m_keyBindings = keyBindings;
}

const CatalogSettings& UserProfile::GetCatalogSettings() const {
  return m_catalogSettings;
}

CatalogSettings& UserProfile::GetCatalogSettings() {
  return m_catalogSettings;
}

const CanvasTypeRegistry& UserProfile::GetCanvasTypeRegistry() const {
  return m_typeRegistry;
}

CanvasTypeRegistry& UserProfile::GetCanvasTypeRegistry() {
  return m_typeRegistry;
}

const BookViewProperties& UserProfile::GetDefaultBookViewProperties() const {
  return m_defaultBookViewProperties;
}

void UserProfile::SetDefaultBookViewProperties(
    const BookViewProperties& properties) {
  m_defaultBookViewProperties = properties;
}

const OrderImbalanceIndicatorProperties& UserProfile::
    GetDefaultOrderImbalanceIndicatorProperties() const {
  return m_defaultOrderImbalanceIndicatorProperties;
}

void UserProfile::SetDefaultOrderImbalanceIndicatorProperties(
    const OrderImbalanceIndicatorProperties& properties) {
  m_defaultOrderImbalanceIndicatorProperties = properties;
}

const boost::optional<OrderImbalanceIndicatorWindowSettings>& UserProfile::
    GetInitialOrderImbalanceIndicatorWindowSettings() const {
  return m_initialOrderImbalanceIndicatorWindowSettings;
}

void UserProfile::SetInitialOrderImbalanceIndicatorWindowSettings(
    const OrderImbalanceIndicatorWindowSettings& settings) {
  m_initialOrderImbalanceIndicatorWindowSettings = settings;
}

const RiskTimerProperties& UserProfile::GetRiskTimerProperties() const {
  return m_riskTimerProperties;
}

RiskTimerProperties& UserProfile::GetRiskTimerProperties() {
  return m_riskTimerProperties;
}

const TimeAndSalesProperties& UserProfile::
    GetDefaultTimeAndSalesProperties() const {
  return m_defaultTimeAndSalesProperties;
}

void UserProfile::SetDefaultTimeAndSalesProperties(
    const TimeAndSalesProperties& properties) {
  m_defaultTimeAndSalesProperties = properties;
}

const PortfolioViewerProperties& UserProfile::
    GetDefaultPortfolioViewerProperties() const {
  return m_defaultPortfolioViewerProperties;
}

void UserProfile::SetDefaultPortfolioViewerProperties(
    const PortfolioViewerProperties& properties) {
  m_defaultPortfolioViewerProperties = properties;
}

RegionMap<InteractionsProperties>& UserProfile::GetInteractionProperties() {
  return m_interactionProperties;
}

const RegionMap<InteractionsProperties>&
    UserProfile::GetInteractionProperties() const {
  return m_interactionProperties;
}

Quantity UserProfile::GetDefaultQuantity(const Security& security,
    Side side) const {
  auto baseQuantity = GetInteractionProperties().Get(
    security).m_defaultQuantity;
  if(baseQuantity <= 0) {
    return 0;
  }
  auto& activeBlotter = GetBlotterSettings().GetActiveBlotter();
  auto position = activeBlotter.GetOpenPositionsModel().GetOpenPosition(
    security);
  Quantity defaultQuantity;
  Quantity currentQuantity;
  if(!position.is_initialized()) {
    currentQuantity = 0;
  } else {
    currentQuantity = position->m_inventory.m_position.m_quantity;
  }
  if(side == Side::BID && currentQuantity < 0 ||
      side == Side::ASK && currentQuantity > 0) {
    defaultQuantity = std::min(baseQuantity, Abs(currentQuantity));
  } else {
    defaultQuantity = baseQuantity - Abs(currentQuantity) % baseQuantity;
  }
  return defaultQuantity;
}

const boost::optional<PortfolioViewerWindowSettings>&
    UserProfile::GetInitialPortfolioViewerWindowSettings() const {
  return m_initialPortfolioViewerWindowSettings;
}

void UserProfile::SetInitialPortfolioViewerWindowSettings(
    const PortfolioViewerWindowSettings& settings) {
  m_initialPortfolioViewerWindowSettings = settings;
}
