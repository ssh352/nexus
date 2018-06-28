#include "spire/time_and_sales/time_and_sales_table_view.hpp"
#include <QApplication>
#include <QEvent>
#include <QHoverEvent>
#include <QMovie>
#include <QScrollBar>
#include <QTableView>
#include "spire/spire/dimensions.hpp"
#include "spire/time_and_sales/snapshot_loading_widget.hpp"
#include "spire/time_and_sales/time_and_sales_window_model.hpp"
#include "spire/ui/custom_qt_variants.hpp"
#include "spire/ui/item_padding_delegate.hpp"
#include "spire/ui/overlay_widget.hpp"

using namespace spire;

namespace {
  const auto MINIMUM_TABLE_WIDTH = 750;
  const auto SCROLL_BAR_FADE_TIME_MS = 500;
}

time_and_sales_table_view::time_and_sales_table_view(QWidget* parent)
    : QScrollArea(parent),
      m_model(nullptr) {
  setStyleSheet(QString(R"(
    QWidget {
      background-color: #FFFFFF;
      border: none;
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
  setMouseTracking(true);
  setAttribute(Qt::WA_Hover);
  horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
  verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  connect(horizontalScrollBar(), &QScrollBar::valueChanged, this,
    &time_and_sales_table_view::on_horizontal_slider_value_changed);
  connect(verticalScrollBar(), &QScrollBar::valueChanged, this,
    &time_and_sales_table_view::on_vertical_slider_value_changed);
  m_header = new QHeaderView(Qt::Horizontal, this);
  m_header->setMinimumSectionSize(scale_width(35));
  m_header->setStretchLastSection(true);
  m_header->setSectionsClickable(false);
  m_header->setSectionsMovable(true);
  m_header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  m_header->setStyleSheet(QString(R"(
    QHeaderView::section {
      background-color: #FFFFFF;
      background-image: url(:icons/column-border.png);
      background-position: left;
      background-repeat: repeat;
      border: none;
      color: #4B23A0;
      font-family: Roboto;
      font-weight: 550;
      padding-left: %1px;
      padding-right: %1px;
    }

    QHeaderView::section::first {
      background: none;
      background-color: #FFFFFF;
    })").arg(scale_width(8)));
  connect(m_header, &QHeaderView::sectionResized, this,
    &time_and_sales_table_view::on_header_resize);
  connect(m_header, &QHeaderView::sectionMoved, this,
    &time_and_sales_table_view::on_header_move);
  auto main_widget = new QWidget(this);
  main_widget->setMinimumWidth(MINIMUM_TABLE_WIDTH);
  m_layout = new QVBoxLayout(main_widget);
  m_layout->setContentsMargins({});
  m_layout->setSpacing(0);
  m_header_padding = new QWidget(this); 
  m_header_padding->setFixedHeight(m_header->height());
  m_layout->addWidget(m_header_padding);
  m_table = new QTableView(this);
  m_table->setMinimumWidth(MINIMUM_TABLE_WIDTH);
  m_table->setFocusPolicy(Qt::NoFocus);
  m_table->setAttribute(Qt::WA_TransparentForMouseEvents);
  m_table->setSelectionMode(QAbstractItemView::NoSelection);
  m_table->horizontalHeader()->setStretchLastSection(true);
  m_table->horizontalHeader()->hide();
  m_table->verticalHeader()->hide();
  m_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_table->setStyleSheet(R"(
      border: none;
      gridline-color: #C8C8C8;)");
  m_table->installEventFilter(this);
  m_table->setItemDelegate(new item_padding_delegate(scale_width(5),
    new custom_variant_item_delegate(), this));
  m_layout->addWidget(m_table);
  m_h_scroll_bar_timer.setInterval(SCROLL_BAR_FADE_TIME_MS);
  connect(&m_h_scroll_bar_timer, &QTimer::timeout, this,
    &time_and_sales_table_view::fade_out_horizontal_scroll_bar);
  m_v_scroll_bar_timer.setInterval(SCROLL_BAR_FADE_TIME_MS);
  connect(&m_v_scroll_bar_timer, &QTimer::timeout, this,
    &time_and_sales_table_view::fade_out_vertical_scroll_bar);
  setWidget(main_widget);
}

void time_and_sales_table_view::set_model(time_and_sales_window_model* model) {
  m_model = model;
  auto filter = new custom_variant_sort_filter_proxy_model(this);
  filter->setSourceModel(m_model);
  connect(filter, &QAbstractItemModel::rowsAboutToBeInserted, this,
    &time_and_sales_table_view::on_rows_about_to_be_inserted);
  m_header->setModel(filter);
  m_table->setModel(filter);
}

void time_and_sales_table_view::set_properties(
    const time_and_sales_properties& properties) {
  m_table->setShowGrid(properties.m_show_grid);
  QFontMetrics metrics(properties.m_font);
  auto row_height = metrics.height() + scale_height(2);
  m_table->verticalHeader()->setDefaultSectionSize(row_height);
  auto header_font = m_table->verticalHeader()->font();
  if(properties.m_font.pointSize() >= 11) {
    header_font.setPointSizeF(properties.m_font.pointSize() * 0.8);
  } else {
    header_font.setPointSize(9);
  }
  m_header->setFont(header_font);
  QFontMetrics header_metrics(header_font);
  m_header->setFixedHeight(header_metrics.height() * 1.8);
  m_header_padding->setFixedHeight(m_header->height());
  if(m_table->model()->rowCount() > 0) {
    update_table_height();
  }
}

void time_and_sales_table_view::show_transition_widget() {
  if(m_table->model()->rowCount(QModelIndex()) == 0) {
    auto backing_widget = new QLabel(this);
    auto logo = new QMovie(":/icons/pre-loader.gif", QByteArray(),
      backing_widget);
    logo->setScaledSize(scale(32, 32));
    backing_widget->setMovie(logo);
    backing_widget->setStyleSheet(
      QString("padding-top: %1px;").arg(scale_height(50)));
    backing_widget->setAlignment(Qt::AlignHCenter);
    backing_widget->movie()->start();
    m_transition_widget = std::make_unique<overlay_widget>(
      this, backing_widget, this);
  }
}

bool time_and_sales_table_view::event(QEvent* event) {
  if(event->type() == QEvent::HoverMove) {
    auto e = static_cast<QHoverEvent*>(event);
    if(is_within_horizontal_scroll_bar(e->pos()) &&
        !verticalScrollBar()->isSliderDown()) {
      setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
      setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    } else if(is_within_vertical_scroll_bar(e->pos()) &&
        !horizontalScrollBar()->isSliderDown()) {
      setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
      setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    } else {
      if(!m_v_scroll_bar_timer.isActive() &&
          verticalScrollBarPolicy() != Qt::ScrollBarAlwaysOff &&
          !verticalScrollBar()->isSliderDown()) {
        fade_out_vertical_scroll_bar();
      }
      if(!m_h_scroll_bar_timer.isActive() &&
          horizontalScrollBarPolicy() != Qt::ScrollBarAlwaysOff &&
          !horizontalScrollBar()->isSliderDown()) {
        fade_out_horizontal_scroll_bar();
      }
    }
  } else if(event->type() == QEvent::HoverLeave) {
    if(!m_h_scroll_bar_timer.isActive()) {
      fade_out_horizontal_scroll_bar();
    }
    if(!m_v_scroll_bar_timer.isActive()) {
      fade_out_vertical_scroll_bar();
    }
  }
  return QScrollArea::event(event);
}

bool time_and_sales_table_view::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_table) {
    if(event->type() == QEvent::Paint) {
      m_table->update();
      m_header->update();
    }
  }
  return QScrollArea::eventFilter(watched, event);
}

void time_and_sales_table_view::resizeEvent(QResizeEvent* event) {
  widget()->resize(width(), widget()->height());
  m_header->setFixedWidth(widget()->width());
}

void time_and_sales_table_view::wheelEvent(QWheelEvent* event) {
  if(event->modifiers() & Qt::ShiftModifier) {
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    horizontalScrollBar()->setValue(horizontalScrollBar()->value() -
      (event->delta() / 2));
    m_h_scroll_bar_timer.start();
  } else if(!event->modifiers().testFlag(Qt::ShiftModifier)) {
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    verticalScrollBar()->setValue(verticalScrollBar()->value() -
      (event->delta() / 2));
    m_v_scroll_bar_timer.start();
  }
}

void time_and_sales_table_view::fade_out_horizontal_scroll_bar() {
  m_h_scroll_bar_timer.stop();
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void time_and_sales_table_view::fade_out_vertical_scroll_bar() {
  m_v_scroll_bar_timer.stop();
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void time_and_sales_table_view::show_loading_widget() {
  m_loading_widget = std::make_unique<snapshot_loading_widget>(this);
  m_layout->addWidget(m_loading_widget.get());
}

void time_and_sales_table_view::hide_loading_widget() {
  m_loading_widget.reset();
}

int time_and_sales_table_view::get_table_height_with_additional_row() {
  auto height = (m_table->model()->rowCount() + 1) *
    m_table->rowHeight(0) + m_header->height();
  if(m_loading_widget != nullptr) {
    height += m_loading_widget->height();
  }
  return height;
}

void time_and_sales_table_view::update_table_height() {
  widget()->setFixedHeight(get_table_height_with_additional_row());
  if(verticalScrollBar()->value() != 0) {
    verticalScrollBar()->setValue(verticalScrollBar()->value() +
      m_table->rowHeight(0));
  }
}

bool time_and_sales_table_view::is_within_horizontal_scroll_bar(
    const QPoint& pos) {
  return pos.y() > visibleRegion().boundingRect().height() -
    horizontalScrollBar()->height();
}

bool time_and_sales_table_view::is_within_vertical_scroll_bar(const QPoint& pos) {
  return pos.x() > width() - verticalScrollBar()->width();
}

void time_and_sales_table_view::on_header_resize(int index, int old_size,
    int new_size) {
  m_table->horizontalHeader()->resizeSection(index,
    m_header->sectionSize(index));
}

void time_and_sales_table_view::on_header_move(int logical_index,
    int old_index, int new_index) {
  m_table->horizontalHeader()->moveSection(old_index, new_index);
}

void time_and_sales_table_view::on_horizontal_slider_value_changed(
    int new_value) {
  if(new_value != 0) {
    m_header->move(widget()->pos().x(), m_header->pos().y());
  } else {
    m_header->move(0, m_header->pos().y());
  }
}

void time_and_sales_table_view::on_vertical_slider_value_changed(
    int new_value) {
  if(m_model == nullptr) {
    return;
  }
  m_model->set_row_visible(m_table->rowAt(
    widget()->visibleRegion().boundingRect().top()));
}

void time_and_sales_table_view::on_rows_about_to_be_inserted() {
  if(m_table->model()->rowCount() > 0) {
    update_table_height();
  } else {
    m_transition_widget.reset();
  }
}