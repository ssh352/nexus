#ifndef SPIRE_DISJUNCTION_TERM_HPP
#define SPIRE_DISJUNCTION_TERM_HPP
#include <memory>
#include <vector>
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/Term.hpp"

namespace Spire {

  //! Is satisfied iff at least one of the given terms evaluates to true.
  class DisjunctionTerm : public Term {
    public:

      //! Constructs a DisjunctionTerm.
      /*!
        \param terms The terms to evaluate for tags.
      */
      explicit DisjunctionTerm(std::vector<std::shared_ptr<Term>> terms);

      bool is_satisfied(const std::vector<KeyBindings::Tag>& tags) const
        override;

    private:
      std::vector<std::shared_ptr<Term>> m_terms;
  };
}

#endif