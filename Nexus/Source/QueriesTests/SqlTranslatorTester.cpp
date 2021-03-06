#include <Beam/Queries/StandardValues.hpp>
#include <doctest/doctest.h>
#include "Nexus/Queries/SqlTranslator.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Nexus;
using namespace Nexus::Queries;

TEST_SUITE("SqlTranslator") {
  TEST_CASE("query_order_fields") {
    auto infoParameterExpression = ParameterExpression(0, OrderInfoType());
    auto fieldsAccessExpression = MemberAccessExpression("fields",
      OrderFieldsType(), infoParameterExpression);
    auto securityAccessExpression = MemberAccessExpression("security",
      SecurityType(), fieldsAccessExpression);
    auto marketAccessExpression = MemberAccessExpression("market", StringType(),
      securityAccessExpression);
    auto queryMarketCode = StringValue("XTSX");
    auto marketCodeExpression = ConstantExpression(queryMarketCode);
    auto equalExpression = MakeEqualsExpression(marketCodeExpression,
      marketAccessExpression);
    auto translator = Nexus::Queries::SqlTranslator("submissions",
      equalExpression);
    auto translation = translator.Build();
    auto query = std::string();
    translation.append_query(query);
    REQUIRE(query == "(\"XTSX\" = market)");
  }
}
