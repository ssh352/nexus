#ifndef SPIRE_FUNCTIONNODE_HPP
#define SPIRE_FUNCTIONNODE_HPP
#include <Beam/Serialization/ShuttleVector.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/SignatureNode.hpp"

namespace Spire {

  /*! \class FunctionNode
      \brief A CanvasNode that applies a function to its children.
   */
  class FunctionNode : public SignatureNode {
    protected:

      //! Constructs a FunctionNode.
      FunctionNode() = default;

      //! Defines a FunctionNode.
      /*!
        \param name The name of the function.
        \param parameterNames The name of each parameter.
        \param signatures The available signatures for this function.
      */
      void DefineFunction(std::string name,
        std::vector<std::string> parameterNames,
        std::vector<Signature> signatures);

      virtual const std::vector<Signature>& GetSignatures() const;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);

    private:
      friend struct Beam::Serialization::DataShuttle;
      std::string m_name;
      std::vector<Signature> m_signatures;
  };

  template<typename Shuttler>
  void FunctionNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    SignatureNode::Shuttle(shuttle, version);
    shuttle.Shuttle("name", m_name);
    shuttle.Shuttle("signatures", m_signatures);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::FunctionNode> : std::false_type {};
}
}

#endif
