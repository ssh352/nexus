#include "Spire/Blotter/BlotterWindow.hpp"
#include <unordered_map>
#include <QAbstractItemModel>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QMenu>
#include <QShortcut>
#include <QStatusBar>
#include <QToolBar>
#include "Spire/Blotter/ActivityLogModel.hpp"
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Blotter/BlotterMonitorsDialog.hpp"
#include "Spire/Blotter/BlotterTaskEntryItemDelegate.hpp"
#include "Spire/Blotter/BlotterTasksModel.hpp"
#include "Spire/Blotter/BlotterSettings.hpp"
#include "Spire/Blotter/BlotterWindowSettings.hpp"
#include "Spire/Spire/UserProfile.hpp"
#include "Spire/UI/ValueLabel.hpp"
#include "ui_BlotterWindow.h"

using namespace Beam;
using namespace Beam::Tasks;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

namespace {
  const auto UPDATE_INTERVAL = 100;
  std::unordered_map<BlotterModel*, BlotterWindow*> blotterWindows;

  class LinkBlotterAction : public QAction {
    public:
      LinkBlotterAction(Ref<BlotterModel> blotterModel, QObject* parent);

      virtual ~LinkBlotterAction();

      BlotterModel& GetBlotterModel();

    private:
      BlotterModel* m_blotterModel;
  };
}

LinkBlotterAction::LinkBlotterAction(Ref<BlotterModel> blotterModel,
    QObject* parent)
    : QAction(parent),
      m_blotterModel(blotterModel.Get()) {}

LinkBlotterAction::~LinkBlotterAction() {}

BlotterModel& LinkBlotterAction::GetBlotterModel() {
  return *m_blotterModel;
}

BlotterWindow& BlotterWindow::GetBlotterWindow(Ref<UserProfile> userProfile,
    Ref<BlotterModel> model) {
  return GetBlotterWindow(Ref(userProfile), Ref(model), nullptr, 0);
}

BlotterWindow& BlotterWindow::GetBlotterWindow(Ref<UserProfile> userProfile,
    Ref<BlotterModel> model, QWidget* parent, Qt::WindowFlags flags) {
  auto windowIterator = blotterWindows.find(model.Get());
  if(windowIterator == blotterWindows.end()) {
    auto window = new BlotterWindow(userProfile.Get(), model.Get(), parent,
      flags);
    window->setAttribute(Qt::WA_DeleteOnClose);
    windowIterator = blotterWindows.insert(
      make_pair(model.Get(), window)).first;
  }
  return *windowIterator->second;
}

BlotterWindow::~BlotterWindow() {}

const BlotterModel& BlotterWindow::GetModel() const {
  return *m_model;
}

BlotterModel& BlotterWindow::GetModel() {
  return *m_model;
}

unique_ptr<WindowSettings> BlotterWindow::GetWindowSettings() const {
  return std::make_unique<BlotterWindowSettings>(*this);
}

BlotterWindow::BlotterWindow(UserProfile* userProfile, BlotterModel* model,
    QWidget* parent, Qt::WindowFlags flags)
    : QFrame(parent, flags),
      m_ui(std::make_unique<Ui_BlotterWindow>()),
      m_userProfile(userProfile),
      m_model(model) {
  m_ui->setupUi(this);
  setWindowTitle(QString::fromStdString(m_model->GetName()) + tr(" - Blotter"));
  m_ui->m_openPositionsTab->SetModel(Ref(*m_userProfile), Ref(*m_model));
  m_ui->m_profitAndLossTab->SetModel(Ref(*m_userProfile),
    Ref(m_model->GetProfitAndLossModel()));
  m_ui->m_orderLogTab->SetModel(Ref(*m_userProfile), Ref(*m_model));
  m_ui->m_activityLogTab->SetModel(Ref(*m_userProfile), Ref(*m_model));
  m_proxyModel = new CustomVariantSortFilterProxyModel(Ref(*m_userProfile));
  m_proxyModel->setSourceModel(&m_model->GetTasksModel());
  m_ui->m_taskTable->setModel(m_proxyModel);
  m_ui->m_taskTable->setItemDelegate(new BlotterTaskEntryItemDelegate(
    Ref(*m_userProfile)));
  m_ui->m_taskTable->horizontalHeader()->setSectionsMovable(true);
  m_statusBar = new QStatusBar(this);
  QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  sizePolicy.setHeightForWidth(m_statusBar->sizePolicy().hasHeightForWidth());
  m_statusBar->setSizePolicy(sizePolicy);
  m_ui->verticalLayout->addWidget(m_statusBar);
  m_statusBar->setStyleSheet("QStatusBar::item { border: 0px solid black };");
  auto spacer = new QLabel("");
  QSizePolicy spacerSizePolicy(QSizePolicy::MinimumExpanding,
    QSizePolicy::Minimum);
  spacerSizePolicy.setHorizontalStretch(1);
  spacer->setSizePolicy(spacerSizePolicy);
  m_statusBar->addWidget(spacer);
  m_totalProfitAndLossLabel = new ValueLabel("Total P/L", Ref(*m_userProfile));
  m_totalProfitAndLossLabel->AdjustSize("9999999.99 USD");
  m_statusBar->addWidget(m_totalProfitAndLossLabel);
  m_unrealizedProfitAndLossLabel = new ValueLabel("Unrealized P/L",
    Ref(*m_userProfile));
  m_unrealizedProfitAndLossLabel->AdjustSize("9999999.99 USD");
  m_statusBar->addWidget(m_unrealizedProfitAndLossLabel);
  m_realizedProfitAndLossLabel = new ValueLabel("Realized P/L",
    Ref(*m_userProfile));
  m_realizedProfitAndLossLabel->AdjustSize("9999999.99 USD");
  m_statusBar->addWidget(m_realizedProfitAndLossLabel);
  m_feesLabel = new ValueLabel("Fees", Ref(*m_userProfile));
  m_feesLabel->AdjustSize("999999.99 USD");
  m_statusBar->addWidget(m_feesLabel);
  m_costBasisLabel = new ValueLabel("Cost Basis", Ref(*m_userProfile));
  m_costBasisLabel->AdjustSize("99999999.99 USD");
  m_statusBar->addWidget(m_costBasisLabel);
  m_toolbar = new QToolBar(this);
  static_cast<QVBoxLayout*>(m_ui->m_tasks->layout())->insertWidget(0,
    m_toolbar);
  m_executeAction = new QAction(this);
  m_executeAction->setText(tr("Execute"));
  m_executeAction->setToolTip(tr("Execute selected tasks."));
  m_executeAction->setShortcut(Qt::Key_Enter);
  m_executeAction->setIcon(QIcon(":/icons/flash_yellow.png"));
  addAction(m_executeAction);
  m_toolbar->addAction(m_executeAction);
  connect(m_executeAction, &QAction::triggered, this,
    &BlotterWindow::OnExecuteAction);
  m_cancelAction = new QAction(this);
  m_cancelAction->setText(tr("Cancel"));
  m_cancelAction->setToolTip(tr("Cancel selected tasks."));
  m_cancelAction->setShortcut(Qt::Key_Escape);
  m_cancelAction->setIcon(QIcon(":/icons/delete.png"));
  addAction(m_cancelAction);
  m_toolbar->addAction(m_cancelAction);
  connect(m_cancelAction, &QAction::triggered, this,
    &BlotterWindow::OnCancelAction);
  m_toolbar->addSeparator();
  m_pinBlotterAction = new QAction(this);
  m_pinBlotterAction->setText(tr("Pin"));
  m_pinBlotterAction->setToolTip(tr("Keep this blotter on exit."));
  m_pinBlotterAction->setIcon(QIcon(":/icons/pin2_blue.png"));
  m_pinBlotterAction->setCheckable(true);
  m_pinBlotterAction->setChecked(m_model->IsPersistent());
  m_pinBlotterAction->setEnabled(!m_model->IsConsolidated());
  addAction(m_pinBlotterAction);
  m_toolbar->addAction(m_pinBlotterAction);
  connect(m_pinBlotterAction, &QAction::toggled, this,
    &BlotterWindow::OnPinBlotterToggled);
  m_toggleActiveBlotterAction = new QAction(this);
  m_toggleActiveBlotterAction->setCheckable(true);
  SetActive(&m_userProfile->GetBlotterSettings().GetActiveBlotter() == m_model);
  m_activeBlotterChangedConnection =
    m_userProfile->GetBlotterSettings().ConnectActiveBlotterChangedSignal(
    std::bind(&BlotterWindow::OnActiveBlotterChanged, this,
    std::placeholders::_1));
  addAction(m_toggleActiveBlotterAction);
  m_toolbar->addAction(m_toggleActiveBlotterAction);
  connect(m_toggleActiveBlotterAction, &QAction::toggled, this,
    &BlotterWindow::OnActiveBlotterToggled);
  QFontMetrics metrics(m_ui->m_taskTable->font());
  m_ui->m_taskTable->verticalHeader()->setDefaultSectionSize(
    metrics.height() + 4);
  connect(m_ui->m_taskTable->selectionModel(),
    &QItemSelectionModel::currentChanged, this,
    &BlotterWindow::OnCurrentChanged);
  const auto STICKY_WIDTH = 24;
  m_ui->m_taskTable->setColumnWidth(BlotterTasksModel::STICKY_COLUMN,
    STICKY_WIDTH);
  m_ui->m_taskTable->installEventFilter(this);
  connect(m_ui->m_taskTable, &QTableView::customContextMenuRequested, this,
    &BlotterWindow::OnTaskContextMenu);
  connect(&m_updateTimer, &QTimer::timeout, this,
    &BlotterWindow::OnUpdateTimer);
  m_updateTimer.start(UPDATE_INTERVAL);
  m_profitAndLossUpdateConnection =
    m_model->GetProfitAndLossModel().ConnectProfitAndLossUpdateSignal(
    std::bind(&BlotterWindow::OnProfitAndLossUpdate, this,
    std::placeholders::_1));
  m_taskAddedConnection = m_model->GetTasksModel().ConnectTaskAddedSignal(
    std::bind(&BlotterWindow::OnTaskAdded, this, std::placeholders::_1));
  m_taskRemovedConnection = m_model->GetTasksModel().ConnectTaskRemovedSignal(
    std::bind(&BlotterWindow::OnTaskRemoved, this, std::placeholders::_1));
}

void BlotterWindow::showEvent(QShowEvent* event) {
  m_userProfile->GetBlotterSettings().RemoveRecentlyClosedWindow(*this);
}

void BlotterWindow::closeEvent(QCloseEvent* event) {
  blotterWindows.erase(m_model);
  if(m_model->IsPersistent()) {
    m_userProfile->GetBlotterSettings().AddRecentlyClosedWindow(*this);
  } else {
    m_userProfile->GetBlotterSettings().RemoveBlotter(*m_model);
  }
  QFrame::closeEvent(event);
}

bool BlotterWindow::eventFilter(QObject* object, QEvent* event) {
  if(object == m_ui->m_taskTable) {
    if(auto keyEvent = dynamic_cast<QKeyEvent*>(event)) {
      if(!m_ui->m_taskTable->selectionModel()->selectedRows().empty() &&
          keyEvent->modifiers() == 0 && keyEvent->key() == Qt::Key_Escape) {
        return QWidget::eventFilter(object, event);
      }
      QKeySequence key(keyEvent->modifiers() + keyEvent->key());
      auto cancelBinding =
        m_userProfile->GetKeyBindings().GetCancelFromBinding(key);
      if(cancelBinding.is_initialized()) {
        KeyBindings::CancelBinding::HandleCancel(*cancelBinding,
          Store(m_tasksExecuted));
        return true;
      }
    }
  }
  return QWidget::eventFilter(object, event);
}

void BlotterWindow::OnTaskAdded(const BlotterTasksModel::TaskEntry& entry) {
  m_tasksExecuted.push_back(entry.m_context);
  auto index = m_model->GetTasksModel().index(entry.m_index,
    BlotterTasksModel::STICKY_COLUMN);
  auto editorTimer = std::make_shared<QTimer>();
  editorTimer->setSingleShot(true);
  connect(editorTimer.get(), &QTimer::timeout,
    [=] () mutable {
      m_ui->m_taskTable->openPersistentEditor(
        m_proxyModel->mapFromSource(index));
      editorTimer.reset();
    });
  editorTimer->start(0);
  entry.m_context->m_task->GetPublisher().Monitor(
    m_slotHandler.GetSlot<Task::StateEntry>(std::bind(
    &BlotterWindow::OnTaskState, this,
    std::weak_ptr<BlotterTasksModel::TaskContext>(entry.m_context),
    std::placeholders::_1)));
}

void BlotterWindow::OnTaskRemoved(const BlotterTasksModel::TaskEntry& entry) {
  RemoveFirst(m_tasksExecuted, entry.m_context);
}

void BlotterWindow::SetActive(bool isActive) {
  if(isActive) {
    m_toggleActiveBlotterAction->setText(tr("Set Active"));
    m_toggleActiveBlotterAction->setToolTip(
      tr("Makes this the active blotter."));
    m_toggleActiveBlotterAction->setIcon(QIcon(":/icons/lightbulb_on.png"));
    m_toggleActiveBlotterAction->setChecked(true);
    m_toggleActiveBlotterAction->setEnabled(false);
  } else {
    m_toggleActiveBlotterAction->setText(tr("Set Active"));
    m_toggleActiveBlotterAction->setToolTip(
      tr("Makes this the active blotter."));
    m_toggleActiveBlotterAction->setIcon(QIcon(":/icons/lightbulb.png"));
    m_toggleActiveBlotterAction->setChecked(false);
    m_toggleActiveBlotterAction->setEnabled(true);
  }
}

void BlotterWindow::OnActiveBlotterChanged(BlotterModel& blotter) {
  SetActive(m_model == &blotter);
}

void BlotterWindow::OnProfitAndLossUpdate(
    const SpirePortfolioMonitor::UpdateEntry& update) {
  m_totalProfitAndLossLabel->SetValue(QVariant::fromValue(
    update.m_currencyInventory.m_grossProfitAndLoss -
    update.m_currencyInventory.m_fees + update.m_unrealizedCurrency));
  m_realizedProfitAndLossLabel->SetValue(QVariant::fromValue(
    update.m_currencyInventory.m_grossProfitAndLoss -
    update.m_currencyInventory.m_fees));
  m_unrealizedProfitAndLossLabel->SetValue(QVariant::fromValue(
    update.m_unrealizedCurrency));
  m_feesLabel->SetValue(QVariant::fromValue(update.m_currencyInventory.m_fees));
  m_costBasisLabel->SetValue(QVariant::fromValue(
    update.m_currencyInventory.m_position.m_costBasis));
}

void BlotterWindow::OnTaskState(
    std::weak_ptr<BlotterTasksModel::TaskContext> weakTask,
    const Task::StateEntry& update) {
  auto task = weakTask.lock();
  if(task == nullptr) {
    return;
  }
  if(IsTerminal(update.m_state)) {
    RemoveFirst(m_tasksExecuted, task);
  }
}

void BlotterWindow::OnExecuteAction() {
  for(auto& index : m_ui->m_taskTable->selectionModel()->selectedRows()) {
    auto& entry = m_model->GetTasksModel().GetEntry(
      m_proxyModel->mapToSource(index).row());
    entry.m_context->m_task->Execute();
  }
}

void BlotterWindow::OnCancelAction() {
  for(auto& index : m_ui->m_taskTable->selectionModel()->selectedRows()) {
    auto& entry = m_model->GetTasksModel().GetEntry(
      m_proxyModel->mapToSource(index).row());
    entry.m_context->m_task->Cancel();
  }
}

void BlotterWindow::OnActiveBlotterToggled(bool checked) {
  if(checked) {
    m_userProfile->GetBlotterSettings().SetActiveBlotter(*m_model);
    SetActive(true);
  } else {
    SetActive(false);
  }
}

void BlotterWindow::OnPinBlotterToggled(bool checked) {
  m_model->SetPersistent(checked);
}

void BlotterWindow::OnCurrentChanged(const QModelIndex& current,
    const QModelIndex& previous) {
  if(!current.isValid()) {
    m_ui->m_taskTab->ResetDisplayedNode();
    return;
  }
  m_ui->m_taskTab->SetDisplayedNode(*m_model->GetTasksModel().GetEntry(
    m_proxyModel->mapToSource(current).row()).m_context->m_node);
}

void BlotterWindow::OnTaskContextMenu(const QPoint& position) {
  QMenu contextMenu;
  QAction propertiesAction(&contextMenu);
  propertiesAction.setText(tr("Properties"));
  propertiesAction.setToolTip(tr("Opens the properties for this blotter."));
  contextMenu.addAction(&propertiesAction);
  QMenu linkMenu("Links");
  vector<unique_ptr<QAction>> linkActions;
  for(auto& model : m_userProfile->GetBlotterSettings().GetAllBlotters()) {
    if(model.get() != m_model) {
      auto linkAction = new LinkBlotterAction(Ref(*model), &linkMenu);
      linkAction->setText(QString::fromStdString(model->GetName()));
      linkAction->setCheckable(true);
      linkAction->setEnabled(!m_model->IsConsolidated());
      auto isLinked = false;
      for(auto& linkedBlotter : m_model->GetLinkedBlotters()) {
        if(model.get() == linkedBlotter) {
          isLinked = true;
          break;
        }
      }
      linkAction->setChecked(isLinked);
      linkMenu.addAction(linkAction);
      linkActions.emplace_back(linkAction);
    }
  }
  if(!linkActions.empty()) {
    contextMenu.addMenu(&linkMenu);
  }
  auto selectedAction = contextMenu.exec(static_cast<QWidget*>(
    sender())->mapToGlobal(position));
  if(selectedAction == &propertiesAction) {
    BlotterMonitorsDialog blotterMonitorsDialog(Ref(*m_userProfile),
      m_model->GetTasksModel().GetProperties(), Ref(*m_model), this);
    if(blotterMonitorsDialog.exec() == QDialog::Rejected) {
      return;
    }
    m_model->GetTasksModel().SetProperties(
      blotterMonitorsDialog.GetProperties());
  } else {
    auto linkAction = dynamic_cast<LinkBlotterAction*>(selectedAction);
    if(linkAction == nullptr) {
      return;
    }
    auto& blotter = linkAction->GetBlotterModel();
    if(linkAction->isChecked()) {
      m_model->Link(Ref(blotter));
    } else {
      m_model->Unlink(blotter);
    }
  }
}

void BlotterWindow::OnUpdateTimer() {
  HandleTasks(m_slotHandler);
}