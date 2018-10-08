#ifndef SPIRE_WINDOWSETTINGS_HPP
#define SPIRE_WINDOWSETTINGS_HPP
#include <memory>
#include <string>
#include <vector>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Pointers/Ref.hpp>
#include "Spire/Spire/Spire.hpp"
#include "Spire/UI/UI.hpp"

class QWidget;

namespace Spire {
namespace UI {

  /*! \class WindowSettings
      \brief Stores the information needed to re-open a window.
   */
  class WindowSettings {
    public:

      //! Loads a profile's window settings.
      /*!
        \param userProfile The user's profile.
      */
      static std::vector<std::unique_ptr<WindowSettings>> Load(
        const UserProfile& userProfile);

      //! Saves a profile's window settings.
      /*!
        \param userProfile The user's profile.
      */
      static void Save(const UserProfile& userProfile);

      virtual ~WindowSettings();

      //! Returns the name of the window.
      virtual std::string GetName() const;

      //! Re-opens the window.
      /*!
        \param userProfile The user's profile.
      */
      virtual QWidget* Reopen(Beam::Ref<UserProfile> userProfile) const = 0;

      //! Applies the settings to an existing window/widget.
      /*!
        \param userProfile The user's profile.
        \param widget The widget to apply the settings to.
      */
      virtual void Apply(Beam::Ref<UserProfile> userProfile,
        Beam::Out<QWidget> widget) const = 0;
  };
}
}

#endif
