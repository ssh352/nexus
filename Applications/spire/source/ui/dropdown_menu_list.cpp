#include "spire/ui/dropdown_menu_list.hpp"
#include <QVBoxLayout>
#include "spire/spire/dimensions.hpp"
#include "spire/ui/drop_shadow.hpp"

using namespace Spire;

DropdownMenuList::DropdownMenuList(
    const std::initializer_list<QString>& items,
    QWidget* parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint) {
  setAttribute(Qt::WA_ShowWithoutActivating);
  setAttribute(Qt::WA_TranslucentBackground);
  m_shadow = std::make_unique<DropShadow>(false, this);
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins({});
  m_scroll_area = new QScrollArea(this);
  m_scroll_area->setWidgetResizable(true);
  m_scroll_area->setObjectName("dropdown_menu_list_scroll_area");
  m_scroll_area->setFrameShape(QFrame::NoFrame);
  m_scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  m_scroll_area->setStyleSheet(QString(R"(
    #security_info_list_view_scroll_area {
      background-color: #FFFFFF;
      border-bottom: %2px solid #A0A0A0;
      border-left: %2px solid #A0A0A0;
      border-right: %2px solid #A0A0A0;
      border-top: none;
    }
    
    QScrollBar {
      background-color: #FFFFFF;
      border: none;
    }

    QScrollBar::handle:vertical {
      background-color: #EBEBEB;
      margin-left: %3px;
      width: %1px;
    }

    QScrollBar::sub-line:vertical {
      background: none;
      border: none;
    }

    QScrollBar::add-line:vertical {
      background: none;
      border: none;
    })").arg(scale_width(15)).arg(scale_width(1)).arg(scale_width(2)));
  layout->addWidget(m_scroll_area);
  m_list_widget = new QWidget(m_scroll_area);
  auto list_layout = new QVBoxLayout(m_list_widget);
  list_layout->setContentsMargins({});
  list_layout->setSpacing(0);
  m_list_widget->setStyleSheet("background-color: red;");
  m_scroll_area->setWidget(m_list_widget);
}
