#ifndef SPIRE_EMPTY_BOOK_VIEW_MODEL_HPP
#define SPIRE_EMPTY_BOOK_VIEW_MODEL_HPP
#include "spire/book_view/book_view.hpp"
#include "spire/book_view/book_view_model.hpp"

namespace spire {

  //! Implements a book view model with no quotes.
  class empty_book_view_model final : public book_view_model {
    public:

      //! Constructs an empty book view model.
      /*!
        \param s The security to model.
      */
      empty_book_view_model(Nexus::Security security);

      const Nexus::Security& get_security() const override;

      const Nexus::BboQuote& get_bbo() const override;

      const std::vector<Nexus::BookQuote>& get_asks() const override;

      const std::vector<Nexus::BookQuote>& get_bids() const override;

      boost::optional<Nexus::Money> get_high() const override;

      boost::optional<Nexus::Money> get_low() const override;

      boost::optional<Nexus::Money> get_open() const override;

      boost::optional<Nexus::Money> get_close() const override;

      Nexus::Quantity get_volume() const override;

      qt_promise<void> load() override;

      boost::signals2::connection connect_bbo_slot(
        const bbo_signal::slot_type& slot) const override;

      boost::signals2::connection connect_book_quote_slot(
        const book_quote_signal::slot_type& slot) const override;

      boost::signals2::connection connect_high_slot(
        const price_signal::slot_type& slot) const override;

      boost::signals2::connection connect_low_slot(
        const price_signal::slot_type& slot) const override;

      boost::signals2::connection connect_open_slot(
        const price_signal::slot_type& slot) const override;

      boost::signals2::connection connect_close_slot(
        const price_signal::slot_type& slot) const override;

      boost::signals2::connection connect_volume_slot(
        const quantity_signal::slot_type& slot) const override;

    private:
      Nexus::Security m_security;
  };
}

#endif
