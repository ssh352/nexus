#include "spire/ui/check_box.hpp"
#include <QFocusEvent>
#include <QMouseEvent>

using namespace spire;

check_box::check_box(const QString& text, QWidget* parent)
  : QCheckBox(text, parent),
    m_last_focus_reason(Qt::MouseFocusReason) {}

void check_box::set_stylesheet(const QString& text_style,
    const QString& indicator_style, const QString& checked_style,
    const QString& hover_style, const QString& focused_style) {
  m_text_style = QString("QCheckBox { %1 }").arg(text_style);
  m_indicator_style = QString("QCheckBox::indicator { %1 }").
    arg(indicator_style);
  m_checked_style = QString("QCheckBox::indicator:checked { %1 }")
    .arg(checked_style);
  m_hover_style = QString("QCheckBox::indicator:hover { %1 }")
    .arg(hover_style);
  m_focused_style = QString("QCheckBox::indicator { %1 }")
    .arg(focused_style);
  if(m_last_focus_reason == Qt::MouseFocusReason) {
    set_hover_stylesheet();
  } else {
    set_focused_stylesheet();
  }
}

void check_box::focusInEvent(QFocusEvent* event)  {
  if(event->reason() == Qt::ActiveWindowFocusReason) {
    if(m_last_focus_reason == Qt::MouseFocusReason) {
      set_hover_stylesheet();
    } else {
      set_focused_stylesheet();
    }
    return;
  } else if(event->reason() != Qt::MouseFocusReason) {
    set_focused_stylesheet();
  } else {
    set_hover_stylesheet();
  }
  m_last_focus_reason = event->reason();
}

void check_box::focusOutEvent(QFocusEvent* event) {
  set_hover_stylesheet();
}

void check_box::mouseReleaseEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    set_hover_stylesheet();
    m_last_focus_reason = Qt::MouseFocusReason;
  }
  QCheckBox::mouseReleaseEvent(event);
}

void check_box::set_hover_stylesheet() {
  setStyleSheet(m_text_style + m_indicator_style + m_checked_style +
    m_hover_style);
}

void check_box::set_focused_stylesheet() {
  setStyleSheet(m_text_style + m_indicator_style + m_checked_style +
    m_focused_style);
}