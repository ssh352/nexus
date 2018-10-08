#ifndef SPIRE_CHARTWINDOW_HPP
#define SPIRE_CHARTWINDOW_HPP
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/TaskQueue.hpp>
#include <boost/signals2/connection.hpp>
#include <QMainWindow>
#include <QTimer>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Charting/Charting.hpp"
#include "Spire/Charting/ChartInteractions.hpp"
#include "Spire/Charting/ChartPlotController.hpp"
#include "Spire/Charting/ChartValue.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/UI/PersistentWindow.hpp"
#include "Spire/UI/SecurityContext.hpp"
#include "Spire/UI/SecurityViewStack.hpp"
#include "Spire/UI/WindowSettings.hpp"
#include "Spire/UI/UI.hpp"

class QMenu;
class Ui_ChartWindow;

namespace Spire {

  /*! \class ChartWindow
      \brief Displays a chart.
   */
  class ChartWindow : public QMainWindow, public UI::PersistentWindow,
      public UI::SecurityContext {
    public:

      //! Constructs a ChartWindow.
      /*!
        \param userProfile The user's profile.
        \param identifier The SecurityContext's identifier.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      ChartWindow(Beam::Ref<UserProfile> userProfile,
        const std::string& identifier = "", QWidget* parent = nullptr,
        Qt::WindowFlags flags = 0);

      virtual ~ChartWindow();

      //! Returns the ChartInteractionMode.
      ChartInteractionMode GetInteractionMode() const;

      //! Sets the interaction mode.
      void SetInteractionMode(ChartInteractionMode interactionMode);

      //! Returns <code>true</code> iff auto-scale is enabled.
      bool IsAutoScaleEnabled() const;

      //! Sets whether auto-scale is enabled.
      void SetAutoScale(bool autoScale);

      //! Returns <code>true</code> iff the grid is locked.
      bool IsLockGridEnabled() const;

      //! Sets whether the grid is locked.
      void SetLockGrid(bool lockGrid);

      //! Sets the Security to display.
      /*!
        \param security The Security to display.
      */
      void DisplaySecurity(const Nexus::Security& security);

      virtual std::unique_ptr<UI::WindowSettings> GetWindowSettings() const;

    protected:
      virtual void showEvent(QShowEvent* event);
      virtual void closeEvent(QCloseEvent* event);
      virtual void keyPressEvent(QKeyEvent* event);
      virtual void HandleLink(SecurityContext& context);
      virtual void HandleUnlink();

    private:
      friend class ChartWindowSettings;
      std::unique_ptr<Ui_ChartWindow> m_ui;
      QMenu* m_linkMenu;
      ChartIntervalComboBox* m_intervalComboBox;
      UserProfile* m_userProfile;
      ChartInteractionMode m_interactionMode;
      Beam::DelayPtr<ChartPlotController> m_controller;
      Nexus::Security m_security;
      std::string m_linkIdentifier;
      UI::SecurityViewStack m_securityViewStack;
      ChartValue m_xPan;
      ChartValue m_yPan;
      QTimer m_updateTimer;
      Beam::TaskQueue m_slotHandler;
      boost::signals2::scoped_connection m_linkConnection;
      boost::signals2::scoped_connection m_verticalSliderConnection;
      boost::signals2::scoped_connection m_horizontalSliderConnection;
      boost::signals2::scoped_connection m_intervalChangedConnection;
      boost::signals2::scoped_connection m_beginPanConnection;
      boost::signals2::scoped_connection m_endPanConnection;

      void AdjustSlider(int previousMinimum, int previousMaximum, int minimum,
        int maximum, UI::ScalableScrollBar* scrollBar);
      void UpdateInteractionMode();
      void OnUpdateTimer();
      void OnVerticalSliderChanged(int previousMinimum, int previousMaximum,
        int minimum, int maximum);
      void OnHorizontalSliderChanged(int previousMinimum, int previousMaximum,
        int minimum, int maximum);
      void OnIntervalChanged(const std::shared_ptr<NativeType>& type,
        ChartValue value);
      void OnBeginPan();
      void OnEndPan();
      void OnPanActionToggled(bool toggled);
      void OnZoomActionToggled(bool toggled);
      void OnAutoScaleActionToggled(bool toggled);
      void OnLockGridActionToggled(bool toggled);
      void OnLinkMenuActionTriggered(bool triggered);
      void OnLinkActionTriggered(QAction* action);
  };
}

#endif
