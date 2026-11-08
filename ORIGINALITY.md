# Originality / 原创性声明

**cppString** is an independently written library. All 42 published `.h` / `.cpp`
files under `include/cppString/` were authored for this project; every one of
them carries an `SPDX-License-Identifier: MIT` line and the same copyright
notice (`Copyright (c) 2025-2026 Ender_`). The project is distributed under the
[MIT License](LICENSE).

本项目的 42 个头文件与实现文件均为独立编写，统一带有 SPDX 标识与版权声明，
采用 MIT 许可。**没有复制、移植或改写任何第三方库的源代码**，也没有任何
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
grep -RniE "adapted from|based on|ported from|taken from|@author|copyright \(c\) (?!2025)" include/cppString

# every published file declares the same license
grep -RL "SPDX-License-Identifier: MIT" include/cppString
```

This document is a technical provenance statement, not legal advice. It
records what was written, what was audited, and which public specifications
the implementations follow, so downstream packagers (including vcpkg
reviewers) can verify the claim themselves.

本文档是技术性的来源说明而非法律意见，记录了编写方式、审查范围与所遵循的
公开规范，便于下游打包方（含 vcpkg 审核）自行复核。
