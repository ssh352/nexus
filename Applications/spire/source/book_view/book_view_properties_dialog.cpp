#include "spire/book_view/book_view_properties_dialog.hpp"
#include <QHBoxLayout>
#include <QTabBar>
#include <QTabWidget>
#include <QVBoxLayout>
#include "spire/book_view/book_view_highlight_properties_widget.hpp"
#include "spire/book_view/book_view_level_properties_widget.hpp"
#include "spire/key_bindings/interactions_properties_widget.hpp"
#include "spire/spire/dimensions.hpp"
#include "spire/ui/properties_window_buttons_widget.hpp"
#include "spire/ui/window.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace spire;

book_view_properties_dialog::book_view_properties_dialog(
    const book_view_properties& properties, const Security& security,
    QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags | Qt::Window | Qt::FramelessWindowHint |
        Qt::WindowCloseButtonHint) {
  auto body = new QWidget(this);
  body->setFixedSize(scale(492, 394));
  body->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  body->setStyleSheet("background-color: #F5F5F5;");
  auto window_layout = new QHBoxLayout(this);
  window_layout->setContentsMargins({});
  auto window = new spire::window(body, this);
  setWindowTitle(tr("Properties"));
  window->set_svg_icon(":/icons/bookview-black.svg",
    ":/icons/bookview-grey.svg");
  window_layout->addWidget(window);
  auto layout = new QVBoxLayout(body);
  layout->setContentsMargins(scale_width(8), scale_height(10), scale_width(8),
    0);
  layout->setSpacing(0);
  auto tab_widget = new QTabWidget(body);
  tab_widget->tabBar()->setFixedHeight(scale_height(40));
  tab_widget->setStyleSheet(QString(R"(
    QWidget {
      outline: %6px solid #4B23A0;
    }

    QTabWidget::pane {
      border: none;
    }

    QTabBar::tab {
      background-color: #EBEBEB;
      font-family: Roboto;
      font-size: %1px;
      height: %2px;
      margin: %3px %4px %3px 0px;
      width: %5px;
    }

    QTabBar::tab:hover {
      color: #4B23A0;
    }

    QTabBar::tab:selected {
      background-color: #F5F5F5;
      color: #4B23A0;
    })").arg(scale_height(12)).arg(scale_height(20)).arg(scale_height(10))
        .arg(scale_width(2)).arg(scale_width(80)).arg(scale_width(1)));
  auto levels_tab_widget = new book_view_level_properties_widget(properties,
    tab_widget);
  tab_widget->addTab(levels_tab_widget, tr("Display"));
  auto highlights_tab_widget = new book_view_highlight_properties_widget(
    properties, tab_widget);
  tab_widget->addTab(highlights_tab_widget, tr("Highlights"));
  if(security != Security()) {
    auto interactions_tab_widget = new interactions_properties_widget(
      tab_widget);
    tab_widget->addTab(interactions_tab_widget, tr("Interactions"));
  }
  layout->addWidget(tab_widget);
  auto button_group_widget = new properties_window_buttons_widget(this);
  button_group_widget->connect_cancel_signal([=] { reject(); });
  button_group_widget->connect_ok_signal([=] { accept(); });
  layout->addWidget(button_group_widget);
}

book_view_properties book_view_properties_dialog::get_properties() const {
  return m_properties;
}

connection book_view_properties_dialog::connect_apply_signal(
    const apply_signal::slot_type& slot) const {
  return m_apply_signal.connect(slot);
}

connection book_view_properties_dialog::connect_apply_all_signal(
    const apply_all_signal::slot_type& slot) const {
  return m_apply_all_signal.connect(slot);
}

connection book_view_properties_dialog::connect_save_default_signal(
    const save_default_signal::slot_type& slot) const {
  return m_save_default_signal.connect(slot);
}
