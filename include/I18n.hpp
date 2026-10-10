#pragma once

/** Bilingual ES/EN strings: call sites pass both texts, L() picks the active language. */
namespace I18n {

enum class Lang { ES = 0, EN = 1 };

inline Lang g_lang = Lang::ES;

inline const char* L(const char* es, const char* en) {
    return g_lang == Lang::EN ? en : es;
}

inline bool isEn() {
    return g_lang == Lang::EN;
}

} // namespace I18n
