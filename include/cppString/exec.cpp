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

#include "exec.h"
#include "coding.h"
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

#ifdef _WIN32
#   ifndef WIN32_LEAN_AND_MEAN
#       define WIN32_LEAN_AND_MEAN
#   endif
#   include <windows.h>
#else
#   include <fcntl.h>
#   include <poll.h>
#   include <signal.h>
#   include <sys/types.h>
#   include <sys/wait.h>
#   include <unistd.h>
#   include <cerrno>
#endif

namespace sp
{
	namespace exec
	{
		namespace
		{
#ifdef _WIN32
			// All paths, command lines and captured text are UTF-8 in the
			// public API; Windows entry points need UTF-16 instead.
			std::wstring widen(const String& text)
			{
				if (text.empty())
					return std::wstring{};

				const int needed = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
					text.data(), static_cast<int>(text.size()), nullptr, 0);
				if (needed <= 0)
					return std::wstring{};

				std::wstring out(static_cast<std::size_t>(needed), L'\0');
				MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
					text.data(), static_cast<int>(text.size()), out.data(), needed);
				return out;
			}

			// Windows command-line quoting rules (backslashes only double up
			// immediately before a quote or at the very end of the argument).
			String quote_win(const String& text)
			{
				if (!text.empty() && text.find_first_of(" \t\n\v\"") == String::npos)
					return text;

				String out;
				out.reserve(text.size() + 2);
				out += '"';

				std::size_t i = 0;
				while (i < text.size())
				{
					std::size_t slashes = 0;
					while (i < text.size() && text[i] == '\\') { ++slashes; ++i; }

					if (i == text.size())
					{
						out.append(slashes * 2, '\\');
						break;
					}

					if (text[i] == '"')
					{
						out.append(slashes * 2 + 1, '\\');
						out += '"';
					}
					else
					{
						out.append(slashes, '\\');
						out += text[i];
					}
					++i;
				}

				out += '"';
				return out;
			}

			// PowerShell's -EncodedCommand expects UTF-16LE text in base64.
			// Encoding the command avoids every quoting pitfall of -Command.
			String utf16le_bytes(const String& text)
			{
				const std::wstring units = widen(text);
				String bytes;
				bytes.reserve(units.size() * 2);

				for (const wchar_t unit : units)
				{
					const unsigned code = static_cast<unsigned>(
						static_cast<unsigned short>(unit));
					bytes += static_cast<char>(code & 0xFFu);
					bytes += static_cast<char>((code >> 8) & 0xFFu);
				}
				return bytes;
			}

			String encoded_command(const String& text)
			{
				return sp::coding::toBase64(cppString{ utf16le_bytes(text) }).s_str();
			}

			String make_command_line(Shell shell, const String& command)
			{
				switch (shell)
				{
				case Shell::powershell:
					return "powershell.exe -NoProfile -NonInteractive -EncodedCommand "
						+ encoded_command(command);
				case Shell::pwsh:
					return "pwsh.exe -NoProfile -NonInteractive -EncodedCommand "
						+ encoded_command(command);
				case Shell::sh:
					return "sh.exe -c " + quote_win(command);
				case Shell::bash:
					return "bash.exe -c " + quote_win(command);
				case Shell::cmd:
				case Shell::automatic:
				default:
					// /S keeps the outer quotes from being reinterpreted, so a
					// command such as "C:\Program Files\x.exe" arg survives.
					return "cmd.exe /D /S /C \"" + command + "\"";
				}
			}

			void close_handle(HANDLE& handle) noexcept
			{
				if (handle && handle != INVALID_HANDLE_VALUE)
				{
					CloseHandle(handle);
					handle = nullptr;
				}
			}

			// Peek first: ReadFile on a synchronous pipe blocks when empty,
			// which would deadlock while the child waits on a full stderr pipe.
			void drain(HANDLE handle, String& sink)
			{
				if (!handle)
					return;

				for (;;)
				{
					DWORD available = 0;
					if (!PeekNamedPipe(handle, nullptr, 0, nullptr, &available, nullptr)
						|| available == 0)
						break;

					char buffer[4096];
					const DWORD want = (std::min)(available,
						static_cast<DWORD>(sizeof(buffer)));
					DWORD got = 0;
					if (!ReadFile(handle, buffer, want, &got, nullptr) || got == 0)
						break;
					sink.append(buffer, got);
				}
			}

			Result launch(const String& command_line, const String& working_dir,
				std::chrono::milliseconds timeout, bool merge_stderr_flag)
			{
				Result result{};

				SECURITY_ATTRIBUTES sa{};
				sa.nLength = sizeof(sa);
				sa.lpSecurityDescriptor = nullptr;
				sa.bInheritHandle = TRUE;

				HANDLE out_read = nullptr, out_write = nullptr;
				HANDLE err_read = nullptr, err_write = nullptr;
				HANDLE in_read = nullptr, in_write = nullptr;

				auto close_all = [&]() noexcept
				{
					close_handle(out_read);  close_handle(out_write);
					close_handle(err_read);  close_handle(err_write);
					close_handle(in_read);   close_handle(in_write);
				};

				if (!CreatePipe(&out_read, &out_write, &sa, 0))
					return result;
				SetHandleInformation(out_read, HANDLE_FLAG_INHERIT, 0);

				if (!merge_stderr_flag)
				{
					if (!CreatePipe(&err_read, &err_write, &sa, 0))
					{
						close_all();
						return result;
					}
					SetHandleInformation(err_read, HANDLE_FLAG_INHERIT, 0);
				}

				// Give the child an stdin pipe whose write end is closed right
				// after startup, so it sees EOF instead of hanging on input.
				if (!CreatePipe(&in_read, &in_write, &sa, 0))
				{
					close_all();
					return result;
				}
				SetHandleInformation(in_write, HANDLE_FLAG_INHERIT, 0);

				STARTUPINFOW si{};
				si.cb = sizeof(si);
				si.dwFlags = STARTF_USESTDHANDLES;
				si.hStdInput = in_read;
				si.hStdOutput = out_write;
				si.hStdError = merge_stderr_flag ? out_write : err_write;

				PROCESS_INFORMATION pi{};
				std::wstring cmd = widen(command_line);
				std::wstring cwd = widen(working_dir);

				const BOOL created = CreateProcessW(
					nullptr,
					cmd.empty() ? nullptr : cmd.data(),
					nullptr, nullptr,
					TRUE,                 // inherit only the pipe ends we marked
					CREATE_NO_WINDOW,
					nullptr,
					cwd.empty() ? nullptr : cwd.c_str(),
					&si, &pi);

				// The parent must drop its copies of the child's ends, or the
				// pipe never signals EOF.
				close_handle(out_write);
				close_handle(err_write);
				close_handle(in_read);
				close_handle(in_write);

				if (!created)
				{
					close_all();
					result.err = cppString{ "sp::exec: could not start the process" };
					return result;
				}

				result.started = true;
				close_handle(pi.hThread);

				// A timed-out shell may have spawned grandchildren
				// (cmd.exe -> ping.exe). A job object lets us kill the whole
				// tree. Hosts that already use jobs may refuse assignment;
				// in that case fall back to terminating just the child.
				HANDLE job = CreateJobObjectW(nullptr, nullptr);
				if (job && !AssignProcessToJobObject(job, pi.hProcess))
					close_handle(job);

				String out_buf, err_buf;
				const auto deadline = std::chrono::steady_clock::now() + timeout;

				for (;;)
				{
					drain(out_read, out_buf);
					if (err_read)
						drain(err_read, err_buf);

					if (WaitForSingleObject(pi.hProcess, 10) == WAIT_OBJECT_0)
						break;

					if (timeout.count() > 0
						&& std::chrono::steady_clock::now() >= deadline)
					{
						if (job)
							TerminateJobObject(job, 1);
						else
							TerminateProcess(pi.hProcess, 1);
						WaitForSingleObject(pi.hProcess, 2000);
						result.timed_out = true;
						break;
					}
				}

				// Flush anything the child wrote just before exiting.
				drain(out_read, out_buf);
				if (err_read)
					drain(err_read, err_buf);

				DWORD code = 0;
				if (GetExitCodeProcess(pi.hProcess, &code))
					result.exit_code = static_cast<int>(code);

				close_handle(pi.hProcess);
				close_handle(job);
				close_all();

				result.out = cppString{ std::move(out_buf) };
				result.err = cppString{ std::move(err_buf) };
				return result;
			}
#else
			String quote_posix(const String& text)
			{
				String out = "'";
				for (const char c : text)
				{
					if (c == '\'')
						out += "'\\''";
					else
						out += c;
				}
				out += '\'';
				return out;
			}

			std::vector<String> shell_argv(Shell shell, const String& command)
			{
				switch (shell)
				{
				case Shell::automatic:
				case Shell::sh:
					return { "/bin/sh", "-c", command };
				case Shell::bash:
					return { "/bin/bash", "-c", command };
				case Shell::powershell:
				case Shell::pwsh:
					return { "pwsh", "-NoProfile", "-NonInteractive", "-Command", command };
				case Shell::cmd:
				default:
					return {};      // cmd.exe does not exist on POSIX
				}
			}

			String make_command_line(Shell shell, const String& command)
			{
				const std::vector<String> argv = shell_argv(shell, command);
				String out;
				for (std::size_t i = 0; i < argv.size(); ++i)
				{
					if (i != 0)
						out += ' ';
					out += (i == 0) ? argv[i] : quote_posix(argv[i]);
				}
				return out;
			}

			void set_nonblocking(int fd) noexcept
			{
				const int flags = ::fcntl(fd, F_GETFL, 0);
				if (flags >= 0)
					::fcntl(fd, F_SETFL, flags | O_NONBLOCK);
			}

			void drain_fd(int fd, String& sink)
			{
				if (fd < 0)
					return;
				for (;;)
				{
					char buffer[4096];
					const ssize_t got = ::read(fd, buffer, sizeof(buffer));
					if (got > 0)
					{
						sink.append(buffer, static_cast<std::size_t>(got));
						continue;
					}
					break;      // EOF or EAGAIN
				}
			}

			void close_fd(int& fd) noexcept
			{
				if (fd >= 0)
				{
					::close(fd);
					fd = -1;
				}
			}

			Result launch_argv(const std::vector<String>& argv, const String& working_dir,
				std::chrono::milliseconds timeout, bool merge_stderr_flag)
			{
				Result result{};
				if (argv.empty() || argv[0].empty())
				{
					result.err = cppString{ "sp::exec: empty argument vector" };
					return result;
				}

				int out_pipe[2] = { -1, -1 };
				int err_pipe[2] = { -1, -1 };
				int in_pipe[2] = { -1, -1 };

				auto close_all = [&]() noexcept
				{
					close_fd(out_pipe[0]); close_fd(out_pipe[1]);
					close_fd(err_pipe[0]); close_fd(err_pipe[1]);
					close_fd(in_pipe[0]);  close_fd(in_pipe[1]);
				};

				if (::pipe(out_pipe) != 0)
					return result;
				if (!merge_stderr_flag && ::pipe(err_pipe) != 0)
				{
					close_all();
					return result;
				}
				if (::pipe(in_pipe) != 0)
				{
					close_all();
					return result;
				}

				const pid_t pid = ::fork();
				if (pid < 0)
				{
					close_all();
					return result;
				}

				if (pid == 0)
				{
					::dup2(in_pipe[0], STDIN_FILENO);
					::dup2(out_pipe[1], STDOUT_FILENO);
					::dup2(merge_stderr_flag ? out_pipe[1] : err_pipe[1], STDERR_FILENO);

					for (int fd : { in_pipe[0], in_pipe[1], out_pipe[0], out_pipe[1],
						err_pipe[0], err_pipe[1] })
					{
						if (fd >= 0)
							::close(fd);
					}

					if (!working_dir.empty())
						::chdir(working_dir.c_str());

					std::vector<char*> raw;
					raw.reserve(argv.size() + 1);
					for (const String& item : argv)
						raw.push_back(const_cast<char*>(item.c_str()));
					raw.push_back(nullptr);

					::execvp(raw[0], raw.data());
					::_exit(127);
				}

				close_fd(out_pipe[1]);
				close_fd(err_pipe[1]);
				close_fd(in_pipe[0]);
				close_fd(in_pipe[1]);
				set_nonblocking(out_pipe[0]);
				set_nonblocking(err_pipe[0]);
				result.started = true;

				String out_buf, err_buf;
				const auto deadline = std::chrono::steady_clock::now() + timeout;
				int status = 0;
				bool reaped = false;

				for (;;)
				{
					pollfd fds[2]{};
					int count = 0;
					fds[count].fd = out_pipe[0];
					fds[count].events = POLLIN;
					++count;
					if (err_pipe[0] >= 0)
					{
						fds[count].fd = err_pipe[0];
						fds[count].events = POLLIN;
						++count;
					}

					if (::poll(fds, static_cast<nfds_t>(count), 10) > 0)
					{
						for (int i = 0; i < count; ++i)
						{
							if (fds[i].revents & (POLLIN | POLLHUP))
							{
								drain_fd(fds[i].fd,
									fds[i].fd == out_pipe[0] ? out_buf : err_buf);
							}
						}
					}

					if (::waitpid(pid, &status, WNOHANG) == pid)
					{
						reaped = true;
						break;
					}

					if (timeout.count() > 0
						&& std::chrono::steady_clock::now() >= deadline)
					{
						::kill(pid, SIGKILL);
						::waitpid(pid, &status, 0);
						reaped = true;
						result.timed_out = true;
						break;
					}
				}

				drain_fd(out_pipe[0], out_buf);
				drain_fd(err_pipe[0], err_buf);
				close_all();

				if (!reaped)
					::waitpid(pid, &status, 0);

				if (WIFEXITED(status))
					result.exit_code = WEXITSTATUS(status);
				else if (WIFSIGNALED(status))
					result.exit_code = 128 + WTERMSIG(status);

				result.out = cppString{ std::move(out_buf) };
				result.err = cppString{ std::move(err_buf) };
				return result;
			}
#endif
		}

		cppString Result::output() const
		{
			String combined = out.s_str();
			combined += err.s_str();
			return cppString{ std::move(combined) };
		}

		cppString build_command(Shell shell, const cppString& command)
		{
			return cppString{ make_command_line(shell, command.s_str()) };
		}

		cppString quote(const cppString& argument)
		{
#ifdef _WIN32
			return cppString{ quote_win(argument.s_str()) };
#else
			return cppString{ quote_posix(argument.s_str()) };
#endif
		}

		Result run(const cppString& command, const Options& options)
		{
#ifdef _WIN32
			return launch(make_command_line(options.shell, command.s_str()),
				options.working_directory.s_str(), options.timeout, options.merge_stderr);
#else
			const std::vector<String> argv = shell_argv(options.shell, command.s_str());
			if (argv.empty())
			{
				Result result{};
				result.err = cppString{ "sp::exec: cmd.exe is unavailable on this platform" };
				return result;
			}
			return launch_argv(argv, options.working_directory.s_str(),
				options.timeout, options.merge_stderr);
#endif
		}

		Result cmd(const cppString& command, const Options& options)
		{
			Options copy = options;
			copy.shell = Shell::cmd;
			return run(command, copy);
		}

		Result powershell(const cppString& command, const Options& options)
		{
			Options copy = options;
			copy.shell = Shell::powershell;
			return run(command, copy);
		}

		Result invoke(const std::vector<cppString>& argv, const Options& options)
		{
#ifdef _WIN32
			String line;
			for (std::size_t i = 0; i < argv.size(); ++i)
			{
				if (i != 0)
					line += ' ';
				line += quote_win(argv[i].s_str());
			}
			return launch(line, options.working_directory.s_str(),
				options.timeout, options.merge_stderr);
#else
			std::vector<String> raw;
			raw.reserve(argv.size());
			for (const cppString& item : argv)
				raw.push_back(item.s_str());
			return launch_argv(raw, options.working_directory.s_str(),
				options.timeout, options.merge_stderr);
#endif
		}

		cppString capture(const cppString& command, const Options& options)
		{
			const Result result = run(command, options);
			if (!result.started)
				throw sp::Invalid_Operation("sp::exec::capture: process could not be started");
			if (result.timed_out)
				throw sp::Invalid_Operation("sp::exec::capture: process timed out");
			return result.out;
		}

		bool exists(const cppString& program)
		{
			const String& name = program.s_str();
			if (name.empty())
				return false;

#ifdef _WIN32
			const std::wstring wide = widen(name);
			if (wide.empty())
				return false;

			std::vector<wchar_t> buffer(32768);
			const DWORD length = SearchPathW(nullptr, wide.c_str(), L".exe",
				static_cast<DWORD>(buffer.size()), buffer.data(), nullptr);
			return length > 0 && length < buffer.size();
#else
			if (name.find('/') != String::npos)
				return ::access(name.c_str(), X_OK) == 0;

			const char* environment = std::getenv("PATH");
			if (!environment)
				return false;

			const String path{ environment };
			std::size_t pos = 0;
			while (pos <= path.size())
			{
				const std::size_t colon = path.find(':', pos);
				String directory = path.substr(pos,
					colon == String::npos ? String::npos : colon - pos);
				if (directory.empty())
					directory = ".";
				if (::access((directory + "/" + name).c_str(), X_OK) == 0)
					return true;
				if (colon == String::npos)
					break;
				pos = colon + 1;
			}
			return false;
#endif
		}
	}
}
