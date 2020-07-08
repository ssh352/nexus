#include "Spire/TimeAndSales/TimeAndSalesTableView.hpp"
#include <QEvent>
#include <QHoverEvent>
#include <QMovie>
#include <QScrollBar>
#include <QTableView>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/TimeAndSales/SnapshotLoadingWidget.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindowModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/ItemPaddingDelegate.hpp"
#include "Spire/Ui/TransitionWidget.hpp"

using namespace Spire;

namespace {
  const auto MINIMUM_TABLE_WIDTH = 750;
  const auto SCROLL_BAR_FADE_TIME_MS = 500;
  const auto SCROLL_BAR_MAX_SIZE = 13;
  const auto SCROLL_BAR_MIN_SIZE = 6;
}

TimeAndSalesTableView::TimeAndSalesTableView(QWidget* parent)
    : ScrollArea(true, parent),
      m_was_shown(false),
      m_model(nullptr),
      m_loading_widget(nullptr),
      m_transition_widget(nullptr),
      m_minimum_column_widths({{0, scale_width(54)}, {1, scale_width(46)},
    {2, scale_width(38)}, {3, scale_width(40)}, {4, scale_width(42)}}) {
  connect(horizontalScrollBar(), &QScrollBar::valueChanged, this,
    &TimeAndSalesTableView::on_horizontal_slider_value_changed);
  connect(verticalScrollBar(), &QScrollBar::valueChanged, this,
    &TimeAndSalesTableView::on_vertical_slider_value_changed);
  m_header = new QHeaderView(Qt::Horizontal, this);
  m_header->setStretchLastSection(true);
  m_header->setSectionsClickable(false);
  m_header->setSectionsMovable(true);
  m_header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  m_header->setStyleSheet(QString(R"(
    QHeaderView::section {
      background-color: #FFFFFF;
      background-image: url(:/Icons/column-border.png);
      background-position: left;
      background-repeat: repeat;
      border: none;
      color: #4B23A0;
      padding-left: %1px;
      padding-right: %1px;
    }

    QHeaderView::section::first {
      background: none;
      background-color: #FFFFFF;
    })").arg(scale_width(8)));
  connect(m_header, &QHeaderView::sectionResized, this,
    &TimeAndSalesTableView::on_header_resize);
  connect(m_header, &QHeaderView::sectionMoved, this,
    &TimeAndSalesTableView::on_header_move);
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
  m_table->verticalHeader()->setMinimumSectionSize(0);
  m_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_table->setStyleSheet(R"(
      border: none;
      gridline-color: #C8C8C8;)");
  m_table->installEventFilter(this);
  m_table->setItemDelegate(new ItemPaddingDelegate(scale_width(5),
    new CustomVariantItemDelegate(), this));
  m_layout->addWidget(m_table);
  setWidget(main_widget);
}

void TimeAndSalesTableView::set_model(TimeAndSalesWindowModel* model) {
  m_model = model;
  delete_later(m_loading_widget);
  if(m_model->is_loading() && m_transition_widget == nullptr) {
    m_transition_widget = new TransitionWidget(this);
  }
  m_model_begin_loading_connection = m_model->connect_begin_loading_signal(
    [=] { show_loading_widget(); });
  m_model_end_loading_connection = m_model->connect_end_loading_signal(
    [=] { on_end_loading_signal(); });
  auto filter = new CustomVariantSortFilterProxyModel(this);
  filter->setSourceModel(m_model);
  connect(filter, &QAbstractItemModel::rowsAboutToBeInserted, this,
    &TimeAndSalesTableView::on_rows_about_to_be_inserted);
  m_header->setModel(filter);
  m_table->setModel(filter);
  for(auto i = 0; i < m_table->model()->columnCount(); ++i) {
    m_model->set_column_size_reference(i, m_table->columnWidth(i));
  }
}

void TimeAndSalesTableView::set_properties(
    const TimeAndSalesProperties& properties) {
  m_table->setShowGrid(properties.m_show_grid);
  auto metrics = QFontMetrics(properties.m_font);
  auto row_height = metrics.height() + scale_height(2);
  m_table->verticalHeader()->setDefaultSectionSize(row_height);
  auto header_metrics = QFontMetrics(m_header->model()->headerData(0,
    Qt::Horizontal, Qt::FontRole).value<QFont>());
  m_header->setFixedHeight(static_cast<int>(1.8 * header_metrics.height()));
  m_header_padding->setFixedHeight(m_header->height());
  if(m_table->model()->rowCount() > 0) {
    update_table_height(m_table->model()->rowCount());
  }
}

bool TimeAndSalesTableView::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_table) {
    if(event->type() == QEvent::Paint) {
      m_table->update();
      m_header->update();
    }
  }
  return ScrollArea::eventFilter(watched, event);
}

void TimeAndSalesTableView::resizeEvent(QResizeEvent* event) {
  widget()->resize(width(), widget()->height());
  m_header->setFixedWidth(widget()->width());
}

void TimeAndSalesTableView::showEvent(QShowEvent* event) {
  if(!m_was_shown && m_model != nullptr) {
    m_was_shown = true;
    m_header->resizeSection(0, scale_width(74));
    m_header->resizeSection(1, scale_width(60));
    m_header->resizeSection(2, scale_width(46));
    m_header->resizeSection(3, scale_width(50));
    m_header->resizeSection(4, scale_width(50));
  }
}

void TimeAndSalesTableView::show_loading_widget() {
  m_loading_widget = new SnapshotLoadingWidget(this);
  m_layout->addWidget(m_loading_widget);
  update_table_height(m_table->model()->rowCount());
}

void TimeAndSalesTableView::update_table_height(int num_rows) {
  auto height = (num_rows * m_table->verticalHeader()->defaultSectionSize())
    + m_header->height();
  if(m_loading_widget != nullptr) {
    height += m_loading_widget->height();
  }
  widget()->setFixedHeight(height);
}

void TimeAndSalesTableView::on_end_loading_signal() {
  delete_later(m_transition_widget);
  delete_later(m_loading_widget);
}

void TimeAndSalesTableView::on_header_resize(int index, int old_size,
    int new_size) {
  auto min_width = m_minimum_column_widths[index];
  if(new_size <= min_width) {
    m_header->blockSignals(true);
    m_header->resizeSection(index, min_width);
    m_header->blockSignals(false);
    new_size = min_width;
  }
  m_table->horizontalHeader()->resizeSection(index, new_size);
  m_model->set_column_size_reference(index, new_size);
}

void TimeAndSalesTableView::on_header_move(int logical_index,
    int old_index, int new_index) {
  m_table->horizontalHeader()->moveSection(old_index, new_index);
}

void TimeAndSalesTableView::on_horizontal_slider_value_changed(
    int new_value) {
  if(new_value != 0) {
    m_header->move(widget()->pos().x(), m_header->pos().y());
  } else {
    m_header->move(0, m_header->pos().y());
  }
}

void TimeAndSalesTableView::on_vertical_slider_value_changed(
    int new_value) {
  if(m_model == nullptr) {
    return;
  }
  m_model->set_row_visible(m_table->rowAt(
    widget()->visibleRegion().boundingRect().bottom()));
}

void TimeAndSalesTableView::on_rows_about_to_be_inserted(
    const QModelIndex& parent, int first_index, int last_index) {
  auto num_rows = last_index - first_index + 1;
  update_table_height(m_table->model()->rowCount() + num_rows);
  if(first_index == 0) {
    if(verticalScrollBar()->value() != 0) {
      verticalScrollBar()->setValue(verticalScrollBar()->value() +
        (m_table->verticalHeader()->defaultSectionSize() * num_rows));
    }
  }
}
