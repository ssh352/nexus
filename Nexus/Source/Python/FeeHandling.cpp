#include "Nexus/Python/FeeHandling.hpp"
#include <Beam/Python/Beam.hpp>
#include <boost/lexical_cast.hpp>
#include "Nexus/FeeHandling/AmexFeeTable.hpp"
#include "Nexus/FeeHandling/ArcaFeeTable.hpp"
#include "Nexus/FeeHandling/AsxtFeeTable.hpp"
#include "Nexus/FeeHandling/BatsFeeTable.hpp"
#include "Nexus/FeeHandling/BatyFeeTable.hpp"
#include "Nexus/FeeHandling/ChicFeeTable.hpp"
#include "Nexus/FeeHandling/ConsolidatedTmxFeeTable.hpp"
#include "Nexus/FeeHandling/ConsolidatedUsFeeTable.hpp"
#include "Nexus/FeeHandling/CseFeeTable.hpp"
#include "Nexus/FeeHandling/EdgaFeeTable.hpp"
#include "Nexus/FeeHandling/EdgxFeeTable.hpp"
#include "Nexus/FeeHandling/HkexFeeTable.hpp"
#include "Nexus/FeeHandling/JpxFeeTable.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/FeeHandling/LynxFeeTable.hpp"
#include "Nexus/FeeHandling/MatnFeeTable.hpp"
#include "Nexus/FeeHandling/NeoeFeeTable.hpp"
#include "Nexus/FeeHandling/NexFeeTable.hpp"
#include "Nexus/FeeHandling/NsdqFeeTable.hpp"
#include "Nexus/FeeHandling/NyseFeeTable.hpp"
#include "Nexus/FeeHandling/OmgaFeeTable.hpp"
#include "Nexus/FeeHandling/PureFeeTable.hpp"
#include "Nexus/FeeHandling/TsxFeeTable.hpp"
#include "Nexus/FeeHandling/XatsFeeTable.hpp"
#include "Nexus/FeeHandling/Xcx2FeeTable.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Python;
using namespace pybind11;

void Nexus::Python::ExportAmexFeeTable(pybind11::module& module) {
  auto outer = class_<AmexFeeTable>(module, "AmexFeeTable")
    .def_readwrite("fee_table", &AmexFeeTable::m_feeTable)
    .def_readwrite("subdollar_table", &AmexFeeTable::m_subdollarTable);
  enum_<AmexFeeTable::Type>(outer, "Type")
    .value("ACTIVE", AmexFeeTable::Type::ACTIVE)
    .value("PASSIVE", AmexFeeTable::Type::PASSIVE)
    .value("HIDDEN_ACTIVE", AmexFeeTable::Type::HIDDEN_ACTIVE)
    .value("HIDDEN_PASSIVE", AmexFeeTable::Type::HIDDEN_PASSIVE)
    .value("AT_THE_OPEN", AmexFeeTable::Type::AT_THE_OPEN)
    .value("AT_THE_CLOSE", AmexFeeTable::Type::AT_THE_CLOSE)
    .value("ROUTED", AmexFeeTable::Type::ROUTED);
  module.def("parse_amex_fee_table", &ParseAmexFeeTable);
  module.def("lookup_fee", static_cast<Money (*)(
    const AmexFeeTable&, AmexFeeTable::Type)>(&LookupFee));
  module.def("lookup_subdollar_fee", static_cast<rational<int> (*)(
    const AmexFeeTable&, AmexFeeTable::Type)>(&LookupSubdollarFee));
  module.def("is_amex_hidden_liquidity_provider",
    &IsAmexHiddenLiquidityProvider);
  module.def("calculate_fee", static_cast<Money (*)(const AmexFeeTable&,
    const OrderFields&, const ExecutionReport&)>(&CalculateFee));
}

void Nexus::Python::ExportArcaFeeTable(pybind11::module& module) {
  auto outer = class_<ArcaFeeTable>(module, "ArcaFeeTable")
    .def_readwrite("fee_table", &ArcaFeeTable::m_feeTable)
    .def_readwrite("subdollar_table", &ArcaFeeTable::m_subdollarTable)
    .def_readwrite("routed_fee", &ArcaFeeTable::m_routedFee)
    .def_readwrite("auction_fee", &ArcaFeeTable::m_auctionFee);
  enum_<ArcaFeeTable::Category>(outer, "Category")
    .value("DEFAULT", ArcaFeeTable::Category::DEFAULT)
    .value("ROUTED", ArcaFeeTable::Category::ROUTED)
    .value("AUCTION", ArcaFeeTable::Category::AUCTION);
  enum_<ArcaFeeTable::Tape>(outer, "Tape")
    .value("A", ArcaFeeTable::Tape::A)
    .value("B", ArcaFeeTable::Tape::B)
    .value("C", ArcaFeeTable::Tape::C);
  enum_<ArcaFeeTable::Type>(outer, "Type")
    .value("ACTIVE", ArcaFeeTable::Type::ACTIVE)
    .value("PASSIVE", ArcaFeeTable::Type::PASSIVE)
    .value("HIDDEN_ACTIVE", ArcaFeeTable::Type::HIDDEN_ACTIVE)
    .value("HIDDEN_PASSIVE", ArcaFeeTable::Type::HIDDEN_PASSIVE)
    .value("AT_THE_OPEN", ArcaFeeTable::Type::AT_THE_OPEN)
    .value("AT_THE_CLOSE", ArcaFeeTable::Type::AT_THE_CLOSE);
  enum_<ArcaFeeTable::SubdollarType>(outer, "SubdollarType")
    .value("ROUTED", ArcaFeeTable::SubdollarType::ROUTED)
    .value("AUCTION", ArcaFeeTable::SubdollarType::AUCTION)
    .value("ACTIVE", ArcaFeeTable::SubdollarType::ACTIVE)
    .value("PASSIVE", ArcaFeeTable::SubdollarType::PASSIVE);
  module.def("parse_arca_fee_table", &ParseArcaFeeTable);
  module.def("lookup_fee", static_cast<Money (*)(const ArcaFeeTable&,
    ArcaFeeTable::Tape, ArcaFeeTable::Type)>(&LookupFee));
  module.def("lookup_fee", static_cast<rational<int> (*)(const ArcaFeeTable&,
    ArcaFeeTable::SubdollarType)>(&LookupFee));
  module.def("is_arca_hidden_liquidity_provider",
    &IsArcaHiddenLiquidityProvider);
  module.def("calculate_fee", static_cast<Money (*)(const ArcaFeeTable&,
    const OrderFields&, const ExecutionReport&)>(&CalculateFee));
}

void Nexus::Python::ExportAsxtFeeTable(pybind11::module& module) {
  auto outer = class_<AsxtFeeTable>(module, "AsxtFeeTable")
    .def(init())
    .def(init<const AsxtFeeTable&>())
    .def_readwrite("spire_fee", &AsxtFeeTable::m_spireFee)
    .def_readwrite("clearing_rate_table", &AsxtFeeTable::m_clearingRateTable)
    .def_readwrite("trade_rate", &AsxtFeeTable::m_tradeRate)
    .def_readwrite("gst_rate", &AsxtFeeTable::m_gstRate)
    .def_readwrite("trade_fee_cap", &AsxtFeeTable::m_tradeFeeCap);
  enum_<AsxtFeeTable::PriceClass>(outer, "PriceClass")
    .value("NONE", AsxtFeeTable::PriceClass::NONE)
    .value("TIER_ONE", AsxtFeeTable::PriceClass::TIER_ONE)
    .value("TIER_TWO", AsxtFeeTable::PriceClass::TIER_TWO)
    .value("TIER_THREE", AsxtFeeTable::PriceClass::TIER_THREE);
  enum_<AsxtFeeTable::OrderTypeClass>(outer, "OrderTypeClass")
    .value("NONE", AsxtFeeTable::OrderTypeClass::NONE)
    .value("REGULAR", AsxtFeeTable::OrderTypeClass::REGULAR)
    .value("PEGGED", AsxtFeeTable::OrderTypeClass::PEGGED);
  module.def("parse_asx_fee_table", &ParseAsxFeeTable);
  module.def("lookup_clearing_fee", &LookupClearingFee);
  module.def("lookup_price_class", &LookupPriceClass);
  module.def("lookup_order_type_class", &LookupOrderTypeClass);
  module.def("calculate_fee", static_cast<ExecutionReport (*)(
    const AsxtFeeTable&, const OrderFields&, const ExecutionReport&)>(
    &CalculateFee));
}

void Nexus::Python::ExportBatsFeeTable(pybind11::module& module) {
  class_<BatsFeeTable>(module, "BatsFeeTable")
    .def(init())
    .def(init<const BatsFeeTable&>())
    .def_readwrite("fee_table", &BatsFeeTable::m_feeTable)
    .def_readwrite("default_flag", &BatsFeeTable::m_defaultFlag);
  module.def("parse_bats_fee_table", &ParseBatsFeeTable);
  module.def("lookup_fee", static_cast<rational<int> (*)(const BatsFeeTable&,
    const std::string&)>(&LookupFee));
  module.def("calculate_fee", static_cast<Money (*)(const BatsFeeTable&,
    const ExecutionReport&)>(&CalculateFee));
}

void Nexus::Python::ExportBatyFeeTable(pybind11::module& module) {
  class_<BatyFeeTable>(module, "BatyFeeTable")
    .def(init())
    .def(init<const BatyFeeTable&>())
    .def_readwrite("fee_table", &BatyFeeTable::m_feeTable)
    .def_readwrite("default_flag", &BatyFeeTable::m_defaultFlag);
  module.def("parse_baty_fee_table", &ParseBatyFeeTable);
  module.def("lookup_fee", static_cast<rational<int> (*)(const BatyFeeTable&,
    const std::string&)>(&LookupFee));
  module.def("calculate_fee", static_cast<Money (*)(const BatyFeeTable&,
    const ExecutionReport&)>(&CalculateFee));
}

void Nexus::Python::ExportChicFeeTable(pybind11::module& module) {
  auto outer = class_<ChicFeeTable>(module, "ChicFeeTable")
    .def(init())
    .def(init<const ChicFeeTable&>())
    .def_readwrite("security_table", &ChicFeeTable::m_securityTable);
  enum_<ChicFeeTable::Classification>(outer, "Classification")
    .value("NONE", ChicFeeTable::Classification::NONE)
    .value("INTERLISTED", ChicFeeTable::Classification::INTERLISTED)
    .value("NON_INTERLISTED", ChicFeeTable::Classification::NON_INTERLISTED)
    .value("ETF", ChicFeeTable::Classification::ETF)
    .value("SUBDOLLAR", ChicFeeTable::Classification::SUBDOLLAR)
    .value("SUBDIME", ChicFeeTable::Classification::SUBDIME);
  enum_<ChicFeeTable::Index>(outer, "Index")
    .value("NONE", ChicFeeTable::Index::NONE)
    .value("ACTIVE", ChicFeeTable::Index::ACTIVE)
    .value("PASSIVE", ChicFeeTable::Index::PASSIVE)
    .value("HIDDEN_ACTIVE", ChicFeeTable::Index::HIDDEN_ACTIVE)
    .value("HIDDEN_PASSIVE", ChicFeeTable::Index::HIDDEN_PASSIVE);
  module.def("parse_chic_fee_table", &ParseChicFeeTable);
  module.def("lookup_fee", static_cast<Money (*)(const ChicFeeTable&,
    ChicFeeTable::Index, ChicFeeTable::Classification)>(&LookupFee));
  module.def("calculate_fee", static_cast<Money (*)(const ChicFeeTable&,
    const OrderFields&, const ExecutionReport&)>(&CalculateFee));
}

void Nexus::Python::ExportConsolidatedTmxFeeTable(pybind11::module& module) {
  auto outer = class_<ConsolidatedTmxFeeTable>(module,
      "ConsolidatedTmxFeeTable")
    .def(init())
    .def(init<const ConsolidatedTmxFeeTable&>())
    .def_readwrite("spire_fee", &ConsolidatedTmxFeeTable::m_spireFee)
    .def_readwrite("iiroc_fee", &ConsolidatedTmxFeeTable::m_iirocFee)
    .def_readwrite("cds_fee", &ConsolidatedTmxFeeTable::m_cdsFee)
    .def_readwrite("cds_cap", &ConsolidatedTmxFeeTable::m_cdsCap)
    .def_readwrite("per_order_fee", &ConsolidatedTmxFeeTable::m_perOrderFee)
    .def_readwrite("per_order_cap", &ConsolidatedTmxFeeTable::m_perOrderCap)
    .def_readwrite("chic_fee_table", &ConsolidatedTmxFeeTable::m_chicFeeTable)
    .def_readwrite("lynx_fee_table", &ConsolidatedTmxFeeTable::m_lynxFeeTable)
    .def_readwrite("matn_fee_table", &ConsolidatedTmxFeeTable::m_matnFeeTable)
    .def_readwrite("neoe_fee_table", &ConsolidatedTmxFeeTable::m_neoeFeeTable)
    .def_readwrite("omga_fee_table", &ConsolidatedTmxFeeTable::m_omgaFeeTable)
    .def_readwrite("pure_fee_table", &ConsolidatedTmxFeeTable::m_pureFeeTable)
    .def_readwrite("tsx_fee_table", &ConsolidatedTmxFeeTable::m_tsxFeeTable)
    .def_readwrite("tsx_venture_table",
      &ConsolidatedTmxFeeTable::m_tsxVentureTable)
    .def_readwrite("xats_fee_table", &ConsolidatedTmxFeeTable::m_xatsFeeTable)
    .def_readwrite("xcx2_fee_table", &ConsolidatedTmxFeeTable::m_xcx2FeeTable)
    .def_readwrite("etfs", &ConsolidatedTmxFeeTable::m_etfs)
    .def_readwrite("interlisted", &ConsolidatedTmxFeeTable::m_interlisted);
  class_<ConsolidatedTmxFeeTable::State>(outer, "State")
    .def(init())
    .def_readonly("per_order_charges",
      &ConsolidatedTmxFeeTable::State::m_perOrderCharges)
    .def_readonly("fill_count",
      &ConsolidatedTmxFeeTable::State::m_fillCount);
  module.def("parse_consolidated_tmx_fee_table", &ParseConsolidatedTmxFeeTable);
  module.def("calculate_fee", static_cast<ExecutionReport (*)(
    const ConsolidatedTmxFeeTable&, ConsolidatedTmxFeeTable::State&,
    const Order&, const ExecutionReport&)>(&CalculateFee));
}

void Nexus::Python::ExportConsolidatedUsFeeTable(pybind11::module& module) {
  class_<ConsolidatedUsFeeTable>(module, "ConsolidatedUsFeeTable")
    .def(init())
    .def(init<const ConsolidatedUsFeeTable&>())
    .def_readwrite("spire_fee", &ConsolidatedUsFeeTable::m_spireFee)
    .def_readwrite("sec_rate", &ConsolidatedUsFeeTable::m_secRate)
    .def_readwrite("taf_fee", &ConsolidatedUsFeeTable::m_tafFee)
    .def_readwrite("nscc_rate", &ConsolidatedUsFeeTable::m_nsccRate)
    .def_readwrite("clearing_fee", &ConsolidatedUsFeeTable::m_clearingFee)
    .def_readwrite("amex_fee_table", &ConsolidatedUsFeeTable::m_amexFeeTable)
    .def_readwrite("arca_fee_table", &ConsolidatedUsFeeTable::m_arcaFeeTable)
    .def_readwrite("bats_fee_table", &ConsolidatedUsFeeTable::m_batsFeeTable)
    .def_readwrite("baty_fee_table", &ConsolidatedUsFeeTable::m_batyFeeTable)
    .def_readwrite("edga_fee_table", &ConsolidatedUsFeeTable::m_edgaFeeTable)
    .def_readwrite("edgx_fee_table", &ConsolidatedUsFeeTable::m_edgxFeeTable)
    .def_readwrite("nsdq_fee_table", &ConsolidatedUsFeeTable::m_nsdqFeeTable)
    .def_readwrite("nyse_fee_table", &ConsolidatedUsFeeTable::m_nyseFeeTable);
  module.def("parse_consolidated_us_fee_table", &ParseConsolidatedUsFeeTable);
  module.def("calculate_fee", static_cast<ExecutionReport (*)(
    const ConsolidatedUsFeeTable&, const Order&, const ExecutionReport&)>(
    &CalculateFee));
}

void Nexus::Python::ExportCseFeeTable(pybind11::module& module) {
  auto outer = class_<CseFeeTable>(module, "CseFeeTable")
    .def(init())
    .def(init<const CseFeeTable&>())
    .def_readwrite("fee_table", &CseFeeTable::m_feeTable);
  enum_<CseFeeTable::PriceClass>(outer, "PriceClass")
    .value("NONE", CseFeeTable::PriceClass::NONE)
    .value("DEFAULT", CseFeeTable::PriceClass::DEFAULT)
    .value("SUBDOLLAR", CseFeeTable::PriceClass::SUBDOLLAR)
    .value("SUBDIME", CseFeeTable::PriceClass::SUBDIME);
  module.def("parse_cse_fee_table", &ParseCseFeeTable);
  module.def("lookup_fee", static_cast<Money (*)(const CseFeeTable&,
    LiquidityFlag, CseFeeTable::PriceClass)>(&LookupFee));
  module.def("calculate_fee", static_cast<Money (*)(const CseFeeTable&,
    const ExecutionReport&)>(&CalculateFee));
}

void Nexus::Python::ExportEdgaFeeTable(pybind11::module& module) {
  class_<EdgaFeeTable>(module, "EdgaFeeTable")
    .def(init())
    .def(init<const EdgaFeeTable&>())
    .def_readwrite("fee_table", &EdgaFeeTable::m_feeTable)
    .def_readwrite("default_flag", &EdgaFeeTable::m_defaultFlag);
  module.def("parse_edga_fee_table", &ParseEdgaFeeTable);
  module.def("lookup_fee", static_cast<rational<int> (*)(const EdgaFeeTable&,
    const std::string&)>(&LookupFee));
  module.def("calculate_fee", static_cast<Money (*)(const EdgaFeeTable&,
    const ExecutionReport&)>(&CalculateFee));
}

void Nexus::Python::ExportEdgxFeeTable(pybind11::module& module) {
  class_<EdgxFeeTable>(module, "EdgxFeeTable")
    .def(init())
    .def(init<const EdgxFeeTable&>())
    .def_readwrite("fee_table", &EdgxFeeTable::m_feeTable)
    .def_readwrite("default_flag", &EdgxFeeTable::m_defaultFlag);
  module.def("parse_edga_fee_table", &ParseEdgxFeeTable);
  module.def("lookup_fee", static_cast<rational<int> (*)(const EdgxFeeTable&,
    const std::string&)>(&LookupFee));
  module.def("calculate_fee", static_cast<Money (*)(const EdgxFeeTable&,
    const ExecutionReport&)>(&CalculateFee));
}

void Nexus::Python::ExportFeeHandling(pybind11::module& module) {
  ExportAmexFeeTable(module);
  ExportArcaFeeTable(module);
  ExportAsxtFeeTable(module);
  ExportBatsFeeTable(module);
  ExportBatyFeeTable(module);
  ExportChicFeeTable(module);
  ExportConsolidatedTmxFeeTable(module);
  ExportConsolidatedUsFeeTable(module);
  ExportCseFeeTable(module);
  ExportEdgaFeeTable(module);
  ExportEdgxFeeTable(module);
  ExportHkexFeeTable(module);
  ExportJpxFeeTable(module);
  ExportLiquidityFlag(module);
  ExportLynxFeeTable(module);
  ExportMatnFeeTable(module);
  ExportNeoeFeeTable(module);
  ExportNexFeeTable(module);
  ExportNsdqFeeTable(module);
  ExportNyseFeeTable(module);
  ExportOmgaFeeTable(module);
  ExportPureFeeTable(module);
  ExportTsxFeeTable(module);
  ExportXatsFeeTable(module);
  ExportXcx2FeeTable(module);
}

void Nexus::Python::ExportHkexFeeTable(pybind11::module& module) {
  class_<HkexFeeTable>(module, "HkexFeeTable")
    .def(init())
    .def(init<const HkexFeeTable&>())
    .def_readwrite("spire_fee", &HkexFeeTable::m_spireFee)
    .def_readwrite("stamp_tax", &HkexFeeTable::m_stampTax)
    .def_readwrite("levy", &HkexFeeTable::m_levy)
    .def_readwrite("trading_fee", &HkexFeeTable::m_tradingFee)
    .def_readwrite("brokerage_fee", &HkexFeeTable::m_brokerageFee)
    .def_readwrite("ccass_fee", &HkexFeeTable::m_ccassFee)
    .def_readwrite("minimum_ccass_fee", &HkexFeeTable::m_minimumCcassFee)
    .def_readwrite("stamp_applicability", &HkexFeeTable::m_stampApplicability);
  module.def("parse_hkex_stamp_securities", &ParseHkexStampSecurities);
  module.def("parse_hkex_fee_table", &ParseHkexFeeTable);
  module.def("calculate_fee", static_cast<ExecutionReport (*)(
    const HkexFeeTable&, const OrderFields&, const ExecutionReport&)>(
    &CalculateFee));
}

void Nexus::Python::ExportJpxFeeTable(pybind11::module& module) {
  class_<JpxFeeTable>(module, "JpxFeeTable")
    .def(init())
    .def(init<const JpxFeeTable&>())
    .def_readwrite("spire_fee", &JpxFeeTable::m_spireFee)
    .def_readwrite("brokerage_fee", &JpxFeeTable::m_brokerageFee);
  module.def("parse_jpx_fee_table", &ParseJpxFeeTable);
  module.def("calculate_fee", static_cast<ExecutionReport (*)(
    const JpxFeeTable&, const OrderFields&, const ExecutionReport&)>(
    &CalculateFee));
}

void Nexus::Python::ExportLiquidityFlag(pybind11::module& module) {
  enum_<LiquidityFlag>(module, "LiquidityFlag")
    .value("NONE", LiquidityFlag::NONE)
    .value("ACTIVE", LiquidityFlag::ACTIVE)
    .value("PASSIVE", LiquidityFlag::PASSIVE)
    .def("__str__", &lexical_cast<std::string, LiquidityFlag>);
}

void Nexus::Python::ExportLynxFeeTable(pybind11::module& module) {
  auto outer = class_<LynxFeeTable>(module, "LynxFeeTable")
    .def(init())
    .def(init<const LynxFeeTable&>())
    .def_readwrite("fee_table", &LynxFeeTable::m_feeTable);
  enum_<LynxFeeTable::PriceClass>(outer, "PriceClass")
    .value("NONE", LynxFeeTable::PriceClass::NONE)
    .value("DEFAULT", LynxFeeTable::PriceClass::DEFAULT)
    .value("SUBDOLLAR", LynxFeeTable::PriceClass::SUBDOLLAR);
  module.def("parse_lynx_fee_table", &ParseLynxFeeTable);
  module.def("lookup_fee", static_cast<Money (*)(const LynxFeeTable&,
    LiquidityFlag, LynxFeeTable::PriceClass)>(&LookupFee));
  module.def("calculate_fee", static_cast<Money (*)(const LynxFeeTable&,
    const ExecutionReport&)>(&CalculateFee));
}

void Nexus::Python::ExportMatnFeeTable(pybind11::module& module) {
  auto outer = class_<MatnFeeTable>(module, "MatnFeeTable")
    .def(init())
    .def(init<const MatnFeeTable&>())
    .def_readwrite("general_fee_table", &MatnFeeTable::m_generalFeeTable)
    .def_readwrite("alternative_fee_table",
      &MatnFeeTable::m_alternativeFeeTable);
  enum_<MatnFeeTable::PriceClass>(outer, "PriceClass")
    .value("NONE", MatnFeeTable::PriceClass::NONE)
    .value("DEFAULT", MatnFeeTable::PriceClass::DEFAULT)
    .value("SUBFIVE_DOLLAR", MatnFeeTable::PriceClass::SUBFIVE_DOLLAR)
    .value("SUBDOLLAR", MatnFeeTable::PriceClass::SUBDOLLAR);
  enum_<MatnFeeTable::GeneralIndex>(outer, "GeneralIndex")
    .value("NONE", MatnFeeTable::GeneralIndex::NONE)
    .value("FEE", MatnFeeTable::GeneralIndex::FEE)
    .value("MAX_CHARGE", MatnFeeTable::GeneralIndex::MAX_CHARGE);
  enum_<MatnFeeTable::Category>(outer, "Category")
    .value("NONE", MatnFeeTable::Category::NONE)
    .value("ETF", MatnFeeTable::Category::ETF)
    .value("ODD_LOT", MatnFeeTable::Category::ODD_LOT);
  enum_<MatnFeeTable::Classification>(outer, "Classification")
    .value("NONE", MatnFeeTable::Classification::NONE)
    .value("DEFAULT", MatnFeeTable::Classification::DEFAULT)
    .value("ETF", MatnFeeTable::Classification::ETF);
  module.def("parse_matn_fee_table", &ParseMatnFeeTable);
  module.def("lookup_fee", static_cast<Money (*)(const MatnFeeTable&,
    MatnFeeTable::GeneralIndex, MatnFeeTable::PriceClass)>(&LookupFee));
  module.def("lookup_fee", static_cast<Money (*)(const MatnFeeTable&,
    LiquidityFlag, MatnFeeTable::Category)>(&LookupFee));
  module.def("calculate_fee", static_cast<Money (*)(const MatnFeeTable&,
    MatnFeeTable::Classification, const ExecutionReport&)>(&CalculateFee));
}

void Nexus::Python::ExportNeoeFeeTable(pybind11::module& module) {
  auto outer = class_<NeoeFeeTable>(module, "NeoeFeeTable")
    .def(init())
    .def(init<const NeoeFeeTable&>())
    .def_readwrite("general_fee_table", &NeoeFeeTable::m_generalFeeTable)
    .def_readwrite("interlisted_fee_table",
      &NeoeFeeTable::m_interlistedFeeTable)
    .def_readwrite("neoe_book_fee_table", &NeoeFeeTable::m_neoBookFeeTable);
  enum_<NeoeFeeTable::PriceClass>(outer, "PriceClass")
    .value("NONE", NeoeFeeTable::PriceClass::NONE)
    .value("DEFAULT", NeoeFeeTable::PriceClass::DEFAULT)
    .value("SUBDOLLAR", NeoeFeeTable::PriceClass::SUBDOLLAR);
  module.def("parse_neoe_fee_table", &ParseNeoeFeeTable);
  module.def("is_neo_book_order", &IsNeoBookOrder);
  module.def("lookup_general_fee", static_cast<Money (*)(const NeoeFeeTable&,
    LiquidityFlag, NeoeFeeTable::PriceClass)>(&LookupGeneralFee));
  module.def("lookup_interlisted_fee", &LookupInterlistedFee);
  module.def("lookup_neo_book_fee", &LookupNeoBookFee);
  module.def("calculate_fee", static_cast<Money (*)(const NeoeFeeTable&, bool,
    const OrderFields&, const ExecutionReport&)>(&CalculateFee));
}

void Nexus::Python::ExportNexFeeTable(pybind11::module& module) {
  auto outer = class_<NexFeeTable>(module, "NexFeeTable")
    .def(init())
    .def(init<const NexFeeTable&>())
    .def_readwrite("fee", &NexFeeTable::m_fee);
  module.def("parse_nex_fee_table", &ParseNexFeeTable);
  module.def("calculate_fee", static_cast<Money (*)(const NexFeeTable&,
    const ExecutionReport&)>(&CalculateFee));
}

void Nexus::Python::ExportNsdqFeeTable(pybind11::module& module) {
  auto outer = class_<NsdqFeeTable>(module, "NsdqFeeTable")
    .def(init())
    .def(init<const NsdqFeeTable&>())
    .def_readwrite("fee_table", &NsdqFeeTable::m_feeTable)
    .def_readwrite("subdollar_table", &NsdqFeeTable::m_subdollarTable);
  enum_<NsdqFeeTable::Category>(outer, "Category")
    .value("NONE", NsdqFeeTable::Category::NONE)
    .value("DEFAULT", NsdqFeeTable::Category::DEFAULT)
    .value("HIDDEN", NsdqFeeTable::Category::HIDDEN)
    .value("CROSS", NsdqFeeTable::Category::CROSS)
    .value("ON_OPEN", NsdqFeeTable::Category::ON_OPEN)
    .value("ON_CLOSE", NsdqFeeTable::Category::ON_CLOSE)
    .value("RETAIL", NsdqFeeTable::Category::RETAIL);
  module.def("parse_nsdq_fee_table", &ParseNsdqFeeTable);
  module.def("lookup_fee", static_cast<Money (*)(const NsdqFeeTable&,
    LiquidityFlag, NsdqFeeTable::Category)>(&LookupFee));
  module.def("calculate_fee", static_cast<Money (*)(const NsdqFeeTable&,
    const ExecutionReport&)>(&CalculateFee));
}

void Nexus::Python::ExportNyseFeeTable(pybind11::module& module) {
  auto outer = class_<NyseFeeTable>(module, "NyseFeeTable")
    .def(init())
    .def(init<const NyseFeeTable&>())
    .def_readwrite("fee_table", &NyseFeeTable::m_feeTable)
    .def_readwrite("subdollar_table", &NyseFeeTable::m_subdollarTable);
  enum_<NyseFeeTable::Category>(outer, "Category")
    .value("NONE", NyseFeeTable::Category::NONE)
    .value("DEFAULT", NyseFeeTable::Category::DEFAULT)
    .value("HIDDEN", NyseFeeTable::Category::HIDDEN)
    .value("CROSS", NyseFeeTable::Category::CROSS)
    .value("ON_OPEN", NyseFeeTable::Category::ON_OPEN)
    .value("ON_CLOSE", NyseFeeTable::Category::ON_CLOSE)
    .value("RETAIL", NyseFeeTable::Category::RETAIL);
  module.def("parse_nyse_fee_table", &ParseNyseFeeTable);
  module.def("lookup_fee", static_cast<Money (*)(const NyseFeeTable&,
    LiquidityFlag, NyseFeeTable::Category)>(&LookupFee));
  module.def("is_nyse_hidden_liquidity_provider",
    &IsNyseHiddenLiquidityProvider);
  module.def("calculate_fee", static_cast<Money (*)(const NyseFeeTable&,
    const OrderFields&, const ExecutionReport&)>(&CalculateFee));
}

void Nexus::Python::ExportPureFeeTable(pybind11::module& module) {
  class_<PureFeeTable>(module, "PureFeeTable")
    .def(init<const PureFeeTable&>());
  module.def("parse_pure_fee_table", &ParsePureFeeTable);
  module.def("calculate_fee", static_cast<Money (*)(const PureFeeTable&,
    const Security&, const ExecutionReport&)>(&CalculateFee));
}
