#ifndef NEXUS_PYTHONFEEHANDLING_HPP
#define NEXUS_PYTHONFEEHANDLING_HPP
#include "Nexus/Python/Python.hpp"

namespace Nexus {
namespace Python {

  //! Exports FeeHandling classes.
  void ExportFeeHandling();

  //! Exports the PureFeeTable class.
  void ExportPureFeeTable();
}
}

#endif