#include "Spire/Dashboard/SavedDashboards.hpp"
#include <filesystem>
#include <fstream>
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/IO/BasicOStreamWriter.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <QMessageBox>
#include "Spire/UI/UISerialization.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::UI;
using namespace std;
using namespace std::filesystem;

SavedDashboards::Entry::Entry(string name, DashboardModelSchema schema,
    std::shared_ptr<WindowSettings> settings)
    : m_name(std::move(name)),
      m_schema{std::move(schema)},
      m_settings(std::move(settings)) {}

void SavedDashboards::Load(Out<UserProfile> userProfile) {
  path filePath = userProfile->GetProfilePath() / "dashboards.dat";
  SavedDashboards savedDashboards;
  if(!exists(filePath)) {
    userProfile->GetSavedDashboards() = savedDashboards;
    return;
  }
  try {
    BasicIStreamReader<ifstream> reader(Initialize(filePath, ios::binary));
    SharedBuffer buffer;
    reader.Read(Store(buffer));
    TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
    RegisterSpireTypes(Store(typeRegistry));
    auto receiver = BinaryReceiver<SharedBuffer>(Ref(typeRegistry));
    receiver.SetSource(Ref(buffer));
    receiver.Shuttle(savedDashboards);
  } catch(std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to load dashboards, using defaults."));
    savedDashboards = SavedDashboards{};
  }
  userProfile->GetSavedDashboards() = savedDashboards;
}

void SavedDashboards::Save(const UserProfile& userProfile) {
  path filePath = userProfile.GetProfilePath() / "dashboards.dat";
  try {
    TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
    RegisterSpireTypes(Store(typeRegistry));
    auto sender = BinarySender<SharedBuffer>(Ref(typeRegistry));
    SharedBuffer buffer;
    sender.SetSink(Ref(buffer));
    sender.Shuttle(userProfile.GetSavedDashboards());
    BasicOStreamWriter<ofstream> writer(Initialize(filePath, ios::binary));
    writer.Write(buffer);
  } catch(std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to save dashboards."));
  }
}

SavedDashboards::SavedDashboards(const SavedDashboards& savedDashboards)
    : m_dashboards(savedDashboards.m_dashboards) {}

SavedDashboards& SavedDashboards::operator =(
    const SavedDashboards& savedDashboards) {
  while(!m_dashboards.empty()) {
    Delete(m_dashboards.back().m_name);
  }
  for(auto& savedDashboard : savedDashboards.GetDashboards()) {
    Save(savedDashboard);
  }
  return *this;
}

const vector<SavedDashboards::Entry>& SavedDashboards::GetDashboards() const {
  return m_dashboards;
}

void SavedDashboards::Save(const Entry& entry) {
  auto i = find_if(m_dashboards.begin(), m_dashboards.end(),
    [&] (const Entry& dashboard) {
      return dashboard.m_name == entry.m_name;
    });
  if(i == m_dashboards.end()) {
    m_dashboards.push_back(entry);
    m_dashboardAddedSignal(m_dashboards.back());
    return;
  }
  *i = entry;
}

void SavedDashboards::Delete(const string& name) {
  auto i = find_if(m_dashboards.begin(), m_dashboards.end(),
    [&] (const Entry& entry) {
      return entry.m_name == name;
    });
  if(i == m_dashboards.end()) {
    return;
  }
  auto entry = *i;
  m_dashboards.erase(i);
  m_dashboardRemovedSignal(entry);
}

connection SavedDashboards::ConnectDashboardAddedSignal(
    const DashboardAddedSignal::slot_type& slot) const {
  return m_dashboardAddedSignal.connect(slot);
}

connection SavedDashboards::ConnectDashboardRemovedSignal(
    const DashboardRemovedSignal::slot_type& slot) const {
  return m_dashboardRemovedSignal.connect(slot);
}
