#include "Spire/OrderImbalanceIndicator/HistoricalOrderImbalanceChartView.hpp"
#include <QDate>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QWheelEvent>
#include "Nexus/Definitions/Money.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  const auto ZOOM = 110;
  const auto PAN = 100;

  auto BOTTOM_MARGIN() {
    return scale_height(34);
  }

  auto LEFT_MARGIN() {
    return scale_width(36);
  }

  auto RIGHT_MARGIN() {
    return scale_width(18);
  }

  auto CHART_PADDING() {
    return scale_width(13);
  }

  const auto& CURSOR() {
    // TODO: replace with proper cursor file
    static auto cursor = QCursor(QPixmap::fromImage(
      imageFromSvg(":/Icons/chart-cursor.svg", scale(18, 18))));
    return cursor;
  }
}

HistoricalOrderImbalanceChartView::HistoricalOrderImbalanceChartView(
    const TimeInterval& interval,
    const std::vector<Nexus::OrderImbalance>& imbalances, QWidget* parent)
    : QWidget(parent),
      m_imbalances(imbalances),
      m_interval(interval),
      m_label_font("Roboto"),
      m_font_metrics(m_label_font),
      m_is_dragging(false),
      m_dashed_line_pen(QColor("#333333"), scale_width(1), Qt::CustomDashLine),
      m_display_type(DisplayType::REFERENCE_PRICE) {
  setAttribute(Qt::WA_Hover);
  setMouseTracking(true);
  m_dashed_line_pen.setDashPattern({static_cast<double>(scale_width(3)),
    static_cast<double>(scale_width(3))});
  m_label_font.setPixelSize(scale_height(10));
  m_font_metrics = QFontMetrics(m_label_font);
  m_item_delegate = new CustomVariantItemDelegate(this);
  update_points();
}

void HistoricalOrderImbalanceChartView::leaveEvent(QEvent* event) {
  m_cursor_pos = boost::none;
}

void HistoricalOrderImbalanceChartView::mouseMoveEvent(QMouseEvent* event) {
  if(m_imbalances.empty()) {
    return;
  }
  if(QRect(LEFT_MARGIN(), 0, m_chart_size.width(), m_chart_size.height())
      .contains(event->pos())) {
    m_cursor_pos = event->pos();
    setCursor(CURSOR());
  } else {
    m_cursor_pos = boost::none;
    setCursor(Qt::ArrowCursor);
  }
  if(m_is_dragging) {
    auto pixel_delta = static_cast<int>(
      static_cast<double>(event->x() - m_last_mouse_pos.x()) /
      static_cast<double>(m_chart_size.width()) * PAN);
    if(pixel_delta != 0) {
      auto chart_delta = (m_interval.upper() - m_interval.lower()) / 100 *
        pixel_delta;
      if(event->x() < m_last_mouse_pos.x()) {
          if(m_imbalances.back().m_timestamp >= m_interval.upper()) {
            m_interval = {m_interval.lower() - chart_delta,
              m_interval.upper() - chart_delta};
          }
      } else {
        if(m_imbalances.front().m_timestamp <= m_interval.lower()) {
          m_interval = {m_interval.lower() - chart_delta,
            m_interval.upper() - chart_delta};
        }
      }
      m_last_mouse_pos = event->pos();
      update_points();
    }
  }
  update();
}

void HistoricalOrderImbalanceChartView::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    m_is_dragging = true;
    m_last_mouse_pos = event->pos();
  }
}

void HistoricalOrderImbalanceChartView::mouseReleaseEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    m_is_dragging = false;
  }
}

void HistoricalOrderImbalanceChartView::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.fillRect(event->rect(), Qt::white);
  painter.setPen(Qt::black);
  painter.drawLine(LEFT_MARGIN(), 0, LEFT_MARGIN(),
    m_chart_size.height());
  painter.drawLine(LEFT_MARGIN(), m_chart_size.height(),
    width() - RIGHT_MARGIN(), m_chart_size.height());
  auto label_count = min(static_cast<int>(m_chart_points.size()), 5);
  if(m_chart_points.size() == 1) {
    auto point = m_chart_points.front().m_point;
    draw_x_axis_label(painter, point,
      m_chart_points.front().m_imbalance.m_timestamp);
    auto right_point = QPoint(point.x() + m_chart_size.width() -
      (2 * CHART_PADDING()), point.y());
    draw_x_axis_label(painter, right_point,
      m_chart_points.front().m_imbalance.m_timestamp);
  } else {
    for(auto i = 0; i < label_count; ++i) {
      auto index = map_to(i, 0, label_count - 1, 0, m_chart_points.size() - 1);
      auto& point = m_chart_points[index];
      draw_x_axis_label(painter, point.m_point, point.m_imbalance.m_timestamp);
    }
  }
  auto gradient = QLinearGradient(0, m_chart_size.height(), 0, 0);
  gradient.setColorAt(0, QColor("#E2E0FF"));
  gradient.setColorAt(1, Qt::white);
  painter.fillRect(LEFT_MARGIN() + CHART_PADDING(), 0,
    m_chart_size.width() - (2 * CHART_PADDING()),
    m_chart_size.height() - scale_height(1), gradient);
  auto cover = QPolygon();
  cover << QPoint(LEFT_MARGIN() + scale_width(1), 0) << QPoint(width(), 0);
  if(m_chart_points.size() == 1) {
    auto point1 = m_chart_points.front().m_point;
    point1.setX(LEFT_MARGIN() + CHART_PADDING());
    auto point2 = m_chart_points.front().m_point;
    point2.setX(point1.x() + m_chart_size.width() - (2 * CHART_PADDING()));
    draw_line(painter, point1, point2);
    draw_point(painter, point1);
    draw_point(painter, point2);
    auto label_text = scalar_to_string(Scalar(
      m_chart_points.front().m_imbalance.m_size));
    auto mid_text_width = m_font_metrics.horizontalAdvance(label_text);
    painter.drawText(LEFT_MARGIN() - mid_text_width - scale_width(2),
      scale_height(75), label_text);
    if(m_cursor_pos) {
      if(m_cursor_pos->x() < (m_chart_size.width() / 2) + LEFT_MARGIN()) {
        m_crosshair_point = ChartPoint(point1,
          m_chart_points.front().m_imbalance);
      } else {
        m_crosshair_point = ChartPoint(point2,
          m_chart_points.front().m_imbalance);
      }
    }
  } else {
    painter.setPen(QColor("#333333"));
    painter.setFont(m_label_font);
    auto max_text = scalar_to_string(m_maximum_value);
    auto max_text_width = m_font_metrics.horizontalAdvance(max_text);
    painter.drawText(LEFT_MARGIN() - max_text_width - scale_width(2),
      scale_height(8), max_text);
    auto mid_text = scalar_to_string((m_maximum_value - m_minimum_value) / 2 +
      m_minimum_value);
    auto mid_text_width = m_font_metrics.horizontalAdvance(mid_text);
    painter.drawText(LEFT_MARGIN() - mid_text_width - scale_width(2),
      scale_height(75), mid_text);
    auto min_text = scalar_to_string(m_minimum_value);
    auto min_text_width = m_font_metrics.horizontalAdvance(min_text);
    painter.drawText(LEFT_MARGIN() - min_text_width - scale_width(2),
      m_chart_size.height() - scale_height(8), min_text);
    auto snap_size = static_cast<int>((m_chart_size.width() -
      (2 * CHART_PADDING())) / m_chart_points.size() / 2);
    for(auto point = m_chart_points.begin(); point != m_chart_points.end();
        ++point) {
      if(std::next(point) != m_chart_points.end()) {
        draw_line(painter, point->m_point, std::next(point)->m_point);
      }
      auto pos_x = point->m_point.x();
      if(m_cursor_pos) {
        if(pos_x - snap_size < m_cursor_pos->x() &&
            m_cursor_pos->x() < pos_x + snap_size) {
          qDebug() << point->m_imbalance.m_timestamp.date().year();
          m_crosshair_point = ChartPoint(point->m_point, point->m_imbalance);
        }
      }
    }
    // TODO: forces the points to draw on top of lines, but look into
    //       how to avoid looping over this twice.
    if(static_cast<int>(m_chart_points.size()) * scale_width(8) <
        m_chart_size.width()) {
      for(auto& point : m_chart_points) {
        draw_point(painter, point.m_point);
      }
    }
  }
  if(m_cursor_pos && m_crosshair_point) {
    painter.setPen(m_dashed_line_pen);
    painter.drawLine(LEFT_MARGIN(), m_crosshair_point->m_point.y(), width(),
      m_crosshair_point->m_point.y());
    painter.drawLine(m_crosshair_point->m_point.x(), 0,
      m_crosshair_point->m_point.x(), m_chart_size.height());
    draw_x_axis_label(painter, m_crosshair_point->m_point,
      m_crosshair_point->m_imbalance.m_timestamp, QColor("#333333"),
      Qt::white);
    draw_hover_widgets(painter, m_crosshair_point->m_point);
  }
}

void HistoricalOrderImbalanceChartView::resizeEvent(QResizeEvent* event) {
  m_chart_size.setWidth(width() - LEFT_MARGIN() - RIGHT_MARGIN());
  m_chart_size.setHeight(height() - BOTTOM_MARGIN());
  update_points();
}

void HistoricalOrderImbalanceChartView::wheelEvent(QWheelEvent* event) {
  if(m_imbalances.empty()) {
    return;
  }
  auto chart_range = m_interval.upper() - m_interval.lower();
  if(event->angleDelta().y() < 0) {
    if(m_interval.lower() > m_imbalances.front().m_timestamp ||
        m_interval.upper() < m_imbalances.back().m_timestamp) {
      auto zoom = (chart_range - ((chart_range * ZOOM) / 100)) / 2;
      m_interval = {m_interval.lower() + zoom, m_interval.upper() - zoom};
    }
  } else {
    if(m_chart_points.size() > 1) {
      auto zoom = ((chart_range * 100) / ZOOM - chart_range) / 2;
      m_interval = {m_interval.lower() - zoom, m_interval.upper() + zoom};
    }
  }
  update_points();
  update();
}

// TODO: remove point parameter, just use m_crosshair_point
void HistoricalOrderImbalanceChartView::draw_hover_widgets(QPainter& painter,
    const QPoint& point) {
  auto label_pos = [&] {
      if(point.y() > scale_height(32)) {
        return point - QPoint(scale_width(23), scale_height(28));
      }
      return point + QPoint(-scale_width(23), scale_height(10));
    }();
  painter.setPen({Qt::white, 2});
  painter.setBrush(QColor("#36BB55"));
  painter.drawRect(label_pos.x(), label_pos.y(), scale_width(47),
    scale_height(18));
  painter.setFont(m_label_font);
  auto locale = QLocale();
  auto text = m_item_delegate->displayText(scalar_to_string(
    Scalar(m_crosshair_point->m_imbalance.m_size)), QLocale());
  auto text_width = m_font_metrics.horizontalAdvance(text);
  auto text_pos = QPoint(point.x() - (text_width / 2),
    label_pos.y() + scale_height(12));
  painter.drawText(text_pos, text);
  painter.setBrush(Qt::white);
  painter.drawEllipse(point, scale_width(4), scale_width(4));
  painter.setPen(QColor("#36BB55"));
  painter.setBrush(QColor("#36BB55"));
  painter.drawEllipse(point, scale_width(3), scale_width(3));
}

void HistoricalOrderImbalanceChartView::draw_line(QPainter& painter,
    const QPoint& point1, const QPoint& point2) {
  painter.setPen({QColor("#4B23A0"), static_cast<qreal>(scale_width(2))});
  painter.drawLine(point1, point2);
}

void HistoricalOrderImbalanceChartView::draw_point(QPainter& painter,
    const QPoint& point) {
  painter.setPen(Qt::white);
  painter.setBrush(Qt::white);
  painter.drawEllipse(point, scale_width(4), scale_width(4));
  painter.setPen(QColor("#4B23A0"));
  painter.setBrush(QColor("#4b23A0"));
  painter.drawEllipse(point, scale_width(3), scale_width(3));
}

void HistoricalOrderImbalanceChartView::draw_x_axis_label(QPainter& painter,
    const QPoint& point, const boost::posix_time::ptime& timestamp) {
  draw_x_axis_label(painter, point, timestamp, Qt::transparent, Qt::black);
}

void HistoricalOrderImbalanceChartView::draw_x_axis_label(QPainter& painter,
    const QPoint& point, const boost::posix_time::ptime& timestamp,
    const QColor& background_color, const QColor& text_color) {
  painter.setPen(Qt::black);
  painter.drawLine(point.x(), m_chart_size.height(), point.x(),
    m_chart_size.height() + scale_height(2));
  painter.fillRect(point.x() - scale_width(29), m_chart_size.height(),
    scale_width(59), scale_height(34), background_color);
  painter.setPen(text_color);
  painter.setFont(m_label_font);
  auto locale = QLocale();
  auto date_text = locale.toString(QDate(timestamp.date().year(),
    timestamp.date().month(), timestamp.date().day()), "M/d/yyyy");
  auto date_text_width = m_font_metrics.horizontalAdvance(date_text);
  auto date_text_pos = QPoint(point.x() - (date_text_width / 2),
    m_chart_size.height() + scale_height(14));
  painter.drawText(date_text_pos, date_text);
  auto time_text = locale.toString(QTime(
    static_cast<int>(timestamp.time_of_day().hours()),
    static_cast<int>(timestamp.time_of_day().minutes())), "h:mm A");
  auto time_text_width = m_font_metrics.horizontalAdvance(time_text);
  auto time_text_pos = QPoint(point.x() - (time_text_width / 2),
    m_chart_size.height() + scale_height(25));
  painter.drawText(time_text_pos, time_text);
}

QString HistoricalOrderImbalanceChartView::scalar_to_string(
    Scalar value) const {
  if(m_display_type == DisplayType::REFERENCE_PRICE) {
    return m_item_delegate->displayText(QVariant::fromValue<Money>(
      static_cast<Money>(value)), QLocale());
  }
  return m_item_delegate->displayText(QVariant::fromValue<Quantity>(
    static_cast<Quantity>(value)), QLocale());
}

void HistoricalOrderImbalanceChartView::update_points() {
  m_minimum_value = Scalar(std::numeric_limits<Nexus::Quantity>::max());
  m_maximum_value = Scalar(0);
  m_chart_points.clear();
  for(auto& imbalance : m_imbalances) {
    if(m_interval.lower() <= imbalance.m_timestamp &&
        imbalance.m_timestamp <= m_interval.upper()) {
      m_minimum_value = min(m_minimum_value, Scalar(imbalance.m_size));
      m_maximum_value = max(m_maximum_value, Scalar(imbalance.m_size));
    }
  }
  auto chart_drawable_width = m_chart_size.width() - (2 * CHART_PADDING());
  auto lower_index = std::lower_bound(m_imbalances.begin(), m_imbalances.end(),
    m_interval.lower(), [] (const auto& item, const auto& value) {
      return item.m_timestamp < value;
    });
  auto upper_index = std::upper_bound(m_imbalances.begin(), m_imbalances.end(),
    m_interval.upper(), [] (const auto& value, const auto& item) {
      return item.m_timestamp > value;
    });
  auto data_point_count = std::distance(lower_index, upper_index);
  if(data_point_count == 1) {
    m_chart_points.emplace_back(QPoint(LEFT_MARGIN() + CHART_PADDING(),
      m_chart_size.height() / 2), *lower_index);
    return;
  }
  auto start_index = lower_index;
  for(; lower_index != upper_index; ++lower_index) {
    auto index = std::distance(m_imbalances.begin(), lower_index) -
      std::distance(m_imbalances.begin(), start_index);
    auto x = LEFT_MARGIN() + CHART_PADDING() +
      static_cast<int>(static_cast<double>(chart_drawable_width) /
      static_cast<double>(data_point_count - 1) * static_cast<double>(index));
    auto y = map_to(lower_index->m_size,
      static_cast<Nexus::Quantity>(m_maximum_value),
      static_cast<Nexus::Quantity>(m_minimum_value),
      0 + scale_height(5), m_chart_size.height() - scale_height(8));
    m_chart_points.emplace_back(QPoint(x, y), *lower_index);
  }
  update();
}