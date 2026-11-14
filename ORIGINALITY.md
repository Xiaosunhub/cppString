# Originality / 原创性声明

**cppString** is an independently written library. All 44 published source
files — 22 public headers in `include/cppString/` and their 22 implementations
in `src/` — were authored for this project; every one of them carries an
`SPDX-License-Identifier: MIT` line and the same copyright notice
(`Copyright (c) 2025-2026 Ender_`). The project is distributed under the
[MIT License](LICENSE).

本项目的 44 个文件（22 个公开头文件位于 `include/cppString/`，22 个实现文件位于
`src/`）均为独立编写，统一带有 SPDX 标识与版权声明，采用 MIT 许可。**没有复制、移植或改写任何第三方库的源代码**，也没有任何
第三方运行时依赖：实现只使用 C++23 标准库（Windows 下 `file` 模块通过
条件编译调用系统 API，其他平台走 POSIX 路径）。

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
| INI, TOML-like helpers, tables, ANSI escapes | De-facto public formats |
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
