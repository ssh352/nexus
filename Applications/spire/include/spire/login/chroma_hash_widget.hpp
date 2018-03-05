#ifndef SPIRE_CHROMA_HASH_WIDGET_HPP
#define SPIRE_CHROMA_HASH_WIDGET_HPP
#include <array>
#include <QColor>
#include <QString>
#include <QWidget>
#include "spire/login/login.hpp"

namespace spire {

  //! \brief Displays chroma hash.
  class chroma_hash_widget : public QWidget {
    public:

      //! Constructs a chroma hash for an empty text.
      chroma_hash_widget(QWidget* parent = nullptr);

      //! Sets the text to hash.
      void set_text(const QString& text);

    private:
      static constexpr auto COMPONENTS = 3;
      std::array<QColor, COMPONENTS> m_colors;
  };
}

#endif
