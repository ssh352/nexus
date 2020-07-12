#ifndef NEXUS_QUOTE_CONVERSIONS_HPP
#define NEXUS_QUOTE_CONVERSIONS_HPP
#include "Nexus/Definitions/BookQuote.hpp"
#include "Nexus/Definitions/MarketQuote.hpp"

namespace Nexus {

  /** Stores a bid/ask pair of BookQuotes. */
  struct BookQuotePair {

    /** The BookQuote on the ASK Side. */
    BookQuote m_ask;

    /** The BookQuote on the BID Side. */
    BookQuote m_bid;

    /** Constructs an uninitialized BookQuotePair. */
    BookQuotePair() = default;

    /**
     * Constructs a BookQuotePair.
     * @param ask The ASK Side BookQuote.
     * @param bid The BID Side BookQuote.
     */
    BookQuotePair(BookQuote ask, BookQuote bid);
  };

  /**
   * Converts a MarketQuote into a BookQuotePair.
   * @param quote The MarketQuote to convert.
   * @return A BookQuotePair representing the <i>quote</i>.
   */
  inline BookQuotePair ToBookQuotePair(const MarketQuote& quote) {
    return BookQuotePair(BookQuote(quote.m_market.GetData(), true,
      quote.m_market, quote.m_ask, quote.m_timestamp),
      BookQuote(quote.m_market.GetData(), true, quote.m_market, quote.m_bid,
      quote.m_timestamp));
  }

  BookQuotePair::BookQuotePair(BookQuote ask, BookQuote bid)
    : m_ask(std::move(ask)),
      m_bid(std::move(bid)) {}
}

#endif
