#include "spire/ui/item_padding_delegate.hpp"
#include <QPainter>

using namespace spire;

item_padding_delegate::item_padding_delegate(int padding,
  QStyledItemDelegate* delegate, QObject* parent)
    : QStyledItemDelegate(parent),
      m_padding(padding),
      m_item_delegate(delegate) {
  m_item_delegate->setParent(this);
}

QString item_padding_delegate::displayText(const QVariant& value,
    const QLocale& locale) const {
  return m_item_delegate->displayText(value, locale);
}

void item_padding_delegate::paint(QPainter* painter,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  painter->save();
  painter->fillRect(option.rect,
    index.data(Qt::BackgroundRole).value<QColor>());
  painter->translate(QPoint(m_padding, 0));
  m_item_delegate->paint(painter, option, index);
  painter->restore();
}

QSize item_padding_delegate::sizeHint(const QStyleOptionViewItem& option,
    const QModelIndex& index) const {
  return m_item_delegate->sizeHint(option, index);
}
