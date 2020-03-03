#ifndef SPIRE_HISTORICAL_ORDER_IMBALANCE_WIDGET_HPP
#define SPIRE_HISTORICAL_ORDER_IMBALANCE_WIDGET_HPP
#include <QWidget>
#include "Nexus/Definitions/OrderImbalance.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicator.hpp"
#include "Spire/Spire/Intervals.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  class HistoricalOrderImbalanceWidget : public QWidget {
    public:

      HistoricalOrderImbalanceWidget(const TimeInterval& interval,
        const std::vector<Nexus::OrderImbalance>& imbalances,
        QWidget* parent = nullptr);

    private:
      DropDownMenu* m_data_dropdown;
      HistoricalOrderImbalanceChartView* m_chart_widget;
  };
}

#endif