#ifndef NEXUS_PYTHONORDEREXECUTIONSERVICE_HPP
#define NEXUS_PYTHONORDEREXECUTIONSERVICE_HPP
#include "Nexus/Python/Python.hpp"

namespace Nexus {
namespace Python {

  //! Exports the AccountQuery class.
  void ExportAccountQuery();

  //! Exports the ExecutionReport struct.
  void ExportExecutionReport();

  //! Exports the Order class.
  void ExportOrder();

  //! Exports the OrderExecutionClient class.
  void ExportOrderExecutionClient();

  //! Exports the OrderExecutionService namespace.
  void ExportOrderExecutionService();

  //! Exports the OrderExecutionService::Tests namespace.
  void ExportOrderExecutionServiceTestInstance();

  //! Exports the OrderFields struct.
  void ExportOrderFields();

  //! Exports the OrderInfo struct.
  void ExportOrderInfo();

  //! Exports the OrderRecord struct.
  void ExportOrderRecord();

  //! Exports the PrimitiveOrder class.
  void ExportPrimitiveOrder();

  //! Exports standard queries.
  void ExportStandardQueries();
}
}

#endif
