# Locale Binary Format

This project stores locale tables in `.tla` files using a small binary format.

## Layout

All fields are little-endian.

```text
u32 magic
u32 version
TkString locale
TkString fallbackLocale
TkHashMap<TkString, TkArray<TkString>> entries
```

## Magic And Version

- `magic` is the ASCII marker `I18N` encoded as a 32-bit little-endian value.
- `version` is currently `1`.

## String Encoding

`TkString` is written as:

```text
i32 byteLength
byte[byteLength] utf8Bytes
```

An empty string has length `0`.

## Entry Encoding

`entries` is serialized as a hash map:

```text
i32 entryCount
repeat entryCount times:
    TkString key
    TkArray<TkString> values
```

Each plural entry stores multiple strings in `values`.
If `values[0]` contains a `%` character, the runtime formats it with `sprintf`-style substitution using the requested plural number.

## Samples

- `docs/testdata/i18n_en_v1.tla`
- `docs/testdata/i18n_en_gb_v1.tla`
- `docs/testdata/i18n_invalid_magic.tla`

## Loader Rules

- `locale` must not be empty.
- `fallbackLocale` is stored by name only; the runtime resolves the pointer later.
- `i18n_invalid_magic.tla` is intentionally invalid and should fail to load.
