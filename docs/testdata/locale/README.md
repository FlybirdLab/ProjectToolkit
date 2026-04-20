# Locale Test Data

Binary `.tla` files used by `LocaleTest`.

Format:
- `u32 magic` = `I18N` (little-endian `0x4E383149`)
- `u32 version` = `1`
- `TkString locale`
- `TkString fallbackLocale`
- `TkHashMap<TkString, TkArray<TkString>> entries`

Files:
- `i18n_en_v1.tla`
- `i18n_en_gb_v1.tla`
- `i18n_invalid_magic.tla`
