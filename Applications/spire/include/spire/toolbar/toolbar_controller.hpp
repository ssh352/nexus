#ifndef SPIRE_TOOLBAR_CONTROLLER_HPP
#define SPIRE_TOOLBAR_CONTROLLER_HPP
#include <memory>
#include <boost/noncopyable.hpp>
#include "Nexus/ServiceClients/ServiceClients.hpp"
#include "spire/toolbar/toolbar.hpp"

namespace spire {

  //! \brief Launches the main application windows.
  class toolbar_controller : private boost::noncopyable {
    public:

      //! Signals that this toolbar and all its windows have closed.
      using closed_signal = signal<void ()>;

      //! Constructs the toolbar controller.
      /*!
        \param service_clients The service clients logged into Spire.
      */
      toolbar_controller(Nexus::VirtualServiceClients& service_clients);

      ~toolbar_controller();

      //! Begins displaying the toolbar window.
      void open();

      //! Connects a slot to the closed signal.
      boost::signals2::connection connect_closed_signal(
        const closed_signal::slot_type& slot) const;

    private:
      mutable closed_signal m_closed_signal;
      Nexus::VirtualServiceClients* m_service_clients;
      std::unique_ptr<toolbar_window> m_toolbar_window;

      void on_closed();
  };
}

#endif