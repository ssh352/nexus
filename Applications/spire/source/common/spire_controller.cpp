#include "spire/spire/spire_controller.hpp"
#include <Beam/Network/SocketThreadPool.hpp>
#include <Beam/Threading/TimerThreadPool.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include "Nexus/ServiceClients/ApplicationServiceClients.hpp"
#include "Nexus/ServiceClients/VirtualServiceClients.hpp"
#include <QMessageBox>
#include <QStandardPaths>
#include "spire/login/login_controller.hpp"
#include "spire/toolbar/toolbar_controller.hpp"

using namespace Beam;
using namespace Beam::Network;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::filesystem;
using namespace Nexus;
using namespace spire;
using namespace std;

spire_controller::spire_controller()
    : m_state(state::NONE),
      m_socket_thread_pool(make_unique<SocketThreadPool>()),
      m_timer_thread_pool(make_unique<TimerThreadPool>()) {}

spire_controller::~spire_controller() = default;

void spire_controller::open() {
  if(m_state != state::NONE) {
    return;
  }
  auto ip_address = load_ip_address();
  if(!ip_address.is_initialized()) {
    return;
  }
  auto service_clients_factory =
    [=] (auto&& username, auto&& password) {
      return MakeVirtualServiceClients(make_unique<ApplicationServiceClients>(
        *ip_address, username, password, Ref(*m_socket_thread_pool),
        Ref(*m_timer_thread_pool)));
    };
  m_login_controller = make_unique<login_controller>(service_clients_factory);
  m_login_controller->connect_logged_in_signal([=]{on_login();});
  m_state = state::LOGIN;
  m_login_controller->open();
}

optional<IpAddress> spire_controller::load_ip_address() {
  auto application_path = QStandardPaths::writableLocation(
    QStandardPaths::DataLocation);
  path config_path = application_path.toStdString();
  if(!exists(config_path)) {
    create_directories(config_path);
  }
  config_path /= "config.yml";
  if(!is_regular(config_path)) {
    filesystem::ofstream config_file(config_path);
    config_file <<
      "---\n"
      "address: 127.0.0.1:20000\n"
      "...";
  }
  YAML::Node config;
  try {
    filesystem::ifstream config_stream(config_path);
    if(!config_stream.good()) {
      QMessageBox::critical(nullptr, QObject::tr("Error"),
        QObject::tr("Unable to load configuration: config.yml"));
      return none;
    }
    YAML::Parser configParser(config_stream);
    configParser.GetNextDocument(config);
  } catch(const YAML::ParserException&) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Invalid configuration file."));
    return none;
  }
  IpAddress address;
  try {
    address = Extract<IpAddress>(config, "address");
  } catch(std::exception&) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Invalid configuration file."));
    return none;
  }
  return address;
}

void spire_controller::on_login() {
  m_service_clients = std::move(m_login_controller->get_service_clients());
  m_login_controller.reset();
  m_toolbar_controller = std::make_unique<toolbar_controller>(
    *m_service_clients);
  m_toolbar_controller->open();
  m_state = state::TOOLBAR;
}