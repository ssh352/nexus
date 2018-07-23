#ifndef SPIRE_BLOTTERTASKPROPERTIES_HPP
#define SPIRE_BLOTTERTASKPROPERTIES_HPP
#include <vector>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include "Spire/Blotter/BlotterTaskMonitor.hpp"

namespace Spire {

  /*! \class BlotterTaskProperties
      \brief Stores the user's properties for displaying Tasks in a
             BlotterWindow.
   */
  class BlotterTaskProperties {
    public:

      //! Returns a default set of BlotterTaskProperties.
      static BlotterTaskProperties GetDefault();

      //! Constructs a default BlotterTasksProperties.
      BlotterTaskProperties();

      //! Copies a BlotterTaskProperties.
      /*!
        \param properties The BlotterTaskProperties to copy.
      */
      BlotterTaskProperties(const BlotterTaskProperties& properties);

      ~BlotterTaskProperties();

      //! Assigns a BlotterTaskProperties.
      /*!
        \param properties The BlotterTaskProperties to copy.
        \return <code>*this</code>
      */
      BlotterTaskProperties& operator =(
        const BlotterTaskProperties& properties);

      //! Returns the Task monitors to display.
      const std::vector<BlotterTaskMonitor>& GetMonitors() const;

      //! Adds a Task monitor to display.
      /*!
        \param monitor The Task monitor to add.
      */
      void Add(const BlotterTaskMonitor& monitor);

      //! Removes a Task monitor.
      /*!
        \param name The name of the Task monitor to remove.
      */
      void Remove(const std::string& name);

      //! Replaces one Task monitor with another.
      /*!
        \param index The index of the Task monitor to replace.
        \param monitor The Task monitor to replace at the specified
               <i>index</i>.
      */
      void Replace(int index, const BlotterTaskMonitor& monitor);

    private:
      friend struct Beam::Serialization::DataShuttle;
      std::vector<BlotterTaskMonitor> m_taskMonitors;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void BlotterTaskProperties::Shuttle(Shuttler& shuttle, unsigned int version) {
    shuttle.Shuttle("task_monitors", m_taskMonitors);
  }
}

#endif
