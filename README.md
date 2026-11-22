# cppString

A modern C++23 string utility library — a `std::string`-compatible core class
(`cppstr::cppString`) plus a collection of focused helper modules for UTF-8,
encoding, data exchange (JSON / CSV / INI / TOML / URI), HTTP web reading, internationalization,
validation, paths, checksums, versioning, formatting, terminal output,
binary parsing, file system operations, regular expressions, date/time,
external command execution, cross-language code execution, and everyday text work.

一个现代 C++23 字符串工具库：与 `std::string` 对齐的核心类 `cppstr::cppString`，
外加 UTF-8、编解码、数据交换（JSON / CSV / INI / TOML / URI）、网页读取、国际化处理、
校验、路径、哈希、语义化版本、格式化、终端输出、二进制解析、
文件系统操作、正则表达式、日期时间、外部命令执行、跨语言代码执行、文本处理等一系列独立模块。

当前版本：`7.1.0`（新增 `langcvt` 多语言代码执行模块）。
Current release: **7.1.0** — adds the `langcvt` code-execution adapters.

> **7.1.0** — adds a new public header group `langcvt/` with `langcvt.h`,
> `pycvt.h`, `Javacvt.h` and `jscvt.h`. `cppstr::pycvt`, `cppstr::javacvt` and
> `cppstr::jscvt` execute Python, Java and JavaScript source code, returning a
> `cppstr::langcvt::Result` with exit code, stdout, stderr, `started` and
> `timed_out`. Source text is passed to `exec::invoke()` as an argv element —
> never through a shell — and Java uses the Java 11+ single-file source
> launcher. 12 new assertions (1291 total).
>
> **7.1.0**：新增公开头文件组 `langcvt/`，包含 `langcvt.h`、`pycvt.h`、
> `Javacvt.h` 与 `jscvt.h`。`cppstr::pycvt`、`cppstr::javacvt` 与
> `cppstr::jscvt` 可执行 Python、Java 与 JavaScript 源码，返回包含退出码、
> 标准输出、标准错误、`started` 与 `timed_out` 的
> `cppstr::langcvt::Result`。源码通过 `exec::invoke()` 作为 argv 参数传递，
> 不经过 shell；Java 使用 Java 11+ 单文件源码启动方式。新增 12 项断言（共 1291 项）。
>
> **7.0.0** — breaking API change: every public symbol now lives in `cppstr`;
> the former short namespace and any compatibility alias are gone. Headers,
> implementations, the test harness, examples and documentation were updated
> consistently. Migrate by using `cppstr::cppString` and qualifying helpers as
> `cppstr::*`. No files or assertions were added (1279 total).
>
> **7.0.0**：破坏性 API 变更：所有公开符号现统一位于 `cppstr`，旧短命名空间及
> 兼容别名均已移除。头文件、实现、测试、示例与文档全部同步更新。升级时使用
> `cppstr::cppString`，辅助模块以 `cppstr::*` 限定。未新增文件或断言（共 1279 项）。
>
> **6.1.0** — adds `csurl.h` / `csurl.cpp` (`cppstr::csurl`), a small synchronous
> HTTP/HTTPS client for reading web pages and APIs. Windows uses the built-in
> WinHTTP API (HTTP + HTTPS); other platforms use a compact POSIX socket client
> over HTTP. There is no curl and no third-party dependency. URL parsing reuses
> `cppstr::url`; `get()` returns a `Response` (status / body / content-type / final
> URL / error) and never throws on transport failure, while `fetch()` returns
> the body on 2xx or throws. Redirects, timeouts, a body cap and a custom
> user-agent are supported. 21 new assertions (1279 total).
>
> **6.1.0**：新增 `csurl.h` / `csurl.cpp`（`cppstr::csurl`），用于读取网页与调用 API
> 的轻量同步 HTTP/HTTPS 客户端。Windows 使用系统内置 WinHTTP（HTTP + HTTPS），
> 其他平台使用精简的 POSIX socket 客户端（HTTP）。不依赖 curl 或任何第三方库。
> URL 解析复用 `cppstr::url`；`get()` 返回 `Response`（状态码 / 正文 / Content-Type /
> 最终 URL / 错误），传输失败不抛异常；`fetch()` 在 2xx 时返回正文，否则抛异常。
> 支持重定向、超时、正文大小上限与自定义 User-Agent。新增 21 项断言（共 1279 项）。
>
> **6.0.0** — a major release introducing `macro.h`, a header-only macro
> layer built on the library version macros. It provides two-stage stringizing
> and token pasting (`STR` / `XSTR` / `CAT` / `XCAT` / `CAT3` / `XCAT3`), an
> integer `CPPSTRING_VERSION_CODE` with `AT_LEAST` / `LESS_THAN` / `EQ` gates,
> compiler / platform / build-mode detection, portable attributes (`NODISCARD`,
> `DEPRECATED`, `LIKELY` / `UNLIKELY`, `FALLTHROUGH`, `FORCEINLINE`, ...),
> source-location diagnostics (`FUNC` / `HERE` / `FILE_LINE` / `VARSTR`), and
> release-active assertion / helper macros. All macros are self-contained —
> there is no `macro.cpp` and no extra link dependency. 49 new assertions
> (1258 total).
>
> **6.0.0**：major 版本，新增纯头文件 `macro.h` 编译期宏工具层，与库版本宏打通。
> 提供两阶段字符串化与 token 连接（`STR` / `XSTR` / `CAT` / `XCAT` / `CAT3` /
> `XCAT3`）、整数版本码 `CPPSTRING_VERSION_CODE` 与 `AT_LEAST` / `LESS_THAN` /
> `EQ` 门控、编译器 / 平台 / 构建模式探测、可移植属性（`NODISCARD`、`DEPRECATED`、
> `LIKELY` / `UNLIKELY`、`FALLTHROUGH`、`FORCEINLINE` 等）、源码位置诊断
> （`FUNC` / `HERE` / `FILE_LINE` / `VARSTR`）以及在 release 构建仍生效的
> 断言与辅助宏。所有宏自包含，无 `macro.cpp`、无额外链接依赖。新增 49 项断言
> （共 1258 项）。
>
> **5.7.0** — adds `toml.h` / `toml.cpp` (`cppstr::toml`), a TOML 1.0 core-subset
> reader and deterministic writer built on the existing `cppstr::json::Value` tree.
> It handles comments, bare / quoted / dotted keys, `[table]` headers, basic /
> literal / multiline strings (including unicode escapes), decimal / hex /
> octal / binary integers, floats, `inf` / `nan`, booleans, arrays and inline
> tables. Date-time literals are kept as strings, array-of-tables is rejected,
> and integers beyond double precision are refused rather than silently rounded.
> 57 new assertions (1209 total).
>
> **5.7.0**：新增 `toml.h` / `toml.cpp`（`cppstr::toml`），基于已有 `cppstr::json::Value`
> 数据模型实现 TOML 1.0 核心子集的解析与确定性序列化。支持注释、裸键 /
> 引号键 / 点分键、`[table]` 表头、基本 / 字面 / 多行字符串（含 unicode 转义）、
> 十进制 / 十六进制 / 八进制 / 二进制整数、浮点数、`inf` / `nan`、布尔、
> 数组与内联表。日期时间字面量按字符串保留，`[[数组表]]` 明确拒绝，超出
> double 精确表示范围的整数直接拒绝而非静默丢精度。新增 57 项断言（共 1209 项）。
>
> **5.6.10** — fixes inherited from 5.6.9: `semver::bump_major` / `bump_minor` / `bump_patch` incremented a parsed `long long` component without a guard; a version at LLONG_MAX overflowed (undefined behaviour) and produced a smaller version instead of being rejected.
> 3 regression assertions were added (1152 total).
>
> **5.6.10**：修复 5.6.9 遗留缺陷：`semver::bump_major` / `bump_minor` / `bump_patch` 直接自增解析后的 `long long` 分量且无保护；分量取到 LLONG_MAX 时发生有符号溢出（未定义行为），还会产出更小的版本号而不是拒绝。 新增 3 项回归断言（共 1152 项）。
>
> **5.6.9** — fixes inherited from 5.6.8: `cppString::substr_between` threw a misleading "empty string" error for empty inputs and equal bounds, used the wrong exception type for real out-of-range bounds, and let an explicit count read past end_pos.
> 5 regression assertions were added (1149 total).
>
> **5.6.9**：修复 5.6.8 遗留缺陷：`cppString::substr_between` 对空串和相等区间抛出误导性的「空字符串」异常、对真正越界使用错误异常类型，且显式 count 会越过 end_pos 读取。 新增 5 项回归断言（共 1149 项）。
>
> **5.6.8** — fixes inherited from 5.6.7: `file::read_lines` stripped a trailing CR even from the final line of a file that has no newline, so a CR that is part of the data was silently lost on a write/read round trip.
> 1 regression assertions were added (1144 total).
>
> **5.6.8**：修复 5.6.7 遗留缺陷：`file::read_lines` 会剥离文件末行（无换行符）结尾的 CR，导致数据中本应保留的 CR 在写入/读取往返时丢失。 新增 1 项回归断言（共 1144 项）。
>
> **5.6.7** — fixes inherited from 5.6.6: `startsWith` / `endsWith` threw `Invalid_Argument` when the prefix or suffix was longer than the string (and on empty strings for the char overloads), the opposite of standard string semantics and a common crash source in callers.
> 8 regression assertions were added (1143 total).
>
> **5.6.7**：修复 5.6.6 遗留缺陷：`startsWith` / `endsWith` 在前缀或后缀长于原串时抛 `Invalid_Argument`（char 重载对空串也抛异常），与标准字符串语义相反，调用方极易误触发。 新增 8 项回归断言（共 1143 项）。
>
> **5.6.6** — fixes inherited from 5.6.5: `cppString::at` / `substr` leaked `std::out_of_range` from the underlying string while every other range-checked API in the library throws `cppstr::Index_Out_Of_Range`.
> 3 regression assertions were added (1135 total).
>
> **5.6.6**：修复 5.6.5 遗留缺陷：`cppString::at` / `substr` 把底层字符串的 `std::out_of_range` 直接抛给调用方，而库内其它越界接口统一抛 `cppstr::Index_Out_Of_Range`。 新增 3 项回归断言（共 1135 项）。
>
> **5.6.5** — fixes inherited from 5.6.4: `utf8::sub(s, start, len)` tested `start + len > count`; an enormous length wrapped the addition, bypassed the bounds check and produced a wrong substring instead of the documented `Index_Out_Of_Range`.
> 3 regression assertions were added (1132 total).
>
> **5.6.5**：修复 5.6.4 遗留缺陷：`utf8::sub(s, start, len)` 用 `start + len > count` 判断，超大长度使加法回绕，绕过越界检查并返回错误子串，而不是文档承诺的 `Index_Out_Of_Range`。 新增 3 项回归断言（共 1132 项）。
>
> **5.6.4** — fixes inherited from 5.6.3: `csv::from_json(v, false)` always promoted the first JSON array to a header, so a headerless document lost its first data row and `to_json`/`from_json` were not inverse as documented.
> 3 regression assertions were added (1129 total).
>
> **5.6.4**：修复 5.6.3 遗留缺陷：`csv::from_json(v, false)` 总把第一个 JSON 数组当作表头，导致无表头文档丢失首行数据，`to_json`/`from_json` 不符合文档承诺的互逆关系。 新增 3 项回归断言（共 1129 项）。
>
> **5.6.3** — fixes inherited from 5.6.2: `csv::parse` silently appended characters that follow a closing quote to the field (`"a"x` became `ax`), which violates RFC 4180 and hides malformed input.
> 2 regression assertions were added (1126 total).
>
> **5.6.3**：修复 5.6.2 遗留缺陷：`csv::parse` 把闭合引号之后的字符静默拼进字段（如 `"a"x` 变成 `ax`），违反 RFC 4180 且掩盖了非法输入。 新增 2 项回归断言（共 1126 项）。
>
> **5.6.2** — fixes inherited from 5.6.1: `coding::fromBinary` / `fromOctal` rejected empty input even though their documented contract (length must be a multiple of 8 / 3) allows it, breaking `toBinary`/`toOctal` round trips for empty data.
> 4 regression assertions were added (1124 total).
>
> **5.6.2**：修复 5.6.1 遗留缺陷：`coding::fromBinary` / `fromOctal` 拒绝空输入，而其文档契约只要求长度为 8 / 3 的倍数，导致空数据的 `toBinary`/`toOctal` 往返失败。 新增 4 项回归断言（共 1124 项）。
>
> **5.6.1** — bug-fix release, no new modules and no new API:
> `json::parse` now keeps one entry per object key (last duplicate wins)
> instead of emitting duplicate keys; JSON Pointer rejects malformed `~`
> escapes (`~2`, trailing `~`) as RFC 6901 requires;
> `url::set_query_param` collapses a repeated key instead of leaving
> several identical pairs; `validate::is_ipv4_cidr` rejects leading-zero
> masks such as `/01` and `/00`; and the `csv::parse` header comment was
> corrected to state that CRLF inside quoted fields is preserved.
> 17 regression assertions were added (1120 total).
>
> **5.6.1**：缺陷修复版本，不新增模块、不新增接口。`json::parse` 现在对每个
> 对象键只保留一项（重复键取最后一个），不再输出重复键；JSON Pointer 按
> RFC 6901 拒绝非法 `~` 转义（`~2`、结尾孤立 `~`）；`url::set_query_param`
> 会合并重复键，不再留下多条相同的 `key=value`；`validate::is_ipv4_cidr`
> 拒绝 `/01`、`/00` 这类带前导零的掩码；同时修正 `csv::parse` 头文件注释，
> 明确引号字段内的 CRLF 是原样保留的。新增 17 项回归断言（共 1120 项）。
>
> **5.6.0** — the published tree is split into `include/cppString/*.h`
> (headers only — the only thing users include) and `src/*.cpp`
> (implementations, never installed). `cppstr::exec` gains `execute(command)` /
> `execute(Actuator, command)`, returning a bool, and `getExecResult(command)` /
> `getExecResult(Actuator, command)`, returning the merged execution transcript.
> The default actuator is cmd.exe on Windows (/bin/sh on POSIX). C++ forbids a
> defaulted parameter in front of a non-defaulted one, so two overloads keep the
> actuator-first calling style. 14 new assertions (1103 total).
>
> **5.6.0**：发布目录拆分为 `include/cppString/*.h`（只放头文件，用户只需包含它）
> 与 `src/*.cpp`（实现文件，不安装）。`cppstr::exec` 新增 `execute(command)` /
> `execute(Actuator, command)` 返回 bool，以及 `getExecResult(command)` /
> `getExecResult(Actuator, command)` 返回合并后的执行过程输出。默认执行器在
> Windows 为 cmd.exe（POSIX 为 /bin/sh）；因 C++ 不允许默认参数位于非默认参数
> 之前，改用两个重载保留「执行器在前」的调用方式。新增 14 项断言（共 1103 项）。
>
> **5.5.0** — adds `exec.h` / `exec.cpp` (`cppstr::exec`): execute external
> commands and capture raw stdout, stderr and the exit code. `run` / `cmd` /
> `powershell` go through a shell, `invoke(argv)` starts a program directly.
> `Options` covers shell choice, working directory, timeout (on Windows a job
> object kills the whole child process tree) and stderr merging; helpers are
> `capture`, `exists`, `quote` and `build_command`. No third-party dependency,
> 21 new assertions (1089 total).
>
> **5.5.0**：新增 `exec.h` / `exec.cpp`（`cppstr::exec`），用于执行 cmd / PowerShell /
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

### Core class — `cppstr::cppString` (`cppString.h`)

- Full `std::string` parity interface: `append` / `insert` / `erase` / `assign` /
  `resize` / `compare` / `rfind` / `find_first_of` / `find_first_not_of` / `substr` …
- Iterators (works with `std::ranges`), `operator<=>`, stream `<<` / `>>`,
  `std::hash` specialization, `std::formatter` support
- Rich extras: `split`, `join`, `trim`, `replace`, `toUpper` / `toLower`,
  `startsWith` / `endsWith` / `contains`, `format`, `concat`, `toNumber` / `fromNumber`,
  repetition (`operator*`), regex helpers
- Typed exception hierarchy (`String_Error`, `Invalid_Argument`,
  `Index_Out_Of_Range`, `Empty_String`, `Invalid_Conversion`, `Invalid_Operation`)

### `utf8.h` — `cppstr::utf8`

Strict validation (rejects truncation, overlongs, surrogates, > U+10FFFF),
codepoint-length, char-indexed substring, per-character split, codepoint
encode/decode, char-aware reverse, terminal `display_width` (CJK = 2 columns),
plus `truncate_by_width` / `pad_by_width` that never split a character.

### `coding.h` — `cppstr::coding`

Base64 / Base32 / Base58 (Bitcoin alphabet) / Base64URL (RFC 4648), hex,
binary, octal, URL percent-encoding, integer radix 2–36, ROT13 / Caesar
cipher, Morse code.

### `json.h` — `cppstr::json`

Full JSON (RFC 8259): a `Value` variant type (null / bool / number / string /
array / object, insertion-ordered objects), a strict recursive-descent
`parse` with byte-position error messages, and `dump` in compact or
pretty-printed form. Round-trip safe: `parse(x.dump()) == x`.
Also RFC 6901 JSON Pointer (`pointer` / `set_pointer`), RFC 7386 JSON
Merge Patch (`merge`) and RFC 6902 JSON Patch (`patch`, all six operations).

### `csv.h` — `cppstr::csv`

Full RFC 4180 documents: `Document` with an optional header row, named
column access, `parse` (quoted separators, doubled quotes, embedded
newlines, CRLF preserved verbatim inside quoted fields), delimiter
auto-detection, and `write` with configurable quoting and line endings. Extends the single-line
`csv::escape` / `csv::split` helpers in `method.h`. `to_json` / `from_json`
convert between documents and JSON object/array forms (lossless round trip).

### `ini.h` — `cppstr::ini`

INI / configuration files: ordered sections and keys, `;` / `#` comments
(including inline), quoted values with escapes, typed getters
(`get_int` / `get_double` / `get_bool`), `set` / `erase` / `erase_section`,
and a `dump` serializer. The unnamed global section uses `*_global` helpers.

### `csurl.h` — `cppstr::csurl`

Small synchronous web reader with no curl and no third-party dependency.
`get(url, options)` returns a `Response` with `status`, `body`, `content_type`,
`final_url` and `error`; transport problems produce `status == 0` and a
non-empty `error()` rather than an exception. `fetch(url, options)` returns
the body for 2xx responses and throws `Invalid_Operation` otherwise. URLs are
parsed with `cppstr::url`; only http/https are accepted, malformed URLs or options
throw `Invalid_Argument`. Redirects, per-operation timeout, a body byte cap
and a custom user-agent are configurable via `Options`. Windows uses the
system WinHTTP API (HTTP + HTTPS, https detected by `supports_https()`);
POSIX builds use a small HTTP/1.0 socket client and reject https explicitly.

### `macro.h` — compile-time macro layer (header-only)

Built on the version macros in `cppString.h`. Two-stage stringizing
(`CPPSTRING_STR` / `CPPSTRING_XSTR`) and token pasting (`CPPSTRING_CAT` /
`CPPSTRING_XCAT` plus three-token `CAT3` / `XCAT3`); `CPPSTRING_VERSION_CODE`
and `CPPSTRING_VERSION_AT_LEAST` / `LESS_THAN` / `EQ` for `#if` and
`static_assert`; compiler / platform / build-mode flags; portable attributes
(`CPPSTRING_NODISCARD`, `CPPSTRING_DEPRECATED`, `CPPSTRING_LIKELY` /
`CPPSTRING_UNLIKELY`, `CPPSTRING_FALLTHROUGH`, `CPPSTRING_FORCEINLINE`,
`CPPSTRING_NOINLINE`, `CPPSTRING_UNUSED`); source location (`CPPSTRING_FUNC`,
`CPPSTRING_FILE_LINE`, `CPPSTRING_HERE`, `CPPSTRING_VARSTR`); and helpers
(`CPPSTRING_COUNT_OF`, `CPPSTRING_STATIC_ASSERT`, `CPPSTRING_ASSERT_ALWAYS`,
`CPPSTRING_LIT`). No `macro.cpp` exists — include the header and go.

### `toml.h` — `cppstr::toml`

TOML 1.0 core subset on top of `cppstr::json::Value`: `parse` reads comments,
bare / quoted / dotted keys, `[table]` headers, basic / literal / multiline
strings, decimal / hex / octal / binary integers, floats, booleans, arrays
and inline tables; `dump` emits a deterministic document that preserves key
insertion order across a round trip. Malformed input throws `Invalid_Format`
with line and byte position, out-of-range numbers throw `Invalid_Conversion`.
Date-time literals are preserved as strings and array-of-tables is rejected.

### `url.h` — `cppstr::url`

RFC 3986 URI handling: `Url` parses and rebuilds scheme / userinfo / host /
port / path / query / fragment (including IPv6 authorities), exposes decoded
query parameters, and resolves relative references with `resolve()`
(dot-segment removal included). `encode` / `decode` wrap percent-encoding.

### `unicode.h` — `cppstr::unicode`

Code-point level internationalization: `fold_case` (ASCII, Latin-1,
Latin Extended-A, Greek, Cyrillic), `to_fullwidth` / `to_halfwidth`,
`transliterate` (Latin diacritics, ligatures, Greek and Cyrillic → ASCII),
and `decompose` / `compose` (NFD-like / NFC-like for precomposed Latin).
Curated tables, not a full UCD implementation.

### `fmt.h` — `cppstr::fmt`

Type-safe `format("{} / {1} / {{escaped}}", ...)` with positional and indexed
arguments, alignment / fill / zero-padding, integer bases (`d`, `x`, `X`, `b`,
`o` with `#` prefixes), float precision (`f`, `e`, `g`), signs, and string
precision (UTF-8 aware). Bad syntax or a mismatched spec throws a typed
exception. Argument conversion is header-only; parsing lives in the library.

### `ansi.h` — `cppstr::ansi`

Terminal escape-sequence helpers: `strip`, `has_ansi`, `visible_width`
(CJK-aware), width-based `truncate` / `pad` that preserve SGR codes, and
`style` / `fg` / `rgb` / `bold` / `underline` / `inverse` / `reset` builders.

### `binary.h` — `cppstr::binary`

Raw byte parsing built around `cppstr::String` buffers (embedded NUL bytes are
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

原始字节解析，缓冲区用 `cppstr::String`（可含 `\0`）：hex / bit 文本互转、
`hex_dump`、可见化、位运算与字节序交换、模式查找与切片、二进制文件读写。
`Reader` 提供大小端整数 / 浮点、varint / svarint、C 字符串、行读取、
签名匹配、对齐与越界检查；`Writer` 可链式写回，`pack<T>()` 打包单个值。
越界抛 `Index_Out_Of_Range`，格式错误抛 `Invalid_Conversion`。

### `file.h` — `cppstr::file`

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

### `regex.h` — `cppstr::regex`

`cppString`-friendly ECMAScript regular expressions (POSIX ERE grammar is
available via `Option::extended`). `escape` quotes metacharacters;
`is_match` / `search` / `match` / `find_all` / `count` cover matching, with
`Match` exposing the whole match, byte offset, length and numbered capture
groups. `replace` / `replace_all` expand `$0..$9`, `${n}`, `$&`, `$$` and
`\1`-style references, plus a callback overload; `split` preserves trailing
empty fields. `Option::icase` enables case-insensitive matching. Use the fully
qualified `cppstr::regex::` prefix if you also have `using namespace std;`.
Invalid patterns throw `Invalid_Argument`, a bad group reference throws
`Invalid_Format`, and a bad group index throws `Index_Out_Of_Range`.

面向 `cppString` 的 ECMAScript 正则封装（也可用 `Option::extended` 切换 POSIX
ERE 语法）：`escape` 转义元字符；`is_match` / `search` / `match` / `find_all` /
`count` 做匹配，`Match` 提供完整匹配、字节偏移、长度与编号捕获组；`replace` /
`replace_all` 支持 `$1`、`${1}`、`$&`、`$$`、`\1` 反向引用及回调重载；
`split` 保留尾部空字段；`Option::icase` 忽略大小写。无效正则抛
`Invalid_Argument`，无效组引用抛 `Invalid_Format`，组下标越界抛
`Index_Out_Of_Range`。与 `std::regex` 同名，建议始终写 `cppstr::regex::` 全限定。

### `datetime.h` — `cppstr::datetime`

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

### `exec.h` — `cppstr::exec`

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

### `langcvt/` — `cppstr::pycvt` / `cppstr::javacvt` / `cppstr::jscvt`

Public headers: `<langcvt/langcvt.h>`, `<langcvt/pycvt.h>`,
`<langcvt/Javacvt.h>` and `<langcvt/jscvt.h>`. All adapters return
`cppstr::langcvt::Result`, an alias of `cppstr::exec::Result`, so callers get
`exit_code`, `out`, `err`, `started`, `timed_out`, `ok()` and `output()`.
`eval()` is the throw-on-failure variant that returns stdout only.

- `pycvt`: `run(code, Options)` executes `<interpreter> -c <code>` with optional
  trailing arguments. `default_interpreter()` picks `python3` or `python`.
- `jscvt`: `run(code, Options)` executes `node -e <code>` (or `nodejs`).
- `javacvt`: writes the source to a unique temporary `.java` file and invokes
  the Java 11+ single-file source launcher (`java File.java`). The class name
  is detected from the `class` declaration or supplied via `Options::class_name`;
  the temporary directory is removed before returning.

Source text is always passed as an argv element through `exec::invoke()`, never
interpolated into a shell command, so quotes and newlines survive unchanged.
A missing interpreter is reported through `Result`, not by throwing.

```cpp
#include <langcvt/pycvt.h>

cppstr::pycvt::Options options;
options.timeout = std::chrono::seconds{ 5 };
const auto result = cppstr::pycvt::run("print(6 * 7)", options);
if (result.ok())
    std::cout << result.out;
```

公开头文件为 `<langcvt/langcvt.h>`、`<langcvt/pycvt.h>`、`<langcvt/Javacvt.h>`
与 `<langcvt/jscvt.h>`。适配器统一返回 `cppstr::langcvt::Result`（即
`cppstr::exec::Result` 的别名），可读取退出码、stdout、stderr、`started`、
`timed_out`，并调用 `ok()` / `output()`；`eval()` 只返回 stdout，失败时抛异常。

- `pycvt`：执行 `<interpreter> -c <code>`，可附加参数；自动选择 `python3` / `python`。
- `jscvt`：执行 `node -e <code>`（或 `nodejs`）。
- `javacvt`：把源码写入唯一临时 `.java` 文件，调用 Java 11+ 单文件源码启动器
  （`java File.java`）；类名自动识别，也可用 `Options::class_name` 指定；返回前删除临时目录。

源码始终作为 argv 参数经 `exec::invoke()` 传递，不拼接进 shell 命令，引号与换行
不会被破坏。解释器缺失通过 `Result` 报告，不抛异常。

### `table.h` — `cppstr::table`

ASCII table rendering with per-column alignment. Column widths use
`utf8::display_width`, so CJK text stays aligned in a terminal.

### `argparse.h` — `cppstr::ArgParser`

Small command-line parser: `--long` / `-s` options, `--key=value`, combined
short flags (`-abc`), `--` terminator, positionals and an auto-generated
`help()` text.

### `method.h` — `cppstr::`

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

### `path.h` — `cppstr::path`

Pure-string path handling (no filesystem access, normalized to `/`):
`join`, `normalize` (drive letters, UNC, `.`, `..`), `filename`, `stem`,
`extension`, `parent`, `is_absolute`, `relative`.

### `checksum.h` — `cppstr::checksum`

CRC-32 (compile-time table), FNV-1a 64, Adler-32, MD5 (legacy, cache keys
only), SHA-256 and HMAC-SHA-256 — raw bytes and hex forms.

### `validate.h` — `cppstr::validate`

`is_ipv4`, `is_ipv6` (with `::` compression and embedded IPv4 tails),
`is_email`, `is_uuid`, `is_url`, `is_mac_address`, `is_hex_color`,
`is_luhn`, `is_ascii`, `is_strong_password`, `is_iban` (mod-97),
`is_jwt`, `is_base64`, `is_hostname`, `is_json`, `is_date` (calendar-valid).

### `semver.h` — `cppstr::semver`

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
#include <cppString/toml.h>
#include <cppString/macro.h>
#include <cppString/csurl.h>

using namespace cppstr;

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

cppString swapped = cppstr::regex::replace_all("2024-02-29", R"((\d{4})-(\d{2})-(\d{2}))",
                                            "${3}/${2}/${1}");   // "29/02/2024"
auto fields = cppstr::regex::split("a, b ,  c", R"(\s*,\s*)");     // {"a", "b", "c"}

auto stamp = cppstr::datetime::parse("2024-02-29T12:34:56+08:00");
cppString shown = cppstr::datetime::format(*stamp, "%Y-%m-%d %H:%M %z");
cppString when  = cppstr::datetime::relative(cppstr::datetime::to_unix(*stamp),
                                         cppstr::datetime::unix_now());
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
    REF v7.1.0
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
1291 项断言的本地测试套件 `testCppString.cpp` 位于 `Project1/Project1/`
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
├── include/cppString/             # public headers only (25 × .h)
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
│   ├── exec.h                     # external command execution, output capture
│   ├── toml.h                     # TOML 1.0 core subset reader/writer
│   ├── macro.h                    # header-only compile-time macro layer
│   └── csurl.h                    # HTTP/HTTPS web page reader (WinHTTP / POSIX)
├── include/langcvt/               # public headers only (4 × .h)
│   ├── langcvt.h                  # shared Result / Options / argv runner
│   ├── pycvt.h                    # Python code execution
│   ├── Javacvt.h                  # Java single-file source execution
│   └── jscvt.h                    # JavaScript (Node.js) code execution
├── src/                           # 28 implementation files (never installed)
├── ORIGINALITY.md                 # source provenance / 原创性声明
├── README.md
├── LICENSE
└── .gitignore
```

The 1291-assertion test suite (`testCppString.cpp`) is used by the local build
gate but is intentionally kept out of the published repository.

## License / 许可

[MIT](LICENSE) © 2025-2026 Ender_ — source provenance: [ORIGINALITY.md](ORIGINALITY.md).
源码来源与合规说明见 `ORIGINALITY.md`。
