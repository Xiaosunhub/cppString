# Originality / 原创性声明

**cppString** is an independently written library. All 67 published source
files — 30 public headers in `include/cppString/` (8 of them in
`include/cppString/langcvt/`) and 33 implementations in `src/` — were authored
for this project; every one of them carries an `SPDX-License-Identifier: MIT`
line and the same copyright notice (`Copyright (c) 2025-2026 Ender_`). The
project is distributed under the [MIT License](LICENSE).

本项目的 67 个文件（30 个公开头文件位于 `include/cppString/`，其中 8 个位于
`include/cppString/langcvt/`，33 个实现文件位于 `src/`）均为独立编写，统一带有 SPDX
标识与版权声明，采用 MIT 许可。其中 `macro.h` 是 header-only 模块，没有对应的
实现文件。**没有复制、移植或改写任何第三方库的源代码**，也没有任何
第三方运行时依赖：实现只使用 C++23 标准库与系统 API（Windows 下 `file` 模块通过
条件编译调用系统 API，其他平台走 POSIX 路径）。`langcvt` 仅在用户机器上调用
已安装的 Python / Java / Node.js 解释器与 C / C++ / Rust / C# 编译器，
不捆绑其任何源码或二进制。

## What was checked / 审查范围

The 5.4.1 release includes a source audit for external authorship markers:

- scanned all published `.h` / `.cpp` files for `copyright`, `license`,
  external URLs, author tags, and phrases such as "adapted from",
  "based on", "ported from", "taken from", plus names of well-known
  third-party libraries — **no external code attribution was found**;
- every file now has an identical, self-consistent SPDX/copyright header
  (nine files previously had no header at all; those were added);
- include guards using reserved identifiers (`__INC_*`) were replaced with
  project-owned `CPPSTRING_*_H_` guards;
- the calendar conversion in `datetime.cpp` was **rewritten from scratch** in
  5.4.1. Earlier code followed the structure of a widely published civil-date
  formula. The current implementation is independent: it reduces any date
  into one 400-year Gregorian cycle (146097 days) and counts the remaining
  years and months directly from the leap rule. Public behaviour is unchanged
  (verified by round-trip regression tests across cycle boundaries).

5.4.1 还把日期换算算法整体重写为独立实现（不再沿用任何已公开的日期公式
代码结构），并用跨 400 年周期边界的往返测试验证行为不变。

The 5.4.2 bug-fix release touches only `json.cpp`, `url.h`, `url.cpp`,
`csv.cpp` and `datetime.cpp`. Every change is an independent fix derived from
the public specifications below (RFC 8259, RFC 3986, RFC 4180) or from plain
integer-overflow reasoning; no third-party source code was consulted or
incorporated. The SPDX headers and authorship of all 42 files are unchanged.

5.4.2 的缺陷修复仅涉及 `json.cpp`、`url.h`、`url.cpp`、`csv.cpp` 与
`datetime.cpp`，均依据上表公开规范或整数溢出分析独立实现，没有参考或引入
任何第三方源码；42 个文件的 SPDX 头与作者声明保持不变。

The 5.5.0 release adds two files, `exec.h` and `exec.cpp`. They are written
from scratch against the documented Win32 and POSIX process APIs
(CreateProcessW / anonymous pipes / job objects, fork / execvp / poll) and
the documented PowerShell `-EncodedCommand` convention. No code from a
third-party process or subprocess library is used; every published file
still carries the same SPDX header and copyright notice.

5.5.0 新增 `exec.h` 与 `exec.cpp` 两个文件，依据公开文档中的 Win32 / POSIX
进程接口（CreateProcessW、匿名管道、Job Object，以及 fork / execvp / poll）
和 PowerShell `-EncodedCommand` 约定独立编写，未使用任何第三方进程封装库；
全部发布文件仍保持统一的 SPDX 头与版权声明。

The 5.6.0 release only moves files and adds a small convenience layer. The
44 existing sources were relocated from `include/cppString/` into the split
`include/cppString/*.h` + `src/*.cpp` layout with no content rewrite; the new
`execute` / `getExecResult` overloads in `exec.h` / `exec.cpp` are thin wrappers
around the existing `run()` implementation, and the actuator-to-shell mapping
is a plain name lookup written for this project (the C++ default-argument rule
is a language constraint, not borrowed code). No file was added, no third-party
source was consulted, and all SPDX headers are unchanged.

5.6.0 只做目录拆分与少量便捷接口：44 个已有文件从 `include/cppString/`
原样迁移到 `include/cppString/*.h` + `src/*.cpp` 的新布局，未改写内容；
`exec.h` / `exec.cpp` 中的 `execute` / `getExecResult` 只是对现有 `run()`
的薄封装，执行器名称到 shell 的映射是本项目自写的普通名称查表（默认参数不能
位于非默认参数之前属于 C++ 语言规则，并非引用他人代码）。没有新增文件，
未参考或引入任何第三方源码，全部 SPDX 头保持不变。

The 5.6.1 release changes only `json.cpp`, `url.cpp`, `validate.cpp` and the
comment in `csv.h`. Every fix is an independent correction derived from the
public specifications below (RFC 8259 object names, RFC 6901 escape rules,
RFC 3986 query strings, CIDR notation) or from direct reasoning about the
library's own data structures. No file was added, no third-party source was
consulted, and all 44 SPDX headers remain unchanged.

5.6.1 仅改动 `json.cpp`、`url.cpp`、`validate.cpp` 及 `csv.h` 的注释，全部
依据上表公开规范（RFC 8259 对象键、RFC 6901 转义规则、RFC 3986 查询串、CIDR
记法）或对本项目自身数据结构的直接分析独立修正；没有新增文件，未参考或引入
任何第三方源码，44 个文件的 SPDX 头保持不变。

The 5.6.2 release changes only `coding.cpp`. The correction follows from the
library's own documented contract and the public specifications below;
no file was added, no third-party source was consulted, and all 44 SPDX
headers remain unchanged.

5.6.2 仅改动 `coding.cpp`，依据本项目自身文档契约与上表公开规范独立修正；
没有新增文件，未参考或引入任何第三方源码，44 个文件的 SPDX 头保持不变。

The 5.6.3 release changes only `csv.cpp`. The correction follows from the
library's own documented contract and the public specifications below;
no file was added, no third-party source was consulted, and all 44 SPDX
headers remain unchanged.

5.6.3 仅改动 `csv.cpp`，依据本项目自身文档契约与上表公开规范独立修正；
没有新增文件，未参考或引入任何第三方源码，44 个文件的 SPDX 头保持不变。

The 5.6.4 release changes only `csv.cpp`. The correction follows from the
library's own documented contract and the public specifications below;
no file was added, no third-party source was consulted, and all 44 SPDX
headers remain unchanged.

5.6.4 仅改动 `csv.cpp`，依据本项目自身文档契约与上表公开规范独立修正；
没有新增文件，未参考或引入任何第三方源码，44 个文件的 SPDX 头保持不变。

The 5.6.5 release changes only `utf8.cpp`. The correction follows from the
library's own documented contract and the public specifications below;
no file was added, no third-party source was consulted, and all 44 SPDX
headers remain unchanged.

5.6.5 仅改动 `utf8.cpp`，依据本项目自身文档契约与上表公开规范独立修正；
没有新增文件，未参考或引入任何第三方源码，44 个文件的 SPDX 头保持不变。

The 5.6.6 release changes only `cppString.cpp`. The correction follows from the
library's own documented contract and the public specifications below;
no file was added, no third-party source was consulted, and all 44 SPDX
headers remain unchanged.

5.6.6 仅改动 `cppString.cpp`，依据本项目自身文档契约与上表公开规范独立修正；
没有新增文件，未参考或引入任何第三方源码，44 个文件的 SPDX 头保持不变。

The 5.6.7 release changes only `cppString.cpp`. The correction follows from the
library's own documented contract and the public specifications below;
no file was added, no third-party source was consulted, and all 44 SPDX
headers remain unchanged.

5.6.7 仅改动 `cppString.cpp`，依据本项目自身文档契约与上表公开规范独立修正；
没有新增文件，未参考或引入任何第三方源码，44 个文件的 SPDX 头保持不变。

The 5.6.8 release changes only `file.cpp`. The correction follows from the
library's own documented contract and the public specifications below;
no file was added, no third-party source was consulted, and all 44 SPDX
headers remain unchanged.

5.6.8 仅改动 `file.cpp`，依据本项目自身文档契约与上表公开规范独立修正；
没有新增文件，未参考或引入任何第三方源码，44 个文件的 SPDX 头保持不变。

The 5.6.9 release changes only `cppString.cpp`. The correction follows from the
library's own documented contract and the public specifications below;
no file was added, no third-party source was consulted, and all 44 SPDX
headers remain unchanged.

5.6.9 仅改动 `cppString.cpp`，依据本项目自身文档契约与上表公开规范独立修正；
没有新增文件，未参考或引入任何第三方源码，44 个文件的 SPDX 头保持不变。

The 5.6.10 release changes only `semver.cpp`. The correction follows from the
library's own documented contract and the public specifications below;
no file was added, no third-party source was consulted, and all 44 SPDX
headers remain unchanged.

5.6.10 仅改动 `semver.cpp`，依据本项目自身文档契约与上表公开规范独立修正；
没有新增文件，未参考或引入任何第三方源码，44 个文件的 SPDX 头保持不变。

The 5.7.0 release adds two files, `toml.h` and `toml.cpp`. The reader is an
independent recursive-descent implementation of the TOML 1.0 core subset; the
writer and the shared `json::Value` document model are part of this project.
Only the published TOML 1.0 specification (toml.io) was consulted for the
grammar, and no third-party parser source was copied. All 46 published files
carry the same SPDX header and copyright notice.

5.7.0 新增 `toml.h` 与 `toml.cpp` 两个文件。解析器依据 TOML 1.0 公开规范
（toml.io）独立实现递归下降解析，序列化器与共用的 `json::Value` 数据模型
均为本项目自有代码；未参考或引入任何第三方解析器源码。当时的 46 个发布文件
保持统一的 SPDX 头与版权声明。

The 6.0.0 release adds one header-only file, `macro.h`. Every macro in it —
two-stage stringizing and concatenation, version-code gates, compiler /
platform detection, portability attributes, source-location diagnostics and
assert helpers — is written from scratch against the C++ standard's defined
preprocessor and attribute rules. No third-party header, framework, or macro
collection was copied or adapted. The version macros are shared with
`cppString.h`; the new file brings the published total to 47 files (24 headers,
23 implementations) and keeps the same SPDX header and copyright notice.

6.0.0 新增一个纯头文件 `macro.h`。其中所有宏（两阶段字符串化与连接、版本码
门控、编译器 / 平台探测、可移植属性、源码位置诊断与断言辅助）均依据 C++
标准规定的预处理器与属性规则从零编写，未复制或改写任何第三方头文件、框架
或宏集合。版本宏与 `cppString.h` 共用；发布文件总数增至 47 个（24 头文件、
23 实现文件），保持统一的 SPDX 头与版权声明。

The 6.1.0 release adds two files, `csurl.h` and `csurl.cpp`: a small
synchronous HTTP client written against the WinHTTP system API on Windows and
POSIX sockets elsewhere. No curl, libcurl, or any third-party HTTP code was
used or copied. URL parsing reuses this project's own `cppstr::url` module. Only
publicly documented WinHTTP / BSD socket APIs were consulted. All 49 published
files keep the same SPDX header and copyright notice.

6.1.0 新增 `csurl.h` 与 `csurl.cpp` 两个文件：基于 Windows 系统 WinHTTP API
与 POSIX socket 独立编写的轻量同步 HTTP 客户端，未使用或复制 curl、libcurl
或任何第三方 HTTP 代码，URL 解析复用本项目自有 `cppstr::url` 模块。仅参考公开的
WinHTTP / BSD socket 文档。全部 49 个发布文件保持统一的 SPDX 头与版权声明。

The 7.0.0 release is a namespace-only migration: all 49 published source
files (25 headers and 24 implementations) now use `cppstr` as their single
public namespace. No source file was added or removed, no third-party code was
consulted, and every SPDX header and copyright notice is unchanged. The test
harness was updated in lockstep but remains outside the published repository.

7.0.0 仅做命名空间迁移：全部 49 个发布源文件（25 个头文件、24 个实现文件）
统一使用 `cppstr` 作为唯一公开命名空间。没有增删源文件，未参考或引入任何
第三方代码，所有 SPDX 头与版权声明保持不变。测试文件同步更新但仍不入库。

The 7.3.0 release adds `algorithm.h` / `algorithm.cpp`
(`cppstr::algorithm`): Knuth-Morris-Pratt and Rabin-Karp search,
std-style permutations, UTF-8-aware LCS and common prefix/suffix helpers,
a sliding-window longest unique run, line sorting, byte rotation,
Roman numerals, run-length encoding and a Boyer-Moore majority vote.
Everything is written from scratch against the standard library and this
project's own `cppstr::utf8` helpers; the published source total is now
59 files, all with the same SPDX header and copyright notice.

7.3.0 新增 `algorithm.h` / `algorithm.cpp`（`cppstr::algorithm`）：KMP 与
Rabin-Karp 搜索、std 风格排列、UTF-8 感知的最长公共子序列与公共前缀/后缀、
滑动窗口最长无重复段、行排序、字节循环左移、罗马数字、游程编码与
Boyer-Moore 多数投票。全部基于标准库与本项目自有的 `cppstr::utf8` 辅助函数
从零编写，发布源文件总数现为 59 个，全部保持统一的 SPDX 头与版权声明。

The 7.4.0 release adds four compiled-language adapters under
`include/cppString/langcvt/` — `ccvt.h` / `cppcvt.h` / `rustcvt.h` /
`c#cvt.h` with implementations `ccvt.cpp`, `cppcvt.cpp`, `rustcvt.cpp` and
`csharpcvt.cpp` — plus a shared compile-and-run pipeline in
`langcvt.h` / `langcvt.cpp` (`temp_workspace`, `is_msvc_driver`,
`compile_and_run`). The adapters write user-provided source to a temporary
file and invoke whatever C / C++ / Rust / C# toolchain is already installed
on the machine (found through PATH and, on Windows, the documented .NET
Framework install locations); nothing is bundled, downloaded or embedded.
The published source total is now 67 files, all with the same SPDX header
and copyright notice.

7.4.0 在 `include/cppString/langcvt/` 下新增四个编译型语言适配器——
`ccvt.h` / `cppcvt.h` / `rustcvt.h` / `c#cvt.h` 与实现 `ccvt.cpp`、
`cppcvt.cpp`、`rustcvt.cpp`、`csharpcvt.cpp`——并在 `langcvt.h` / `langcvt.cpp`
中新增共享编译运行管线（`temp_workspace`、`is_msvc_driver`、`compile_and_run`）。
适配器把用户源码写入临时文件，调用机器上已安装的 C / C++ / Rust / C# 工具链
（通过 PATH 以及 Windows 上文档化的 .NET Framework 安装路径查找），不捆绑、
不下载、不内嵌任何工具链组件。发布源文件总数现为 67 个，全部保持统一的
SPDX 头与版权声明。

The 7.2.0 release promotes the previous `7.2.0-snapshot` to an official
release. It only finalizes version metadata: `CPPSTRING_VERSION` is the plain
semver string `7.2.0`, matching CMake's numeric project version `7.2.0`.
No source file was added, removed or modified in this line, and all SPDX
headers and copyright notices remain unchanged. The unpublished test harness
gained one prerelease-aware assertion during the snapshot phase
(1292 total).

7.2.0 正式版由 `7.2.0-snapshot` 转正而来，仅最终确定版本元数据：
`CPPSTRING_VERSION` 为正式 semver 串 `7.2.0`，与 CMake 数字工程版本
`7.2.0` 一致。本开发线没有新增、删除或修改任何源码文件，所有 SPDX 头与
版权声明保持不变。未入库的测试套件在快照阶段新增了 1 项预发布感知断言
（共 1292 项）。

The 7.1.1 release only relocates the four `langcvt` public headers from
`include/langcvt/` to `include/cppString/langcvt/` so that consumers use the
same `<cppString/langcvt/...>` spelling as the rest of the library. No source
file was added or removed, no implementation changed, and all SPDX headers and
copyright notices remain intact. The test harness was updated for the new
include paths but is still not published.

7.1.1 仅将四个 `langcvt` 公开头文件从 `include/langcvt/` 移到
`include/cppString/langcvt/`，让消费者与其他模块一样使用
`<cppString/langcvt/...>` 写法。没有增删源文件，实现未改动，所有 SPDX 头与
版权声明保持不变。测试文件同步更新包含路径但仍不入库。

The 7.1.0 release adds eight files under the new `cppString/langcvt/` header
group and the `src/` implementation tree: `langcvt.h`, `pycvt.h`, `Javacvt.h`, `jscvt.h`
and their four `.cpp` counterparts. The adapters are written from scratch on
top of this project's own `cppstr::exec::invoke()`; no Python, Java or
Node.js source, runtime, wrapper library or bundled binary was copied. Python
and JavaScript are passed to the user's interpreter as a single argv element;
Java source is written to a uniquely named temporary `.java` file and launched
through the documented Java 11+ source-file mode. The published source total is
now 57 files, all with the same SPDX header and copyright notice.

7.1.0 新增 `cppString/langcvt/` 头文件组与对应实现共 8 个文件：`langcvt.h`、`pycvt.h`、
`Javacvt.h`、`jscvt.h` 及四个同名 `.cpp`。适配器完全基于本项目自有的
`cppstr::exec::invoke()` 编写，未复制 Python、Java 或 Node.js 的源码、运行时、
封装库或二进制。Python 与 JavaScript 源码作为单个 argv 参数交给用户机器上的
解释器；Java 源码写入唯一命名的临时 `.java` 文件，并使用公开的 Java 11+
单文件源码启动方式执行。发布源文件总数现为 57 个，全部保持统一的 SPDX 头与
版权声明。

## Public standards, not copied code / 依据的公开标准

Some functions implement well-known, publicly documented algorithms and data
formats. The constants, tables and rule descriptions come from published
standards or are mathematical facts; **the code implementing them is written
independently for this project**:

| Area | Reference |
| --- | --- |
| MD5 / SHA-256 / HMAC | RFC 1321, FIPS 180-4, RFC 2104 |
| CRC-32 / Adler-32 / FNV-1a | RFC 1950, public-domain definitions |
| Base32 / Base64 / Base64URL / hex / radix | RFC 4648, public-domain definitions |
| URI parsing and resolution | RFC 3986, RFC 1123 |
| CSV | RFC 4180 |
| JSON, Pointer, Patch, Merge Patch | RFC 8259, RFC 6901, RFC 6902, RFC 7386 |
| Date and time | ISO 8601, RFC 3339, proleptic Gregorian calendar |
| UUID v4 | RFC 4122 |
| Regular-expression grammar | ECMA-262 / POSIX ERE via the standard library |
| Process / command execution | Win32 CreateProcess / pipes / job objects, POSIX fork / execvp / poll, PowerShell `-EncodedCommand` |
| UTF-8, normalization, East Asian width | The Unicode Standard |
| INI, TOML, tables, ANSI escapes | INI: de-facto format; TOML 1.0 specification (toml.io) |
| Levenshtein, Dice, Soundex, Luhn, ROT13/Caesar, Morse | Long-standing public-domain algorithms |

The only text reproduced verbatim is the **MIT license template** itself (in
`LICENSE` and in each file header). MIT explicitly permits this, and keeping
the notice intact is a condition of the license.

唯一原样引用的文本是 MIT 许可模板（许可本身要求保留该声明），这不构成代码抄袭。

## Re-checking / 复核方法

```
# no third-party authorship markers
grep -RniE "adapted from|based on|ported from|taken from|@author|copyright \(c\) (?!2025)" include/cppString src

# every published file declares the same license
grep -RL "SPDX-License-Identifier: MIT" include/cppString src
```

This document is a technical provenance statement, not legal advice. It
records what was written, what was audited, and which public specifications
the implementations follow, so downstream packagers (including vcpkg
reviewers) can verify the claim themselves.

本文档是技术性的来源说明而非法律意见，记录了编写方式、审查范围与所遵循的
公开规范，便于下游打包方（含 vcpkg 审核）自行复核。
