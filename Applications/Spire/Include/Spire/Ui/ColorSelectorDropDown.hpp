#ifndef SPIRE_COLOR_SELECTOR_DROP_DOWN_HPP
#define SPIRE_COLOR_SELECTOR_DROP_DOWN_HPP
#include <QGridLayout>
#include <QHBoxLayout>
#include <QWidget>
#include "Spire/Ui/ColorSelectorHexInputWidget.hpp"
#include "Spire/Ui/ColorSelectorHueSlider.hpp"
#include "Spire/Ui/ColorSelectorValueSlider.hpp"
#include "Spire/Ui/DropShadow.hpp"
#include "Spire/Ui/RecentColors.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Represents a drop down widget for selecting colors.
  class ColorSelectorDropDown : public QWidget {
    public:

      //! Signal type for color selection.
      /*!
        \param color The selected color.
      */
      using ColorSignal = Signal<void (const QColor& color)>;

      //! Constructs a ColorSelectorDropDown.
      /*!
        \param current_color The current color to display.
        \param parent The parent widget.
      */
      explicit ColorSelectorDropDown(const QColor& current_color,
        QWidget* parent = nullptr);

      //! Sets the current color.
      /*!
        \param color The current color.
      */
      void set_color(const QColor& color);

      //! Connects a slot to the color selection signal.
      boost::signals2::connection connect_color_signal(
        const ColorSignal::slot_type& slot) const;

    protected:
      void childEvent(QChildEvent* event) override;
      bool eventFilter(QObject* watched, QEvent* event) override;
      void hideEvent(QHideEvent* event) override;

    private:
      mutable ColorSignal m_color_signal;
      RecentColors& m_recent_colors;
      boost::signals2::scoped_connection m_recent_colors_connection;
      QColor m_current_color;
      QColor m_original_color;
      QGridLayout* m_basic_colors_layout;
      QHBoxLayout* m_recent_colors_layout;
      ColorSelectorValueSlider* m_color_value_slider;
      ColorSelectorHueSlider* m_color_hue_slider;
      ColorSelectorHexInputWidget* m_hex_input;
      DropShadow* m_drop_shadow;

      void add_basic_color_button(int x, int y, const QColor& color);
      void add_recent_color_button(int index, const QColor& color);
      void update_recent_colors_layout();
      void on_color_button_clicked(const QColor& color);
      void on_color_selected(const QColor& color);
      void on_recent_colors_changed();
  };
}

#endif