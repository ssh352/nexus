#ifndef NEXUS_PYTHONBACKTESTER_HPP
#define NEXUS_PYTHONBACKTESTER_HPP
#include "Nexus/Python/Python.hpp"

namespace Nexus {
namespace Python {

  //! Exports all of the backtester classes.
  void ExportBacktester();

  //! Exports the BacktesterEnvironment class.
  void ExportBacktesterEnvironment();

  //! Exports the BacktesterServiceClients class.
  void ExportBacktesterServiceClients();
}
}

#endif