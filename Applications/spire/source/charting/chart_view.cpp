#include "spire/charting/chart_view.hpp"
#include <locale>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <QFontMetrics>
#include <QPainter>
#include <QPaintEvent>
#include "Nexus/Definitions/Money.hpp"
#include "spire/charting/chart_model.hpp"
#include "spire/spire/dimensions.hpp"

using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  const auto& CROSSHAIR_IMAGE() {
    static const auto image = imageFromSvg(":/icons/chart-cursor.svg",
      scale(20, 20));
    return image;
  }

  QVariant to_variant(ChartValue::Type type, ChartValue value) {
    if(type == ChartValue::Type::DURATION) {
      return QVariant::fromValue(static_cast<time_duration>(value));
    } else if(type == ChartValue::Type::MONEY) {
      return QVariant::fromValue(static_cast<Money>(value));
    } else if(type == ChartValue::Type::QUANTITY) {
      return QVariant::fromValue(static_cast<Quantity>(value));
    } else if(type == ChartValue::Type::TIMESTAMP) {
      return QVariant::fromValue(static_cast<ptime>(value));
    }
    return QVariant();
  }

  template<typename T, typename U>
  U map_to(T value, T a, T b, U c, U d) {
    return ((value - a) / (b - a)) * (d - c) + c;
  }

  ChartValue calculate_step(ChartValue::Type value_type,
      ChartValue range) {
    if(value_type == ChartValue::Type::MONEY) {
      // TODO: compiler bug workaround
      return ChartValue(Money::FromValue("0.1").get());
    } else if(value_type == ChartValue::Type::TIMESTAMP) {
      return ChartValue(minutes(10));
    }
    return ChartValue();
  }
}

ChartView::ChartView(ChartModel& model, QWidget* parent)
    : QWidget(parent),
      m_model(&model),
      m_x_origin(0),
      m_label_font("Roboto"),
      m_font_metrics(QFont()),
      m_item_delegate(new CustomVariantItemDelegate(this)),
      m_dashed_line_pen(QColor("#E5E5E5"), scale_width(1), Qt::CustomDashLine),
      m_label_text_color(QColor("#25212E")),
      m_is_auto_scaled(true) {
  setFocusPolicy(Qt::NoFocus);
  setMouseTracking(true);
  setAttribute(Qt::WA_Hover);
  m_label_font.setPixelSize(scale_height(10));
  m_font_metrics = QFontMetrics(m_label_font);
  setCursor(Qt::BlankCursor);
  m_dashed_line_pen.setDashPattern({static_cast<double>(scale_width(3)),
    static_cast<double>(scale_width(3))});
}

ChartPoint ChartView::convert_pixels_to_chart(const QPoint& point) const {
  return ChartPoint(
    map_to(static_cast<double>(point.x()), 0.0,
      static_cast<double>(m_x_origin), m_top_left.m_x, m_bottom_right.m_x),
    map_to(static_cast<double>(point.y()), static_cast<double>(m_y_origin),
      0.0, m_bottom_right.m_y, m_top_left.m_y));
}

QPoint ChartView::convert_chart_to_pixels(const ChartPoint& point) const {
  return QPoint(
    map_to(point.m_x, m_top_left.m_x, m_bottom_right.m_x, 0.0,
      static_cast<double>(m_x_origin)),
    map_to(point.m_y, m_bottom_right.m_y, m_top_left.m_y,
      static_cast<double>(m_y_origin), 0.0));
}

void ChartView::set_crosshair(const ChartPoint& position) {
  set_crosshair(convert_chart_to_pixels(position));
}

void ChartView::set_crosshair(const QPoint& position) {
  m_crosshair_pos = position;
  if(m_crosshair_pos.value().x() <= m_x_origin &&
      m_crosshair_pos.value().y() <= m_y_origin) {
    setCursor(Qt::BlankCursor);
  } else {
    setCursor(Qt::ArrowCursor);
  }
  update();
}

void ChartView::reset_crosshair() {
  m_crosshair_pos.reset();
}

std::tuple<ChartPoint, ChartPoint> ChartView::get_region() const {
  return {m_top_left, m_bottom_right};
}

void ChartView::set_region(const ChartPoint& top_left,
    const ChartPoint& bottom_right) {
  if(std::tie(m_top_left.m_x, m_top_left.m_y) ==
      std::tie(top_left.m_x, top_left.m_y) &&
      std::tie(m_bottom_right.m_x, m_bottom_right.m_y) ==
      std::tie(bottom_right.m_x, bottom_right.m_y)) {
    return;
  }
  m_top_left = top_left;
  m_bottom_right = bottom_right;
  m_x_range = m_bottom_right.m_x - m_top_left.m_x;
  m_x_axis_step = calculate_step(m_model->get_x_axis_type(), m_x_range);
  m_y_range = m_top_left.m_y - m_bottom_right.m_y;
  m_y_axis_step = calculate_step(m_model->get_y_axis_type(), m_y_range);
  update_origins();
  m_candlestick_promise = m_model->load(m_top_left.m_x,
    m_bottom_right.m_x);
  m_candlestick_promise.then([=] (auto result) {
    m_candlesticks = std::move(result.Get());
    if(m_is_auto_scaled) {
      update_auto_scale();
    } else {
      update();
    }
  });
  update();
}

void ChartView::set_auto_scale(bool auto_scale) {
  m_is_auto_scaled = auto_scale;
  if(m_is_auto_scaled) {
    update_auto_scale();
  }
}

void ChartView::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  painter.setFont(m_label_font);
  painter.setPen(Qt::white);
  painter.drawLine(m_x_origin, 0, m_x_origin, m_y_origin);
  painter.drawLine(0, m_y_origin, m_x_origin, m_y_origin);
  if(m_x_range <= ChartValue(0) || m_y_range <= ChartValue(0)) {
    return;
  }
  for(auto y : m_y_axis_values) {
    auto y_pos = map_to(y, m_bottom_right.m_y,
    m_top_left.m_y, m_y_origin, 0);
    painter.setPen("#3A3348");
    painter.drawLine(0, y_pos, m_x_origin, y_pos);
    painter.setPen(Qt::white);
    painter.drawLine(m_x_origin, y_pos, m_x_origin + scale_width(2), y_pos);
    painter.drawText(m_x_origin + scale_width(3),
      y_pos + (m_font_metrics.height() / 3), m_item_delegate->displayText(
      to_variant(m_model->get_y_axis_type(), y), QLocale()));
  }
  for(auto x : m_x_axis_values) {
    auto x_pos = map_to(x, m_top_left.m_x, m_bottom_right.m_x, 0,
      m_x_origin);
    painter.setPen("#3A3348");
    painter.drawLine(x_pos, 0, x_pos, m_y_origin);
    painter.setPen(Qt::white);
    painter.drawLine(x_pos, m_y_origin, x_pos, m_y_origin + scale_height(2));
    painter.drawText(x_pos - m_x_axis_text_width / 2,
      m_y_origin + m_font_metrics.height() + scale_height(2),
      m_item_delegate->displayText(to_variant(m_model->get_x_axis_type(), x),
      QLocale()));
  }
  painter.setPen(Qt::white);
  for(auto& candlestick : m_candlesticks) {
    auto open = convert_chart_to_pixels({candlestick.GetStart(),
      candlestick.GetOpen()});
    auto close = convert_chart_to_pixels({candlestick.GetEnd(),
      candlestick.GetClose()});
    auto high = map_to(candlestick.GetHigh(), m_bottom_right.m_y,
      m_top_left.m_y, m_y_origin, 0);
    auto low = map_to(candlestick.GetLow(), m_bottom_right.m_y,
      m_top_left.m_y, m_y_origin, 0);
    auto wick_pos = open.x() + (close.x() - open.x()) / 2;
    if(wick_pos < m_x_origin && high < m_y_origin) {
      painter.fillRect(QRect(QPoint(wick_pos, high),
        QPoint(wick_pos, std::min(low, m_y_origin - 1))), QColor("#C8C8C8"));
    }
    if(open.y() > close.y()) {
      if(close.y() < m_y_origin) {
        painter.fillRect(QRect(QPoint(open.x(), close.y()),
          QPoint(std::min(close.x() - 1, m_x_origin - 1),
          std::min(open.y(), m_y_origin - 1))), QColor("#8AF5C0"));
        painter.fillRect(QRect(QPoint(open.x() + 1, close.y() + 1),
          QPoint(std::min(close.x() - 2, m_x_origin - 1),
          std::min(open.y() - 1, m_y_origin - 1))), QColor("#1FD37A"));
      }
    } else {
      if(open.y() < m_y_origin) {
        painter.fillRect(QRect(open,
          QPoint(std::min(close.x() - 1, m_x_origin - 1),
          std::min(close.y(), m_y_origin - 1))), QColor("#FFA7A0"));
        painter.fillRect(QRect(QPoint(open.x() + 1, open.y() + 1),
          QPoint(std::min(close.x() - 2, m_x_origin - 1),
          std::min(close.y() - 1, m_y_origin - 1))), QColor("#EF5357"));
      }
    }
  }
  if(m_crosshair_pos && m_crosshair_pos.value().x() <= m_x_origin &&
      m_crosshair_pos.value().y() <= m_y_origin) {
    painter.setPen(m_dashed_line_pen);
    painter.drawLine(m_crosshair_pos.value().x(), 0,
      m_crosshair_pos.value().x(), m_y_origin);
    painter.drawLine(0, m_crosshair_pos.value().y(), m_x_origin,
      m_crosshair_pos.value().y());
    painter.drawImage(m_crosshair_pos.value().x() -
      (CROSSHAIR_IMAGE().width() / 2), m_crosshair_pos.value().y() -
      (CROSSHAIR_IMAGE().height() / 2), CROSSHAIR_IMAGE());
    auto crosshair_value = convert_pixels_to_chart(m_crosshair_pos.value());
    auto x_label = m_item_delegate->displayText(to_variant(
      m_model->get_x_axis_type(), crosshair_value.m_x), QLocale());
    auto x_label_width = m_font_metrics.width(x_label);
    painter.fillRect(m_crosshair_pos.value().x() - (x_label_width / 2) -
      scale_width(5), m_y_origin, x_label_width + scale_width(10),
      scale_height(21), Qt::white);
    painter.fillRect(m_crosshair_pos.value().x(), m_y_origin, scale_width(1),
      scale_height(3), Qt::black);
    auto text_width = m_font_metrics.width(x_label);
    painter.setPen(m_label_text_color);
    painter.drawText(m_crosshair_pos.value().x() - text_width / 2,
      m_y_origin + m_font_metrics.height() + scale_height(2), x_label);
    painter.fillRect(m_x_origin,
      m_crosshair_pos.value().y() - (scale_height(15) / 2),
      width() - m_x_origin, scale_height(15), Qt::white);
    painter.fillRect(m_x_origin, m_crosshair_pos.value().y(), scale_width(3),
      scale_height(1), Qt::black);
    auto y_label = m_item_delegate->displayText(to_variant(
      m_model->get_y_axis_type(), crosshair_value.m_y), QLocale());
    painter.setPen(m_label_text_color);
    painter.drawText(m_x_origin + scale_width(3), m_crosshair_pos.value().y() +
      (m_font_metrics.height() / 3), y_label);
  }
}

void ChartView::resizeEvent(QResizeEvent* event) {
  update_origins();
  QWidget::resizeEvent(event);
}

void ChartView::showEvent(QShowEvent* event) {
  if(m_top_left.m_x == ChartValue() && m_top_left.m_y == ChartValue() &&
      m_bottom_right.m_x == ChartValue() &&
      m_bottom_right.m_y == ChartValue()) {
    auto current_time = boost::posix_time::second_clock::local_time();
    auto bottom_right = ChartPoint(ChartValue(current_time),
      ChartValue(Nexus::Money(0)));
    auto top_left = ChartPoint(
      ChartValue(current_time - boost::posix_time::hours(1)),
      ChartValue(Nexus::Money(1)));
    set_region(top_left, bottom_right);
  }
  QWidget::showEvent(event);
}

void ChartView::update_auto_scale() {
  if(m_candlesticks.empty()) {
    return;
  }
  auto auto_scale_top = m_candlesticks.front().GetHigh();
  auto auto_scale_bottom = m_candlesticks.front().GetLow();
  for(auto& candle : m_candlesticks) {
    auto_scale_top = std::max(auto_scale_top, candle.GetHigh());
    auto_scale_bottom = std::min(auto_scale_bottom, candle.GetLow());
  }
  set_region({m_top_left.m_x, auto_scale_top},
    {m_bottom_right.m_x, auto_scale_bottom});
}

void ChartView::update_origins() {
  auto x_value = m_top_left.m_x - (m_top_left.m_x % m_x_axis_step) +
    m_x_axis_step;
  m_x_axis_values.clear();
  m_x_axis_text_width = 0;
  while(x_value <= m_bottom_right.m_x) {
    m_x_axis_values.push_back(x_value);
    auto text_width = m_font_metrics.width(m_item_delegate->displayText(
      to_variant(m_model->get_x_axis_type(), x_value), QLocale()));
    m_x_axis_text_width = std::max(m_x_axis_text_width, text_width);
    x_value += m_x_axis_step;
  }
  auto y_value = m_bottom_right.m_y - (m_bottom_right.m_y % m_y_axis_step) +
    m_y_axis_step;
  m_y_axis_values.clear();
  auto old_x_origin = m_x_origin;
  m_x_origin = INT_MAX;
  auto top_label = m_top_left.m_y - (m_top_left.m_y % m_y_axis_step);
  while(y_value <= top_label) {
    m_y_axis_values.push_back(y_value);
    auto origin = width() - (m_font_metrics.width("M") * (
      m_item_delegate->displayText(to_variant(m_model->get_y_axis_type(),
      y_value), QLocale()).length()) - scale_width(4));
    m_x_origin = std::min(m_x_origin, origin);
    y_value += m_y_axis_step;
  }
  if(m_x_origin == INT_MAX) {
    m_x_origin = old_x_origin;
  }
  m_y_origin = height() - (m_font_metrics.height() + scale_height(9));
}