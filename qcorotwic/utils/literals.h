#include <QStringLiteral>
#include <QStringView>

namespace QCoroTwic::Literals {

constexpr QStringView operator""_qsv(const char16_t *str, std::size_t len) {
    return QStringView{str, static_cast<qsizetype>(len)};
}

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-string-literal-operator-template"
#pragma clang diagnostic ignored "-Wpedantic"
#elif __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

template<typename CharT, CharT ... str>
QString operator""_qs() noexcept {
    static_assert(std::is_same_v<CharT, char16_t>, "The _qs literal operator can only be used for Unicode string literals");
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return QString{QStringPrivate{nullptr, {str ...}, sizeof...(str) / 2 -1}};
#else
    static constexpr const QStaticStringData<sizeof...(str)> qstring_literal {
        Q_STATIC_STRING_DATA_HEADER_INITIALIZER(sizeof...(str)),
        {str ...}
    };
    return QString{QStringDataPtr{qstring_literal.data_ptr()}};
#endif
}

#ifdef __clang__
#pragma clang diagnostic pop
#elif __GNUC__
#pragma GCC diagnostic pop
#endif

} // namespace QCoroTwic::Literals
