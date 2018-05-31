#include "spire/time_and_sales/time_and_sales_window.hpp"
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <QHeaderView>
#include <QKeyEvent>
#include <QScrollBar>
#include <QTableView>
#include <QVBoxLayout>
#include "spire/security_input/security_input_dialog.hpp"
#include "spire/security_input/security_input_model.hpp"
#include "spire/time_and_sales/empty_time_and_sales_model.hpp"
#include "spire/time_and_sales/periodic_time_and_sales_model.hpp"
#include "spire/time_and_sales/time_and_sales_properties_dialog.hpp"
#include "spire/time_and_sales/time_and_sales_window_model.hpp"
#include "spire/spire/dimensions.hpp"
#include "spire/ui/custom_qt_variants.hpp"
#include "spire/ui/drop_shadow.hpp"
#include "spire/ui/item_padding_delegate.hpp"
#include "spire/ui/window.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace spire;

time_and_sales_window::time_and_sales_window(
    const time_and_sales_properties& properties, 
    security_input_model& input_model, QWidget* parent)
    : QWidget(parent),
      m_input_model(&input_model),
      m_v_scrolling(false),
      m_h_scrolling(false) {
  m_body = new QWidget(this);
  m_body->setMinimumSize(scale(180, 200));
  resize(scale_width(182), scale_height(452));
  m_body->setStyleSheet("background-color: #FFFFFF;");
  auto window_layout = new QHBoxLayout(this);
  window_layout->setContentsMargins({});
  auto window = new spire::window(m_body, this);
  setWindowTitle(tr("Time and Sales"));
  window->set_icon(imageFromSvg(":/icons/time-sale-black.svg", scale(26, 26),
    QRect(translate(8, 8), scale(10, 10))),
    imageFromSvg(":/icons/time-sale-grey.svg", scale(26, 26),
    QRect(translate(8, 8), scale(10, 10))));
  window_layout->addWidget(window);
  auto layout = new QVBoxLayout(m_body);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  auto padding_widget = new QWidget(this);
  padding_widget->setFixedHeight(scale_height(4));
  padding_widget->setStyleSheet("background-color: #F5F5F5;");
  layout->addWidget(padding_widget);
  m_empty_window_label = new QLabel(
    tr("Type on keyboard<br/>to search a security"), this);
  m_empty_window_label->setAlignment(Qt::AlignCenter);
  m_empty_window_label->setStyleSheet(QString(R"(
    font-family: Roboto;
    font-size: %1px;)").arg(scale_height(11)));
  layout->addWidget(m_empty_window_label);
  m_overlay_widget = new QLabel(m_body);
  m_overlay_widget->setStyleSheet(
    "background-color: rgba(245, 245, 245, 153);");
  m_overlay_widget->hide();
  m_table = new QTableView(this);
  m_table->setItemDelegate(new item_padding_delegate(scale_width(5),
    new custom_variant_item_delegate(), this));
  m_table->setMouseTracking(true);
  m_table->installEventFilter(this);
  m_table->viewport()->setAttribute(Qt::WA_TransparentForMouseEvents);
  m_table->setFocusPolicy(Qt::NoFocus);
  m_table->setSelectionMode(QAbstractItemView::NoSelection);
  m_table->horizontalHeader()->setStretchLastSection(true);
  m_table->horizontalHeader()->viewport()->setMouseTracking(true);
  m_table->horizontalHeader()->viewport()->installEventFilter(this);
  m_table->verticalHeader()->setVisible(false);
  m_v_scroll_bar_timer = new QTimer(this);
  m_v_scroll_bar_timer->setInterval(500);
  connect(m_v_scroll_bar_timer, &QTimer::timeout, [=] {
    fade_out_vertical_scroll_bar(); });
  m_h_scroll_bar_timer = new QTimer(this);
  m_h_scroll_bar_timer->setInterval(500);
  connect(m_h_scroll_bar_timer, &QTimer::timeout, [=] {
    fade_out_horizontal_scroll_bar(); });
  m_table->horizontalScrollBar()->installEventFilter(this);
  m_table->verticalScrollBar()->installEventFilter(this);
  m_table->horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
  m_table->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
  m_table->horizontalScrollBar()->setAttribute(Qt::WA_Hover);
  m_table->verticalScrollBar()->setAttribute(Qt::WA_Hover);
  m_table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_table->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  m_table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
  m_table->verticalHeader()->setSectionsClickable(false);
  m_table->horizontalHeader()->setSectionsClickable(false);
  m_table->horizontalHeader()->setSectionsMovable(true);
  m_table->horizontalHeader()->setFixedHeight(scale_height(26));
  m_table->horizontalHeader()->setDefaultAlignment(
    Qt::AlignLeft | Qt::AlignVCenter);
  m_table->horizontalHeader()->setStyleSheet(QString(R"(
    QHeaderView::section {
      background-color: #FFFFFF;
      background-image: url(:icons/column-border.png);
      background-position: left;
      background-repeat: repeat;
      border: none;
      color: #4B23A0;
      font-family: Roboto;
      font-size: %1px;
      font-weight: 550;
      padding-left: %2px;
    }

    QHeaderView::section::first {
      background: none;
      background-color: #FFFFFF;
    })").arg(scale_height(11)).arg(scale_width(8)));
  m_table->setStyleSheet(QString(R"(
    QTableView {
      border: none;
      gridline-color: #C8C8C8;
    }

    QScrollBar::horizontal {
      height: %1px;
    }

    QScrollBar::vertical {
      width: %2px;
    }

    QScrollBar::handle {
      background-color: #C8C8C8;
    }

    QScrollBar::handle:horizontal {
      min-width: %3px;
    }

    QScrollBar::handle:vertical {
      min-height: %4px;
    }

    QScrollBar::add-line, QScrollBar::sub-line,
    QScrollBar::add-page, QScrollBar::sub-page {
      background: none;
      border: none;
      height: 0px;
      width: 0px;
    })").arg(scale_height(12)).arg(scale_height(12))
        .arg(scale_width(30)).arg(scale_height(30)));
  m_table->hide();
  layout->addWidget(m_table);
  m_volume_label = new QLabel(tr("Volume: 0"), this);
  m_volume_label->setFocusPolicy(Qt::NoFocus);
  m_volume_label->setFixedHeight(scale_height(20));
  m_volume_label->setStyleSheet(QString(R"(
    background-color: #F5F5F5;
    color: #333333;
    font-family: Roboto;
    font-size: %1px;
    font-weight: 550;
    padding-left: %2px;)").arg(scale_height(10)).arg(scale_width(8)));
  layout->addWidget(m_volume_label);
  m_context_menu = new QMenu(this);
  auto properties_action = new QAction(tr("Properties"), m_context_menu);
  connect(properties_action, &QAction::triggered,
    [=] { show_properties_dialog(); });
  m_context_menu->addAction(properties_action);
  auto book_view_action = new QAction(tr("Link Book View"), m_context_menu);
  m_context_menu->addAction(book_view_action);
  auto chart_action = new QAction(tr("Link Chart"), m_context_menu);
  m_context_menu->addAction(chart_action);
  auto export_action = new QAction(tr("Export Table"), m_context_menu);
  m_context_menu->addAction(export_action);
  m_context_menu->setFixedWidth(scale_width(140));
  m_context_menu->setWindowFlag(Qt::NoDropShadowWindowHint);
  m_context_menu_shadow = std::make_unique<drop_shadow>(
    true, true, m_context_menu);
  m_context_menu->setStyleSheet(QString(R"(
    QMenu {
      background-color: #FFFFFF;
      border: %1px solid #A0A0A0 %2px solid #A0A0A0;
      color: #000000;
      font-family: Roboto;
      font-size: %3px;
      padding: %4px 0px;
    }

    QMenu::item {
      padding: %5px 0px %5px %6px;
    }

    QMenu::item:selected, QMenu::item:hover {
      background-color: #8D78EC;
      color: #FFFFFF;
    })")
    .arg(scale_height(1)).arg(scale_width(1))
    .arg(scale_height(12)).arg(scale_height(5))
    .arg(scale_height(3)).arg(scale_width(8)));
  set_model(std::make_shared<empty_time_and_sales_model>(Security()));
  set_properties(properties);
  m_properties_dialog = new time_and_sales_properties_dialog(properties, this);
  connect(m_properties_dialog, &QDialog::accepted,
    [=] { on_properties_ok(); });
  m_properties_dialog->connect_apply_signal(
    [=] (auto p) { on_properties_apply(); });
  QWidget::window()->setAttribute(Qt::WA_Hover);
  QWidget::window()->installEventFilter(this);
}

void time_and_sales_window::set_model(
    std::shared_ptr<time_and_sales_model> model) {
  model->connect_volume_signal(
    [=] (const Quantity& v) { update_volume(v); });
  m_model.emplace(std::move(model), m_properties);
  connect(m_model.get_ptr(), &QAbstractTableModel::rowsAboutToBeInserted,
    [=] { maintain_table_position(); });
  auto filter = new custom_variant_sort_filter_proxy_model(this);
  filter->setSourceModel(&m_model.get());
  m_table->setModel(filter);
}

const time_and_sales_properties& time_and_sales_window::get_properties() const {
  return m_properties;
}

void time_and_sales_window::set_properties(
    const time_and_sales_properties& properties) {
  m_properties = properties;
  m_model.get().set_properties(m_properties);
  for(auto i = 0; i < static_cast<int>(
      m_properties.m_show_columns.size()); ++i) {
    if(!m_properties.m_show_columns[i]) {
      m_table->hideColumn(i);
    } else {
      m_table->showColumn(i);
    }
  }
  if(m_properties.m_show_grid) {
    m_table->setShowGrid(true);
  } else {
    m_table->setShowGrid(false);
  }
  QFontMetrics metrics(m_properties.m_font);
  auto row_height = metrics.height() + scale_height(2);
  m_table->verticalHeader()->setDefaultSectionSize(row_height);
}

connection time_and_sales_window::connect_security_change_signal(
    const change_security_signal::slot_type& slot) const {
  return m_change_security_signal.connect(slot);
}

connection time_and_sales_window::connect_closed_signal(
    const closed_signal::slot_type& slot) const {
  return m_closed_signal.connect(slot);
}

void time_and_sales_window::closeEvent(QCloseEvent* event) {
  m_closed_signal();
}

void time_and_sales_window::contextMenuEvent(QContextMenuEvent* event) {
  QRect widget_geometry;
  if(!m_empty_window_label->isHidden()) {
    widget_geometry = QRect(
      m_empty_window_label->mapToGlobal(
        m_empty_window_label->geometry().topLeft()),
      m_empty_window_label->mapToGlobal(
        m_empty_window_label->geometry().bottomRight()));
  } else {
    widget_geometry = QRect(
      m_table->mapToGlobal(m_table->geometry().topLeft()),
      m_table->mapToGlobal(m_table->geometry().bottomRight()));
  }
  if(widget_geometry.contains(event->globalPos())) {
    m_context_menu->exec(event->globalPos());
  }
}

bool time_and_sales_window::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_table ||
      watched == m_table->horizontalHeader()->viewport()) {
    if(event->type() == QEvent::Wheel) {
      auto e = static_cast<QWheelEvent*>(event);
      if(e->modifiers() & Qt::ShiftModifier) {
        if(m_table->verticalScrollBar()->isVisible()) {
          m_table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }
        m_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        if(e->angleDelta().y() < 0) {
          m_table->horizontalScrollBar()->setValue(
            m_table->horizontalScrollBar()->value() +
            m_table->horizontalScrollBar()->singleStep());
        } else {
          m_table->horizontalScrollBar()->setValue(
            m_table->horizontalScrollBar()->value() -
            m_table->horizontalScrollBar()->singleStep());
        }
        m_h_scrolling = true;
        m_h_scroll_bar_timer->start();
      } else {
        if(!within_h_scroll_bar(e->pos())) {
          if(m_table->horizontalScrollBar()->isVisible()) {
            m_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
          }
          m_table->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
          if(e->angleDelta().y() < 0) {
            m_table->verticalScrollBar()->setValue(
              m_table->verticalScrollBar()->value() +
              m_table->verticalScrollBar()->singleStep());
          } else {
            m_table->verticalScrollBar()->setValue(
              m_table->verticalScrollBar()->value() -
              m_table->verticalScrollBar()->singleStep());
          }
          m_v_scrolling = true;
          m_v_scroll_bar_timer->start();
        }
      }
    } else if(event->type() == QEvent::MouseMove) {
      auto e = static_cast<QMouseEvent*>(event);
      if(e->pos().x() > width() - m_table->verticalScrollBar()->width()) {
        m_table->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        m_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      } else if(within_h_scroll_bar(e->pos())) {
        m_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        m_table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      } else {
        if(!m_v_scrolling &&
            m_table->verticalScrollBarPolicy() != Qt::ScrollBarAlwaysOff) {
          fade_out_vertical_scroll_bar();
        }
        if(!m_h_scrolling &&
            m_table->horizontalScrollBarPolicy() != Qt::ScrollBarAlwaysOff) {
          fade_out_horizontal_scroll_bar();
        }
      }
    }
  } else if(watched == m_table->verticalScrollBar()) {
    if(event->type() == QEvent::HoverLeave) {
      fade_out_vertical_scroll_bar();
    }
  } else if(watched == m_table->horizontalScrollBar()) {
    if(event->type() == QEvent::HoverLeave) {
      fade_out_horizontal_scroll_bar();
    }
  } else if(watched == QWidget::window()) {
    if(event->type() == QEvent::HoverLeave) {
      if(m_table->verticalScrollBarPolicy() != Qt::ScrollBarAlwaysOff) {
        fade_out_vertical_scroll_bar();
      } else if(m_table->horizontalScrollBarPolicy() !=
          Qt::ScrollBarAlwaysOff) {
        fade_out_horizontal_scroll_bar();
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

void time_and_sales_window::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_PageUp) {
    if(m_current_security != Security()) {
      auto s = m_securities.push_front(m_current_security);
      if(s != Security()) {
        set_current(s);
      }
    }
    return;
  } else if(event->key() == Qt::Key_PageDown) {
    if(m_current_security != Security()) {
      auto s = m_securities.push_back(m_current_security);
      if(s != Security()) {
        set_current(s);
      }
    }
    return;
  }
  auto pressed_key = event->text();
  if(pressed_key[0].isLetterOrNumber()) {
    show_overlay_widget();
    auto dialog = new security_input_dialog(*m_input_model, pressed_key, this);
    dialog->move(geometry().center().x() - dialog->width() / 2,
      geometry().center().y() - dialog->height() / 2);
    if(dialog->exec() == QDialog::Accepted) {
      auto s = dialog->get_security();
      if(s != Security() && s != m_current_security) {
        m_securities.push(m_current_security);
        set_current(s);
        activateWindow();
      }
    }
    m_overlay_widget->hide();
  }
}

void time_and_sales_window::fade_out_horizontal_scroll_bar() {
  m_h_scroll_bar_timer->stop();
  m_h_scrolling = false;
  m_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void time_and_sales_window::fade_out_vertical_scroll_bar() {
  m_v_scroll_bar_timer->stop();
  m_v_scrolling = false;
  m_table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void time_and_sales_window::maintain_table_position() {
  if(m_table->verticalScrollBar()->value() != 0) {
    m_table->verticalScrollBar()->setValue(
      m_table->verticalScrollBar()->value() + m_table->rowHeight(0));
  }
}

void time_and_sales_window::on_properties_apply() {
  set_properties(m_properties_dialog->get_properties());
}

void time_and_sales_window::on_properties_ok() {
  set_properties(m_properties_dialog->get_properties());
}

void time_and_sales_window::show_overlay_widget() {
  if(m_empty_window_label->isVisible()) {
    m_overlay_widget->resize(m_empty_window_label->size());
    m_overlay_widget->move(m_empty_window_label->mapTo(m_empty_window_label,
      m_empty_window_label->pos()));
  } else {
    m_overlay_widget->resize(m_table->size());
    m_overlay_widget->move(m_table->mapTo(m_table, m_table->pos()));
  }
  m_overlay_widget->show();
  m_overlay_widget->raise();
}

void time_and_sales_window::show_properties_dialog() {
  m_properties_dialog->show();
}

void time_and_sales_window::set_current(const Security& s) {
  if(s == m_current_security) {
    return;
  }
  m_empty_window_label->hide();
  m_table->show();
  m_current_security = s;
  m_change_security_signal(s);
  setWindowTitle(QString::fromStdString(ToString(s)) +
    tr(" - Time and Sales"));
}

void time_and_sales_window::update_volume(const Quantity& volume) {
  m_volume_label->setText(tr("Volume: ") + Beam::ToString(volume).c_str());
}

bool time_and_sales_window::within_h_scroll_bar(const QPoint& pos) {
  return pos.y() > m_table->height() - 
    m_table->horizontalScrollBar()->height();
}
