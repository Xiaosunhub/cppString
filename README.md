# cppString

A modern C++23 string utility library — a `std::string`-compatible core class
(`sp::cppString`) plus a collection of focused helper modules for UTF-8,
encoding, data exchange (JSON / CSV / INI / URI), internationalization,
validation, paths, checksums, versioning, formatting, terminal output,
binary parsing, file system operations, regular expressions, date/time,
external command execution, and everyday text work.

一个现代 C++23 字符串工具库：与 `std::string` 对齐的核心类 `sp::cppString`，
外加 UTF-8、编解码、数据交换（JSON / CSV / INI / URI）、国际化处理、
校验、路径、哈希、语义化版本、格式化、终端输出、二进制解析、
文件系统操作、正则表达式、日期时间、外部命令执行、文本处理等一系列独立模块。

当前版本：`5.6.0`（头文件与实现分目录，`exec` 新增简洁接口）。
Current release: **5.6.0** — headers/implementations split and a short-form `exec` API.

> **5.6.0** — the published tree is split into `include/cppString/*.h`
> (headers only — the only thing users include) and `src/*.cpp`
> (implementations, never installed). `sp::exec` gains `execute(command)` /
> `execute(Actuator, command)`, returning a bool, and `getExecResult(command)` /
> `getExecResult(Actuator, command)`, returning the merged execution transcript.
> The default actuator is cmd.exe on Windows (/bin/sh on POSIX). C++ forbids a
> defaulted parameter in front of a non-defaulted one, so two overloads keep the
> actuator-first calling style. 14 new assertions (1103 total).
>
> **5.6.0**：发布目录拆分为 `include/cppString/*.h`（只放头文件，用户只需包含它）
> 与 `src/*.cpp`（实现文件，不安装）。`sp::exec` 新增 `execute(command)` /
> `execute(Actuator, command)` 返回 bool，以及 `getExecResult(command)` /
> `getExecResult(Actuator, command)` 返回合并后的执行过程输出。默认执行器在
> Windows 为 cmd.exe（POSIX 为 /bin/sh）；因 C++ 不允许默认参数位于非默认参数
> 之前，改用两个重载保留「执行器在前」的调用方式。新增 14 项断言（共 1103 项）。
>
> **5.5.0** — adds `exec.h` / `exec.cpp` (`sp::exec`): execute external
> commands and capture raw stdout, stderr and the exit code. `run` / `cmd` /
> `powershell` go through a shell, `invoke(argv)` starts a program directly.
> `Options` covers shell choice, working directory, timeout (on Windows a job
> object kills the whole child process tree) and stderr merging; helpers are
> `capture`, `exists`, `quote` and `build_command`. No third-party dependency,
> 21 new assertions (1089 total).
>
> **5.5.0**：新增 `exec.h` / `exec.cpp`（`sp::exec`），用于执行 cmd / PowerShell /
> shell 命令并捕获原始 stdout、stderr 与退出码。`run` / `cmd` / `powershell` 经
> shell 执行，`invoke(argv)` 不经 shell 直接启动程序；`Options` 支持选择 shell、
> 工作目录、超时（Windows 用 Job Object 终止整棵进程树）与 stderr 合并，
> 另有 `capture` / `exists` / `quote` / `build_command`。无第三方依赖，
> 新增 21 项断言（共 1089 项）。
>
> **5.4.2** — bug-fix release: no new modules and no new API. `json::dump()`
> now rejects NaN/Infinity instead of emitting non-JSON `nan` / `inf`; a URI
> keeps a present-but-empty authority (`file:///tmp/a`, `http:///p`);
> `csv::parse` preserves CRLF inside quoted fields and round-trips a blank
> line as a record with zero fields; `datetime::duration` and
> `datetime::relative` no longer overflow on `INT64_MIN`. 23 regression
> assertions were added (1068 total).
>
> **5.4.2**：缺陷修复版本，不新增模块、不新增接口。修复 JSON 把 NaN/Infinity
> 序列化成非法 `nan` / `inf`、URL 空 authority 丢失 `//`、CSV 引号字段内
> CRLF 被折叠成 LF、空行往返不一致，以及 `datetime::duration` /
> `relative` 在 `INT64_MIN` 下的有符号溢出。新增 23 项回归断言（共 1068 项）。
>
> **5.4.1** — all library sources now live in `include/cppString/`; CMake
> install/export rules and a sample vcpkg portfile were added; the export macro
> no longer hardcodes `__declspec` (so non-MSVC toolchains work), include guards
> use project-owned identifiers and every file carries an SPDX header. The
> calendar conversion in `datetime.cpp` was rewritten independently.
> New helpers, **no new modules**: `utf8::has_bom / strip_bom / add_bom`,
> `ljust / rjust / repeat`, `validate::is_slug / is_ipv4_cidr`.
> Source provenance: [ORIGINALITY.md](ORIGINALITY.md).
>
> **5.4.1**：库源码统一移入 `include/cppString/`；新增 CMake 安装/导出与 vcpkg
> 端口示例；修正跨平台导出宏、include guard 与 SPDX 版权头；日期换算算法独立重写。
> 新增函数（不新增模块）：UTF-8 BOM 处理、左右对齐与重复、slug / IPv4 CIDR 校验。
> 源码来源说明见 `ORIGINALITY.md`。

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

### `exec.h` — `sp::exec`

Run external commands and capture raw stdout / stderr plus the exit code.
`run` / `cmd` / `powershell` go through a shell (cmd.exe, PowerShell via
`-EncodedCommand`, or sh / bash / pwsh on POSIX); `invoke(argv)` starts an
executable directly without any shell. `Options` selects the shell, working
directory, a timeout (0 = wait forever) and stderr merging; `Result` exposes
`out` / `err` / `exit_code` / `started` / `timed_out` / `ok()` / `output()`.
Helpers: `capture` (stdout only, throws on start failure or timeout),
`exists` (PATH lookup), `quote` (platform shell quoting) and `build_command`
(the exact command line, for logging). Windows uses `CreateProcessW` with
anonymous pipes and a job object so a timeout kills the whole child tree;
POSIX uses `fork` / `execvp` with `poll`. No third-party dependency.

Short-form API: `execute(command)` is a bool shortcut — true only when the
process started, did not time out and exited 0; `getExecResult(command)`
returns the full transcript with stderr merged into stdout. Both also have an
actuator-first overload, `execute(Actuator, command)` and
`getExecResult(Actuator, command)`. The default actuator is cmd.exe on Windows
and /bin/sh on POSIX; explicit values are `cmd.exe`, `powershell.exe`, `pwsh`,
`sh`, `bash` or `auto` (a path and `.exe` suffix are accepted). An unknown
actuator throws `Invalid_Argument`. C++ does not allow a default argument
before a non-defaulted one, so the default is provided by the one-argument
overload rather than by `execute(Actuator = "cmd.exe", command)`.

**Security**: never concatenate untrusted input into a shell command — use
`invoke()` with an argv vector. Captured text is raw bytes: Windows console
programs usually emit the system OEM code page, not UTF-8.

执行外部命令并捕获原始 stdout / stderr 与退出码。`run` / `cmd` /
`powershell` 经 shell 执行（Windows cmd.exe、PowerShell 用 `-EncodedCommand`；
POSIX 用 sh / bash / pwsh），`invoke(argv)` 不经过 shell 直接启动程序。
`Options` 可指定 shell、工作目录、超时（0 表示不限时）与 stderr 合并；
`Result` 提供 `out` / `err` / `exit_code` / `started` / `timed_out` /
`ok()` / `output()`。另有 `capture`（只取 stdout）、`exists`（PATH 查找）、
`quote`（按平台转义参数）、`build_command`（返回实际命令行便于记录日志）。
Windows 用 `CreateProcessW` + 匿名管道 + Job Object（超时可终止整棵进程树），
POSIX 用 `fork` / `execvp` + `poll`，无第三方依赖。

简洁接口：`execute(command)` 返回 bool（进程成功启动、未超时且退出码为 0 才为
true）；`getExecResult(command)` 返回合并了 stderr 的完整执行过程输出。两者另有
「执行器在前」的重载 `execute(Actuator, command)` 与
`getExecResult(Actuator, command)`。默认执行器在 Windows 为 cmd.exe、POSIX 为
/bin/sh；显式可选 `cmd.exe`、`powershell.exe`、`pwsh`、`sh`、`bash`、`auto`
（允许带路径与 `.exe` 后缀），未知执行器抛 `Invalid_Argument`。C++ 不允许默认
参数出现在非默认参数之前，因此用单参数重载提供默认值，而不是
`execute(Actuator = "cmd.exe", command)`。

**安全提示**：不要把不可信输入拼进 shell 命令，应改用 `invoke()` 传 argv。
捕获内容是原始字节：Windows 控制台程序通常输出系统 OEM 代码页而非 UTF-8。

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
#include <cppString/cppString.h>
#include <cppString/method.h>
#include <cppString/coding.h>
#include <cppString/utf8.h>
#include <cppString/json.h>
#include <cppString/csv.h>
#include <cppString/url.h>
#include <cppString/fmt.h>
#include <cppString/ansi.h>
#include <cppString/binary.h>
#include <cppString/file.h>
#include <cppString/regex.h>
#include <cppString/datetime.h>

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

Requires a C++23 compiler and CMake 3.20+. No third-party dependencies.

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
cmake --install build --prefix <prefix>
```

Consumers include headers with the package prefix and link the exported
target:

```cmake
find_package(cppString CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE cppString::cppString)
```

```cpp
#include <cppString/cppString.h>
#include <cppString/json.h>
```

Install places headers in `<prefix>/include/cppString/` (and no `.cpp` files),
the library in `<prefix>/lib/`, the package config in
`<prefix>/lib/cmake/cppString/` and the license in
`<prefix>/share/cppString/copyright`. The `src/` tree is used only to build the
library and is not part of the installed package.

### vcpkg / 提交 vcpkg

An overlay / upstream port can use `vcpkg_cmake_config_fixup`:

```cmake
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO Xiaosunhub/cppString
    REF v5.6.0
    SHA512 <sha512 of the release tarball>
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/cppString)
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
```

Port names must be lowercase (for example `cppstring`). Reviewers usually ask
about licensing and provenance — see [ORIGINALITY.md](ORIGINALITY.md).

构建需要 C++23 编译器与 CMake 3.20+，无第三方依赖；安装后用
`#include <cppString/xxx.h>`，链接 `cppString::cppString`。
1103 项断言的本地测试套件 `testCppString.cpp` 位于 `Project1/Project1/`
（不入库），由本地 `release.bat` 门禁脚本在每次发布前编译运行。

## Repository layout

Public headers live in `include/cppString/`, implementations in `src/`. The
public include tree contains **only `.h` files**: consumers add just
`<prefix>/include` and write `#include <cppString/cppString.h>`; no `.cpp` is
installed or visible to them. The local `Project1/` Visual Studio workspace
(project/solution, IDE settings, test harness, `release.bat`) is **never
committed**; the release script mirrors headers into `include/cppString/` and
implementations into `src/` before each push. The local workspace itself uses
the same split.

头文件统一放在 `include/cppString/`，实现文件统一放在 `src/`。公开 include
目录**只含 `.h`**：用户只需把 `<prefix>/include` 加入搜索路径并写
`#include <cppString/cppString.h>`，无需（也不会）看到任何 `.cpp`。
本地 `Project1/`（VS 工程、测试文件、`release.bat`）不入库，本地工程同样按
头文件 / 实现分目录，`release.bat` 发布前分别镜像到两个目录。

```
├── CMakeLists.txt                  # build, install, package export
├── cmake/
│   └── cppStringConfig.cmake.in
├── include/cppString/             # public headers only (22 × .h)
│   ├── cppString.h                # core class, version macros, exceptions
│   ├── utf8.h                     # UTF-8, display width, BOM helpers
│   ├── coding.h                   # base64/base32/base58/hex/radix, ciphers, morse
│   ├── method.h                   # text utilities, diff, padding/repetition
│   ├── path.h                     # path manipulation
│   ├── checksum.h                 # CRC-32, FNV-1a, Adler-32, MD5, SHA-256, HMAC
│   ├── validate.h                 # IP/email/UUID/URL/MAC/Luhn/slug/CIDR
│   ├── semver.h                   # semantic versioning
│   ├── json.h                     # JSON Value, parser, pointer, merge patch
│   ├── table.h                    # ASCII table rendering (CJK-aware)
│   ├── argparse.h                 # command-line argument parser
│   ├── csv.h                      # RFC 4180 CSV documents
│   ├── ini.h                      # INI configuration files
│   ├── url.h                      # RFC 3986 URI parsing and resolution
│   ├── unicode.h                  # case folding, width, transliteration, NFC/NFD
│   ├── fmt.h                      # type-safe text formatting
│   ├── ansi.h                     # ANSI/terminal escape helpers
│   ├── binary.h                   # raw byte parsing: Reader/Writer, hex dump
│   ├── file.h                     # filesystem ops, text/byte I/O, hashes
│   ├── regex.h                    # regular expressions: match, replace, split
│   ├── datetime.h                 # ISO 8601, Unix time, calendar arithmetic
│   └── exec.h                     # external command execution, output capture
├── src/                           # 22 matching .cpp files (never installed)
├── ORIGINALITY.md                 # source provenance / 原创性声明
├── README.md
├── LICENSE
└── .gitignore
```

The 1103-assertion test suite (`testCppString.cpp`) is used by the local build
gate but is intentionally kept out of the published repository.

## License / 许可

[MIT](LICENSE) © 2025-2026 Ender_ — source provenance: [ORIGINALITY.md](ORIGINALITY.md).
源码来源与合规说明见 `ORIGINALITY.md`。
