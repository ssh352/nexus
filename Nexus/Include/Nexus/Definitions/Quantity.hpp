#ifndef NEXUS_QUANTITY_HPP
#define NEXUS_QUANTITY_HPP
#include <cstdint>
#include <cstdlib>
#include <istream>
#include <limits>
#include <ostream>
#include <string>
#include <Beam/Serialization/Receiver.hpp>
#include <Beam/Serialization/Sender.hpp>
#include <Beam/Utilities/Math.hpp>
#include <boost/cstdfloat.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional/optional.hpp>
#include <boost/rational.hpp>
#include "Nexus/Definitions/Definitions.hpp"

namespace Nexus {

  /*! \class Quantity
      \brief Used to represent a quantity up to 15 significant decimal places.
   */
  class Quantity {
    public:

      //! The number of decimal places that can be represented accurately.
      static const unsigned int DECIMAL_PLACES = 6;

      //! The multiplier used.
      static const std::int64_t MULTIPLIER = 1000000;

      //! Returns a Quantity from a string.
      /*!
        \param value The value to represent.
        \return A Quantity representing the specified <i>value</i>.
      */
      static boost::optional<Quantity> FromValue(const std::string& value);

      //! Constructs a Quantity with a value of 0.
      Quantity();

      //! Constructs a Quantity from an int32.
      Quantity(std::int32_t value);

      //! Constructs a Quantity from a uint32.
      Quantity(std::uint32_t value);

      //! Constructs a Quantity from an int64.
      Quantity(std::int64_t value);

      //! Constructs a Quantity from a uint64.
      Quantity(std::uint64_t value);

      //! Constructs a Quantity from a double.
      Quantity(double value);

      //! Converts this Quantity into a float.
      explicit operator boost::float64_t() const;

      //! Less than test.
      /*!
        \param rhs The right hand side of the operation.
        \return <code>true</code> iff this is less than <i>rhs</i>.
      */
      bool operator <(Quantity rhs) const;

      //! Less than or equal test.
      /*!
        \param rhs The right hand side of the operation.
        \return <code>true</code> iff this is less than or equal to <i>rhs</i>.
      */
      bool operator <=(Quantity rhs) const;

      //! Tests for equality.
      /*!
        \param rhs The right hand side of the operation.
        \return <code>true</code> iff this is equal to <i>rhs</i>.
      */
      bool operator ==(Quantity rhs) const;

      //! Tests for inequality.
      /*!
        \param rhs The right hand side of the operation.
        \return <code>true</code> iff this is not equal to <i>rhs</i>.
      */
      bool operator !=(Quantity rhs) const;

      //! Greater than or equal test.
      /*!
        \param rhs The right hand side of the operation.
        \return <code>true</code> iff this is greater than or equal to
                <i>rhs</i>.
      */
      bool operator >=(Quantity rhs) const;

      //! Greater than test.
      /*!
        \param rhs The right hand side of the operation.
        \return <code>true</code> iff this is greater than <i>rhs</i>.
      */
      bool operator >(Quantity rhs) const;

      //! Adds two Quantities together.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i> + <i>rhs</i>.
      */
      Quantity operator +(Quantity rhs) const;

      //! Increases this Quantity.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i>.
      */
      Quantity& operator +=(Quantity rhs);

      //! Subtracts two Quantities together.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i> - <i>rhs</i>.
      */
      Quantity operator -(Quantity rhs) const;

      //! Decreases this Quantity.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i>.
      */
      Quantity& operator -=(Quantity rhs);

      //! Multiplies two Quantities together.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i> * <i>rhs</i>.
      */
      Quantity operator *(Quantity rhs) const;

      //! Multiplies this Quantity.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i>.
      */
      Quantity& operator *=(Quantity rhs);

      //! Divides two Quantities together.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i> / <i>rhs</i>.
      */
      Quantity operator /(Quantity rhs) const;

      //! Divides this Quantity.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i>.
      */
      Quantity& operator /=(Quantity rhs);

      //! Returns the unary negation of this Quantity.
      /*!
        \return -<i>this</i>.
      */
      Quantity operator -() const;

    private:
      template<typename, typename> friend struct Beam::Serialization::Send;
      template<typename, typename> friend struct Beam::Serialization::Receive;
      friend std::ostream& operator <<(std::ostream& out, Quantity value);
      friend Quantity Abs(Quantity);
      struct RawValue{};
      boost::float64_t m_value;

      Quantity(boost::float64_t value, RawValue);
  };

  inline std::ostream& operator <<(std::ostream& out, Quantity value) {
    out << (value.m_value / Quantity::MULTIPLIER);
    return out;
  }

  inline std::istream& operator >>(std::istream& in, Quantity& value) {
    std::string symbol;
    in >> symbol;
    auto parsedValue = Quantity::FromValue(symbol);
    if(!parsedValue.is_initialized()) {
      in.setstate(std::ios::failbit);
      return in;
    }
    value = *parsedValue;
    return in;
  }

  template<typename T>
  Quantity operator +(T lhs, Quantity rhs) {
    return Quantity{lhs} + rhs;
  }

  template<typename T>
  Quantity operator *(T lhs, Quantity rhs) {
    return Quantity{lhs} * rhs;
  }

  template<typename T>
  Quantity operator *(const boost::rational<T>& lhs, Quantity rhs) {
    return (lhs.numerator() * rhs) / lhs.denominator();
  }

  //! Returns the absolute value.
  /*!
    \param value The value.
  */
  inline Quantity Abs(Quantity value) {
    return Quantity{std::abs(value.m_value), Quantity::RawValue{}};
  }

  //! Returns the floor.
  /*!
    \param value The value to floor.
    \param decimalPlaces The decimal place to floor to.
  */
  inline Quantity Floor(Quantity value, int decimalPlaces) {
    auto multiplier = Beam::PowerOfTen(decimalPlaces);
    auto rawValue = static_cast<boost::float64_t>(value);
    return Quantity{std::floor(multiplier * rawValue) / multiplier};
  }

  //! Returns the ceiling.
  /*!
    \param value The value to ceil.
    \param decimalPlaces The decimal place to ceil to.
  */
  inline Quantity Ceil(Quantity value, int decimalPlaces) {
    auto multiplier = Beam::PowerOfTen(decimalPlaces);
    auto rawValue = static_cast<boost::float64_t>(value);
    return Quantity{std::ceil(multiplier * rawValue) / multiplier};
  }

  //! Returns the truncated value.
  /*!
    \param value The value to truncate.
    \param decimalPlaces The decimal place to truncate.
  */
  inline Quantity Truncate(Quantity value, int decimalPlaces) {
    if(value < 0) {
      return Ceil(value, decimalPlaces);
    } else {
      return Floor(value, decimalPlaces);
    }
  }

  //! Returns the rounded value.
  /*!
    \param value The value to round.
    \param decimalPlaces The decimal place to round to.
  */
  inline Quantity Round(Quantity value, int decimalPlaces) {
    auto multiplier = Beam::PowerOfTen(decimalPlaces);
    auto rawValue = static_cast<boost::float64_t>(value);
    return Quantity{std::round(multiplier * rawValue) / multiplier};
  }

  inline boost::optional<Quantity> Quantity::FromValue(
      const std::string& value) {
    if(value.empty()) {
      return boost::none;
    }
    auto i = value.begin();
    std::int64_t sign;
    if(*i == '-') {
      sign = -1;
      ++i;
    } else {
      sign = 1;
      if(*i == '+') {
        ++i;
      }
    }
    std::int64_t leftHand = 0;
    auto hasDecimals = false;
    while(i != value.end()) {
      if(*i >= '0' && *i <= '9') {
        leftHand *= 10;
        leftHand += *i - '0';
        ++i;
      } else if(*i == '.') {
        hasDecimals = true;
        ++i;
        break;
      } else {
        return boost::none;
      }
    }
    std::int64_t rightHand = 0;
    auto multiplier = MULTIPLIER;
    if(hasDecimals) {
      while(i != value.end()) {
        if(*i >= '0' && *i <= '9') {
          rightHand *= 10;
          rightHand += *i - '0';
          ++i;
          multiplier /= 10;
        } else {
          return boost::none;
        }
      }
    }
    auto finalValue = sign * (leftHand * MULTIPLIER + rightHand * multiplier);
    return Quantity{static_cast<boost::float64_t>(finalValue), RawValue{}};
  }

  inline Quantity::Quantity()
      : m_value{0} {}

  inline Quantity::Quantity(std::int32_t value)
      : m_value{static_cast<boost::float64_t>(MULTIPLIER * value)} {}

  inline Quantity::Quantity(std::uint32_t value)
      : m_value{static_cast<boost::float64_t>(MULTIPLIER * value)} {}

  inline Quantity::Quantity(std::int64_t value)
      : m_value{static_cast<boost::float64_t>(MULTIPLIER * value)} {}

  inline Quantity::Quantity(std::uint64_t value)
      : m_value{static_cast<boost::float64_t>(MULTIPLIER * value)} {}

  inline Quantity::Quantity(double value)
      : m_value{static_cast<boost::float64_t>(MULTIPLIER * value)} {}

  inline Quantity::operator boost::float64_t() const {
    return m_value / MULTIPLIER;
  }

  inline bool Quantity::operator <(Quantity rhs) const {
    return m_value < rhs.m_value;
  }

  inline bool Quantity::operator <=(Quantity rhs) const {
    return m_value <= rhs.m_value;
  }

  inline bool Quantity::operator ==(Quantity rhs) const {
    return m_value == rhs.m_value;
  }

  inline bool Quantity::operator !=(Quantity rhs) const {
    return m_value != rhs.m_value;
  }

  inline bool Quantity::operator >=(Quantity rhs) const {
    return m_value >= rhs.m_value;
  }

  inline bool Quantity::operator >(Quantity rhs) const {
    return m_value > rhs.m_value;
  }

  inline Quantity Quantity::operator +(Quantity rhs) const {
    return {m_value + rhs.m_value, RawValue{}};
  }

  inline Quantity& Quantity::operator +=(Quantity rhs) {
    m_value += rhs.m_value;
    return *this;
  }

  inline Quantity Quantity::operator -(Quantity rhs) const {
    return {m_value - rhs.m_value, RawValue{}};
  }

  inline Quantity& Quantity::operator -=(Quantity rhs) {
    m_value -= rhs.m_value;
    return *this;
  }

  inline Quantity Quantity::operator *(Quantity rhs) const {
    return {m_value * (rhs.m_value / MULTIPLIER), RawValue{}};
  }

  inline Quantity& Quantity::operator *=(Quantity rhs) {
    m_value *= (rhs.m_value / MULTIPLIER);
    return *this;
  }

  inline Quantity Quantity::operator /(Quantity rhs) const {
    return {m_value / (rhs.m_value / MULTIPLIER), RawValue{}};
  }

  inline Quantity& Quantity::operator /=(Quantity rhs) {
    m_value /= (rhs.m_value / MULTIPLIER);
    return *this;
  }

  inline Quantity Quantity::operator -() const {
    return {-m_value, RawValue{}};
  }

  inline Quantity::Quantity(boost::float64_t value, RawValue)
      : m_value{value} {}
}

namespace Beam {
  inline std::string ToString(Nexus::Quantity value) {
    return boost::lexical_cast<std::string>(value);
  }

namespace Serialization {
  template<>
  struct IsStructure<Nexus::Quantity> : std::false_type {};

  template<>
  struct Send<Nexus::Quantity> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const Nexus::Quantity& value) const {
      shuttle.Send(name, value.m_value);
    }
  };

  template<>
  struct Receive<Nexus::Quantity> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        Nexus::Quantity& value) const {
      boost::float64_t representation;
      shuttle.Shuttle(name, representation);
      value = Nexus::Quantity{representation, Nexus::Quantity::RawValue{}};
    }
  };
}
}

#endif