// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Ender_
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef CPPSTRING_EXEC_H_
#define CPPSTRING_EXEC_H_

#include "cppString.h"
#include <chrono>
#include <vector>

namespace sp
{
	// Run external commands (cmd.exe, PowerShell/pwsh, sh/bash) and capture
	// their output. No third-party dependencies: Windows uses CreateProcessW
	// with anonymous pipes, POSIX uses fork/execvp with poll().
	//
	// IMPORTANT: run()/cmd()/powershell() pass the command to a shell, so
	// never build a command from untrusted input without quoting. Use
	// invoke() with an explicit argv vector to avoid shell interpretation.
	namespace exec
	{
		enum class Shell
		{
			automatic,    // cmd.exe on Windows, /bin/sh elsewhere
			cmd,          // Windows cmd.exe; unavailable on POSIX
			powershell,   // powershell.exe (pwsh on POSIX)
			pwsh,         // PowerShell 7 executable
			sh,           // POSIX sh
			bash          // bash
		};

		struct CPPSTRING_EXPORT Result
		{
			int exit_code{ -1 };        // -1 when the process could not start
			cppString out{};            // raw stdout bytes (no transcoding)
			cppString err{};            // raw stderr bytes (empty when merged)
			// Windows console programs normally write the system OEM code
			// page rather than UTF-8; decode the bytes yourself if needed.
			bool started{ false };
			bool timed_out{ false };

			[[nodiscard]] bool ok() const noexcept
			{
				return started && !timed_out && exit_code == 0;
			}

			// stdout followed by stderr; useful for a single log string.
			[[nodiscard]] cppString output() const;
		};

		struct CPPSTRING_EXPORT Options
		{
			Shell shell{ Shell::automatic };
			cppString working_directory{};                 // empty = inherit
			std::chrono::milliseconds timeout{ 0 };       // 0 = wait forever
			// On Windows a timeout terminates the whole child process tree
			// through a job object, when the host allows job assignment.
			bool merge_stderr{ false };                    // redirect stderr to out
		};

		// Execute a command through the selected shell. Never throws for a
		// failing command: inspect started / timed_out / exit_code instead.
		[[nodiscard]] Result run(const cppString& command,
			const Options& options = Options{});

		// Convenience wrappers that force a particular shell.
		[[nodiscard]] Result cmd(const cppString& command,
			const Options& options = Options{});
		[[nodiscard]] Result powershell(const cppString& command,
			const Options& options = Options{});

		// Start an executable directly, without any shell. argv[0] is the
		// program (looked up in PATH) and the rest are its arguments.
		[[nodiscard]] Result invoke(const std::vector<cppString>& argv,
			const Options& options = Options{});

		// run() returning only stdout. Throws Invalid_Operation when the
		// process cannot be started or hits the timeout.
		[[nodiscard]] cppString capture(const cppString& command,
			const Options& options = Options{});

		// True when the program is executable / can be found in PATH.
		[[nodiscard]] bool exists(const cppString& program);

		// Quote one argument using the current platform's shell rules.
		[[nodiscard]] cppString quote(const cppString& argument);

		// The exact command line run() would hand to the OS (for logging and
		// auditing). PowerShell commands are passed as -EncodedCommand.
		[[nodiscard]] cppString build_command(Shell shell, const cppString& command);

		// ---- Short-form API (5.6.0) -----------------------------------------
		//
		// C++ forbids a defaulted parameter in front of a non-defaulted one,
		// so `execute(Actuator = "cmd.exe", command)` cannot be declared.
		// Two overloads keep the intended calling style instead:
		//     execute(command)             -> default actuator
		//     execute(Actuator, command)   -> explicit actuator
		// The one-argument form uses cmd.exe on Windows and /bin/sh on POSIX.
		// Recognised actuators: cmd.exe, powershell.exe, pwsh, sh, bash,
		// "auto" (platform default), with or without a path / .exe suffix.
		inline constexpr const char* default_actuator = "cmd.exe";

		// True only if the process started, did not time out and exited with 0.
		// A missing program or a non-zero exit code simply returns false;
		// an unrecognised actuator throws Invalid_Argument.
		[[nodiscard]] bool execute(const cppString& command);
		[[nodiscard]] bool execute(const cppString& actuator, const cppString& command);

		// The execution transcript: stdout with stderr merged in, in the order
		// the process produced it. Start failures and non-zero exits are not
		// errors here - their message / output is returned as text instead.
		// An unrecognised actuator throws Invalid_Argument.
		[[nodiscard]] cppString getExecResult(const cppString& command);
		[[nodiscard]] cppString getExecResult(const cppString& actuator,
			const cppString& command);
	}
}

#endif // CPPSTRING_EXEC_H_
