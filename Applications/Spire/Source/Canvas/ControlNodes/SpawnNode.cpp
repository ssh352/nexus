#include "Spire/Canvas/ControlNodes/SpawnNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Common/NoneNode.hpp"
#include "Spire/Canvas/Types/UnionType.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace boost;
using namespace Spire;
using namespace std;

SpawnNode::SpawnNode() {
  SetText("Spawn");
  SetType(UnionType::GetAnyType());
  AddChild("trigger", make_unique<NoneNode>(UnionType::GetAnyType()));
  AddChild("series", make_unique<NoneNode>(UnionType::GetAnyType()));
}

std::unique_ptr<CanvasNode> SpawnNode::Convert(const CanvasType& type) const {
  auto clone = CanvasNode::Clone(*this);
  auto& clonedSeries = clone->GetChildren().back();
  clone->SetChild(clonedSeries, clonedSeries.Convert(type));
  clone->SetType(type);
  return clone;
}

std::unique_ptr<CanvasNode> SpawnNode::Replace(const CanvasNode& child,
    std::unique_ptr<CanvasNode> replacement) const {
  if(&child == &GetChildren().back()) {
    auto clone = CanvasNode::Clone(*this);
    clone->SetChild(clone->GetChildren().back(), std::move(replacement));
    clone->SetType(clone->GetChildren().back().GetType());
    return clone;
  }
  return CanvasNode::Replace(child, std::move(replacement));
}

void SpawnNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> SpawnNode::Clone() const {
  return make_unique<SpawnNode>(*this);
}

SpawnNode::SpawnNode(ReceiveBuilder) {}
