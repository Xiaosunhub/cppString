# cppString

A modern C++23 string utility library — a `std::string`-compatible core class
(`sp::cppString`) plus a collection of focused helper modules for UTF-8,
encoding, data exchange (JSON / CSV / INI / URI), internationalization,
validation, paths, checksums, versioning, formatting, terminal output,
binary parsing, file system operations, regular expressions, date/time,
and everyday text work.

一个现代 C++23 字符串工具库：与 `std::string` 对齐的核心类 `sp::cppString`，
外加 UTF-8、编解码、数据交换（JSON / CSV / INI / URI）、国际化处理、
校验、路径、哈希、语义化版本、格式化、终端输出、二进制解析、
文件系统操作、正则表达式、日期时间、文本处理等一系列独立模块。

当前版本：`5.4.0`（新增正则与日期时间模块）。
Current release: **5.4.0** — regular expressions and date/time modules.

## Features

### Core class — `sp::cppString` (`cppString.h`)

- Full `std::string` parity interface: `append` / `insert` / `erase` / `assign` /
  `resize` / `compare` / `rfind` / `find_first_of` / `find_first_not_of` / `substr` …
- Iterators (works with `std::ranges`), `operator<=>`, stream `<<` / `>>`,
  `std::hash` specialization, `std::formatter` support
- Rich extras: `split`, `join`, `trim`, `replace`, `toUpper` / `toLower`,
  `startsWith` / `endsWith` / `contains`, `format`, `concat`, `toNumber` / `fromNumber`,
  repetition (`operator*`), regex helpers
- Typed exception hierarchy (`String_Error`, `Invalid_Argument`,
  `Index_Out_Of_Range`, `Empty_String`, `Invalid_Conversion`, `Invalid_Operation`)

### `utf8.h` — `sp::utf8`

Strict validation (rejects truncation, overlongs, surrogates, > U+10FFFF),
codepoint-length, char-indexed substring, per-character split, codepoint
encode/decode, char-aware reverse, terminal `display_width` (CJK = 2 columns),
plus `truncate_by_width` / `pad_by_width` that never split a character.

### `coding.h` — `sp::coding`

Base64 / Base32 / Base58 (Bitcoin alphabet) / Base64URL (RFC 4648), hex,
binary, octal, URL percent-encoding, integer radix 2–36, ROT13 / Caesar
cipher, Morse code.

### `json.h` — `sp::json`

Full JSON (RFC 8259): a `Value` variant type (null / bool / number / string /
array / object, insertion-ordered objects), a strict recursive-descent
`parse` with byte-position error messages, and `dump` in compact or
pretty-printed form. Round-trip safe: `parse(x.dump()) == x`.
Also RFC 6901 JSON Pointer (`pointer` / `set_pointer`), RFC 7386 JSON
Merge Patch (`merge`) and RFC 6902 JSON Patch (`patch`, all six operations).

### `csv.h` — `sp::csv`

Full RFC 4180 documents: `Document` with an optional header row, named
column access, `parse` (quoted separators, doubled quotes, embedded
newlines, CRLF handling), delimiter auto-detection, and `write` with
configurable quoting and line endings. Extends the single-line
`csv::escape` / `csv::split` helpers in `method.h`. `to_json` / `from_json`
convert between documents and JSON object/array forms (lossless round trip).

### `ini.h` — `sp::ini`

INI / configuration files: ordered sections and keys, `;` / `#` comments
(including inline), quoted values with escapes, typed getters
(`get_int` / `get_double` / `get_bool`), `set` / `erase` / `erase_section`,
and a `dump` serializer. The unnamed global section uses `*_global` helpers.

### `url.h` — `sp::url`

RFC 3986 URI handling: `Url` parses and rebuilds scheme / userinfo / host /
port / path / query / fragment (including IPv6 authorities), exposes decoded
query parameters, and resolves relative references with `resolve()`
(dot-segment removal included). `encode` / `decode` wrap percent-encoding.

### `unicode.h` — `sp::unicode`

Code-point level internationalization: `fold_case` (ASCII, Latin-1,
Latin Extended-A, Greek, Cyrillic), `to_fullwidth` / `to_halfwidth`,
`transliterate` (Latin diacritics, ligatures, Greek and Cyrillic → ASCII),
and `decompose` / `compose` (NFD-like / NFC-like for precomposed Latin).
Curated tables, not a full UCD implementation.

### `fmt.h` — `sp::fmt`

Type-safe `format("{} / {1} / {{escaped}}", ...)` with positional and indexed
arguments, alignment / fill / zero-padding, integer bases (`d`, `x`, `X`, `b`,
`o` with `#` prefixes), float precision (`f`, `e`, `g`), signs, and string
precision (UTF-8 aware). Bad syntax or a mismatched spec throws a typed
exception. Argument conversion is header-only; parsing lives in the library.

### `ansi.h` — `sp::ansi`

Terminal escape-sequence helpers: `strip`, `has_ansi`, `visible_width`
(CJK-aware), width-based `truncate` / `pad` that preserve SGR codes, and
`style` / `fg` / `rgb` / `bold` / `underline` / `inverse` / `reset` builders.

### `binary.h` — `sp::binary`

Raw byte parsing built around `sp::String` buffers (embedded NUL bytes are
safe). `to_hex` / `from_hex` (with `0x`, `:` and whitespace tolerant input),
`to_bits` / `from_bits`, a classic `hex_dump` with ASCII sidebar, `printable`,
bit-level helpers (`get_bit` / `set_bit` / `popcount` / `reverse_bits`,
`bit_and` / `bit_or` / `bit_xor` / `bit_not`, byte swaps), pattern scanning
(`find` / `find_all` / `split` / `slice`) and binary-mode `load_file` /
`save_file`. `Reader` is a cursor parser for u8/i8 → u64/i64, f32/f64 in
either byte order (per-call override supported), `peek` / `bytes` / `rest`,
`cstring` / `line` / `read_until`, LEB128 `varint` and zig-zag `svarint`,
`match` / `skip_to` signatures, `seek` / `skip` / `align` with bounds checks.
`Writer` is its chainable counterpart, and `pack<T>()` wraps single values.
Out-of-bounds reads throw `Index_Out_Of_Range`, malformed data throws
`Invalid_Conversion`.

原始字节解析，缓冲区用 `sp::String`（可含 `\0`）：hex / bit 文本互转、
`hex_dump`、可见化、位运算与字节序交换、模式查找与切片、二进制文件读写。
`Reader` 提供大小端整数 / 浮点、varint / svarint、C 字符串、行读取、
签名匹配、对齐与越界检查；`Writer` 可链式写回，`pack<T>()` 打包单个值。
越界抛 `Index_Out_Of_Range`，格式错误抛 `Invalid_Conversion`。

### `file.h` — `sp::file`

Full file-system layer on top of `std::filesystem` (UTF-8 paths survive the
round trip). Queries: `exists` / `is_regular_file` / `is_directory` /
`is_empty` / `equivalent` / `equals`, metadata (`size`, `last_modified` in
Unix seconds, `size_string`, `filename` / `stem` / `extension`), path
resolution (`absolute` / `canonical` / `weakly_canonical` / `relative`,
`current_path`, `temp_directory`, `home_directory`). Lifecycle:
`create_directory(s)`, `remove` / `remove_all`, `copy` / `move` with overwrite,
`touch`, `truncate`. Content: `read_text` / `write_text` / `append_text`,
`read_lines` / `write_lines` (CRLF aware), `read_bytes` / `write_bytes`,
`read_chunk`, `for_each_line`, `write_text_atomic`. Directory `list` returns
sorted `Entry` records (optional recursive) and `find` filters by extension.
`hash()` integrates the checksum module (md5 / sha256 / crc32 / adler32 /
fnv1a64). Queries never throw; filesystem failures are reported as
`Invalid_Operation` (or `false` from lifecycle helpers).

基于 `std::filesystem` 的完整文件操作层（UTF-8 路径无损）：存在性 / 类型判断、
大小与修改时间、文件名 / 扩展名、绝对路径 / 规范化 / 相对路径 / 工作目录、
创建删除复制移动、`touch` / `truncate`、文本与字节读写、逐行读写（兼容 CRLF）、
分块读取、逐行回调、临时文件 + rename 的原子写、目录遍历与按扩展名查找，
以及复用 checksum 模块的文件哈希。查询函数不抛异常；文件系统错误统一为
`Invalid_Operation`（生命周期类函数返回 `false`）。

### `regex.h` — `sp::regex`

`cppString`-friendly ECMAScript regular expressions (POSIX ERE grammar is
available via `Option::extended`). `escape` quotes metacharacters;
`is_match` / `search` / `match` / `find_all` / `count` cover matching, with
`Match` exposing the whole match, byte offset, length and numbered capture
groups. `replace` / `replace_all` expand `$0..$9`, `${n}`, `$&`, `$$` and
`\1`-style references, plus a callback overload; `split` preserves trailing
empty fields. `Option::icase` enables case-insensitive matching. Use the fully
qualified `sp::regex::` prefix if you also have `using namespace std;`.
Invalid patterns throw `Invalid_Argument`, a bad group reference throws
`Invalid_Format`, and a bad group index throws `Index_Out_Of_Range`.

面向 `cppString` 的 ECMAScript 正则封装（也可用 `Option::extended` 切换 POSIX
ERE 语法）：`escape` 转义元字符；`is_match` / `search` / `match` / `find_all` /
`count` 做匹配，`Match` 提供完整匹配、字节偏移、长度与编号捕获组；`replace` /
`replace_all` 支持 `$1`、`${1}`、`$&`、`$$`、`\1` 反向引用及回调重载；
`split` 保留尾部空字段；`Option::icase` 忽略大小写。无效正则抛
`Invalid_Argument`，无效组引用抛 `Invalid_Format`，组下标越界抛
`Index_Out_Of_Range`。与 `std::regex` 同名，建议始终写 `sp::regex::` 全限定。

### `datetime.h` — `sp::datetime`

Timezone-aware date/time on the proleptic Gregorian calendar, with no
time-zone database dependency. `DateTime` carries date, time, milliseconds
and a UTC offset; `parse` accepts ISO 8601 / RFC 3339 (date only, `T` or
space separator, fractional seconds, `Z` / `±HH:MM` / `±HHMM` / `±HH`)
and returns `std::nullopt` on bad input. `format` implements a strftime-like
subset (`%Y %y %m %d %H %I %M %S %p %f %j %a %A %b %B %z %%`), with
`to_iso8601` / `to_rfc3339` shortcuts. `to_unix` / `from_unix` round-trip
epoch seconds (including pre-1970 dates), and `days_from_civil` /
`civil_from_days`, `is_leap_year`, `days_in_month`, `weekday`,
`day_of_year`, `add_days` / `add_seconds`, `diff_seconds` / `diff_days` cover
calendar arithmetic. `utc_now` / `local_now` / `unix_now` read the clock;
`relative` and `duration` produce human-readable strings. Invalid dates
throw `Invalid_Conversion`, out-of-range offsets `Invalid_Argument`.

时区感知的日期时间模块，基于通用格里高利历算法、不依赖时区数据库：
`DateTime` 含年月日时分秒、毫秒与 UTC 偏移；`parse` 解析 ISO 8601 / RFC 3339
（纯日期、`T` 或空格分隔、小数秒、`Z` / `±HH:MM` / `±HHMM` / `±HH`），
非法输入返回 `std::nullopt`；`format` 支持类 strftime 占位符，
`to_iso8601` / `to_rfc3339` 为快捷方式；`to_unix` / `from_unix` 与
Unix 时间戳互转（支持 1970 年以前），另有闰年、月天数、星期、年内天数、
日期加减与差值、`utc_now` / `local_now` / `unix_now`、相对时间与时长格式化。
无效日期抛 `Invalid_Conversion`，越界时区偏移抛 `Invalid_Argument`。

### `table.h` — `sp::table`

ASCII table rendering with per-column alignment. Column widths use
`utf8::display_width`, so CJK text stays aligned in a terminal.

### `argparse.h` — `sp::ArgParser`

Small command-line parser: `--long` / `-s` options, `--key=value`, combined
short flags (`-abc`), `--` terminator, positionals and an auto-generated
`help()` text.

### `method.h` — `sp::`

Everyday text tools: `join`, `split_lines`, `split_by_any`, `pad_left/right`,
`center`, `capitalize`, `title_case`, `swap_case`, `snake/kebab/camel/pascal
case`, `wildcard_match`, `truncate`, `slug`, `chunk`, `ngrams`, `is_palindrome`,
`soundex`, `levenshtein`, `hamming`, `similarity`, `dice_coefficient`,
`is_anagram`, `common_prefix/suffix`, `word_count`, `wrap`, `indent`, `dedent`,
`squeeze`, `count`, `expand_tabs`, `reverse_words`, `diff_lines` (LCS) and
`unified_diff` (standard patch text),
`thousands_sep`, `human_size`, `human_duration`, `to_roman` / `from_roman`,
`render` (`{{key}}` templates), `generate_random`, `uuid_v4`, `to_bool`,
case-insensitive `icontains` / `istarts_with` / `iends_with`,
`json::escape/unescape`, `csv::escape/split`, `html::escape/unescape`,
plus simple file I/O (`read_file` / `read_lines` / `write_file`).

### `path.h` — `sp::path`

Pure-string path handling (no filesystem access, normalized to `/`):
`join`, `normalize` (drive letters, UNC, `.`, `..`), `filename`, `stem`,
`extension`, `parent`, `is_absolute`, `relative`.

### `checksum.h` — `sp::checksum`

CRC-32 (compile-time table), FNV-1a 64, Adler-32, MD5 (legacy, cache keys
only), SHA-256 and HMAC-SHA-256 — raw bytes and hex forms.

### `validate.h` — `sp::validate`

`is_ipv4`, `is_ipv6` (with `::` compression and embedded IPv4 tails),
`is_email`, `is_uuid`, `is_url`, `is_mac_address`, `is_hex_color`,
`is_luhn`, `is_ascii`, `is_strong_password`, `is_iban` (mod-97),
`is_jwt`, `is_base64`, `is_hostname`, `is_json`, `is_date` (calendar-valid).

### `semver.h` — `sp::semver`

Strict semver 2.0.0 validation and precedence comparison
(numeric identifiers, prerelease rules, build metadata ignored),
plus `bump_major` / `bump_minor` / `bump_patch`.

## Quick start

```cpp
#include "cppString.h"
#include "method.h"
#include "coding.h"
#include "utf8.h"
#include "json.h"
#include "csv.h"
#include "url.h"
#include "fmt.h"
#include "ansi.h"
#include "binary.h"
#include "file.h"
#include "regex.h"
#include "datetime.h"

using namespace sp;

cppString s = "hello world";
s.toUpper();                       // "HELLO WORLD"

auto parts = cppString{"a,b,c"}.split(',');   // {"a", "b", "c"}

cppString enc = coding::toBase64Url("secret"); // "c2VjcmV0"
cppString id  = uuid_v4();          // "f47ac10b-58cc-4372-a567-0e02b2c3d479"

cppString cjk = "中文测试";
utf8::length(cjk);                 // 4 (characters, not bytes)
utf8::sub(cjk, 1, 2);              // "文测"

auto doc  = csv::parse(cppString{"name,age\nAda,36\n"});
auto cell = doc.get(0, "age");     // "36"
auto uri  = url::Url{"https://example.com/a/b?q=1"};
auto next = uri.resolve("../c");   // "https://example.com/a/c"

cppString line = fmt::format("{:>8} {:#x} {:.2f}", "id", 255, 3.14159);
cppString safe = ansi::truncate(ansi::bold(cjk), 6);  // width-safe, style closed

binary::Writer w;
w.u16(0x4D5A).cstring(String{"cppString"}).varint(300);
binary::Reader r{w.data()};
auto magic = r.u16();               // 0x4D5A
auto name  = r.cstring();           // "cppString"
auto count = r.varint();            // 300
cppString dump = binary::hex_dump(w.data());

file::write_text("notes.txt", cppString{"first\nsecond\n"});
auto lines = file::read_lines("notes.txt");   // {"first", "second"}
cppString digest = file::hash("notes.txt", "sha256");
for (const auto& e : file::list(".")) { /* sorted entries with size/type */ }

cppString swapped = sp::regex::replace_all("2024-02-29", R"((\d{4})-(\d{2})-(\d{2}))",
                                            "${3}/${2}/${1}");   // "29/02/2024"
auto fields = sp::regex::split("a, b ,  c", R"(\s*,\s*)");     // {"a", "b", "c"}

auto stamp = sp::datetime::parse("2024-02-29T12:34:56+08:00");
cppString shown = sp::datetime::format(*stamp, "%Y-%m-%d %H:%M %z");
cppString when  = sp::datetime::relative(sp::datetime::to_unix(*stamp),
                                         sp::datetime::unix_now());
```

## Building

Requires a C++23 compiler (developed with MSVC v145 / VS 2026, `/std:c++latest`).
The library is plain `.h` / `.cpp` pairs — add them to your project and go.
No third-party dependencies; Windows targets link nothing extra.

To build and run the test suite (1002 assertions) from a VS developer prompt:

```
cl /std:c++latest /EHsc /W3 cppString.cpp method.cpp utf8.cpp coding.cpp ^
   path.cpp checksum.cpp validate.cpp semver.cpp json.cpp table.cpp ^
   argparse.cpp csv.cpp ini.cpp url.cpp unicode.cpp fmt.cpp ansi.cpp ^
   binary.cpp file.cpp regex.cpp datetime.cpp testCppString.cpp
testCppString.exe
```

Or open the local `Project1.slnx` in Visual Studio and press F5
(`testCppString.cpp` is a local-only test harness, not published).

## Repository layout

The repository root is flat: every published source is a `.h` / `.cpp` pair at
the top level. The local `Project1/` Visual Studio workspace (project/solution
files, IDE settings, test harness, `release.bat`) is **never committed**; the
release script mirrors the library sources into the root before each push.

仓库根目录为扁平结构，只发布头文件与实现。本地 `Project1/`（VS 工程、解决方案、
测试文件、`release.bat`）不入库，发布脚本会在每次推送前把库源码同步到根目录。

```
├── cppString.h / .cpp   # core class
├── utf8.h / .cpp        # UTF-8 validation & manipulation, display width
├── coding.h / .cpp      # base64/base32/base58/hex/binary/octal/url, radix, ciphers, morse
├── method.h / .cpp      # text utilities, file I/O, json/csv/html helpers, diff
├── path.h / .cpp        # path manipulation
├── checksum.h / .cpp    # CRC-32, FNV-1a, Adler-32, MD5, SHA-256, HMAC-SHA-256
├── validate.h / .cpp    # IP/email/UUID/URL/MAC/color/Luhn validation
├── semver.h / .cpp      # semantic versioning
├── json.h / .cpp        # JSON Value, parser, serializer, pointer, merge patch
├── table.h / .cpp       # ASCII table rendering (CJK-aware)
├── argparse.h / .cpp    # command-line argument parser
├── csv.h / .cpp         # RFC 4180 CSV documents
├── ini.h / .cpp         # INI configuration files
├── url.h / .cpp         # RFC 3986 URI parsing and resolution
├── unicode.h / .cpp     # case folding, width, transliteration, NFC/NFD
├── fmt.h / .cpp         # type-safe text formatting
├── ansi.h / .cpp        # ANSI/terminal escape helpers
├── binary.h / .cpp      # raw byte parsing: Reader/Writer, hex dump, bit ops
├── file.h / .cpp        # filesystem ops: metadata, dirs, text/byte I/O, hashes
├── regex.h / .cpp       # regular expressions: match, capture, replace, split
├── datetime.h / .cpp    # date/time: ISO 8601, Unix time, calendar arithmetic
├── README.md
├── LICENSE
└── .gitignore
```

The 1002-assertion test suite (`testCppString.cpp`) is used by the local build
gate but is intentionally kept out of the published repository.

## License

[MIT](LICENSE) © 2025 Ender_
