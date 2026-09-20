/*
	OmniaFramework - A collection of useful functionality
	Copyright (C) 2026  OmniaX-Dev

	This file is part of OmniaFramework.

	OmniaFramework is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	OmniaFramework is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with OmniaFramework.  If not, see <https://www.gnu.org/licenses/>.
*/

// Standalone tests for ostd::LogFileOutputHandler.
// Exercises openFile() validation (including the same-working-directory
// path case), buffered printing, flush()/clear() semantics, and the
// no-op color/cursor methods.
//
// Run as a separate executable. No external test framework — just a
// minimal in-house assertion macro that records pass/fail counts.

#include <ostd/io/IOHandlers.hpp>
#include <ostd/io/FileSystem.hpp>
#include <ostd/string/String.hpp>

namespace {

// ============================================================
//  Tiny test harness
// ============================================================

struct TestStats {
	u32 total { 0 };
	u32 passed { 0 };
	u32 failed { 0 };
	String currentTest;
	stdvec<String> failures;
};

static TestStats g_stats;
static ostd::ConsoleOutputHandler g_out;

static void __record_pass(void)
{
	g_stats.total++;
	g_stats.passed++;
}

static void __record_fail(const String& test, const String& detail, i32 line)
{
	g_stats.total++;
	g_stats.failed++;
	String msg;
	msg.add("  [").add(test).add(":").add(line).add("] ").add(detail);
	g_stats.failures.push_back(msg);
	g_out.fg(ostd::ConsoleColors::BrightRed).p("F").reset().flush();
}

#define CHECK(cond)                                                          \
	do {                                                                     \
		if (cond) { __record_pass();                                         \
			g_out.fg(ostd::ConsoleColors::BrightGreen).p(".").reset().flush(); \
		}                                                                    \
		else {                                                               \
			__record_fail(g_stats.currentTest, String("CHECK failed: ") + #cond, __LINE__); \
		}                                                                    \
	} while(0)

#define CHECK_STR_EQ(actual, expected)                                                      \
	do {                                                                                    \
		String __a = (actual);                                                              \
		String __e = (expected);                                                            \
		if (__a == __e.c_str()) { __record_pass();                                          \
			g_out.fg(ostd::ConsoleColors::BrightGreen).p(".").reset().flush();              \
		}                                                                                   \
		else {                                                                              \
			String __msg;                                                                   \
			__msg.add("CHECK_STR_EQ failed:  got \"").add(__a).add("\", expected \"").add(__e).add("\""); \
			__record_fail(g_stats.currentTest, __msg, __LINE__);                            \
		}                                                                                   \
	} while(0)

#define TEST(name)                                                                          \
	g_stats.currentTest = name;                                                             \
	g_out.nl().fg(ostd::ConsoleColors::Cyan).p("[").p(name).p("] ").reset().flush();


// ============================================================
//  Tests
// ============================================================

void test_openFile_bare_filename_same_directory(void)
{
	TEST("openFile_bare_filename_same_directory");

	// No slashes at all — this is the path that used to trip up
	// FileSystem's relative-path handling before the ./ workaround.
	ostd::FileSystem::deleteFile("logfile_test_bare.log");

	ostd::LogFileOutputHandler log;
	CHECK(log.openFile("logfile_test_bare.log"));

	log.p("hello from bare filename").flush();
	CHECK(ostd::FileSystem::fileExists("./logfile_test_bare.log"));

	ostd::FileSystem::deleteFile("logfile_test_bare.log");
}

void test_openFile_invalid_path(void)
{
	TEST("openFile_invalid_path");

	ostd::LogFileOutputHandler log;
	// A directory that does not exist can't hold a new file either.
	CHECK(!log.openFile("this/directory/does/not/exist/app.log"));
}

void test_flush_writes_prefixed_line_and_clears_buffer(void)
{
	TEST("flush_writes_prefixed_line_and_clears_buffer");

	const String path = "./logfile_test_flush.log";
	ostd::FileSystem::deleteFile(path);

	ostd::LogFileOutputHandler log;
	CHECK(log.openFile(path));

	log.p("Application started").flush();
	log.p("User ").p("Alice").p(" logged in with id ").p((i32)42).flush();

	stdvec<String> lines;
	CHECK(ostd::FileSystem::readTextFile(path, lines));
	CHECK(lines.size() == 2u);

	if (lines.size() == 2u)
	{
		// "[dd.mm.yyyy - hh:mm:ss]  ->  <text>"
		CHECK(lines[0].startsWith("["));
		CHECK(lines[0].contains("]  ->  Application started"));
		CHECK(lines[1].contains("]  ->  User Alice logged in with id 42"));
	}

	ostd::FileSystem::deleteFile(path);
}

void test_clear_discards_unflushed_buffer(void)
{
	TEST("clear_discards_unflushed_buffer");

	const String path = "./logfile_test_clear.log";
	ostd::FileSystem::deleteFile(path);

	ostd::LogFileOutputHandler log;
	CHECK(log.openFile(path));

	log.p("this should never reach the file");
	log.clear();
	log.p("only this survives").flush();

	stdvec<String> lines;
	CHECK(ostd::FileSystem::readTextFile(path, lines));
	CHECK(lines.size() == 1u);
	if (lines.size() == 1u)
		CHECK(lines[0].contains("]  ->  only this survives"));

	ostd::FileSystem::deleteFile(path);
}

void test_styling_and_cursor_methods_are_noop(void)
{
	TEST("styling_and_cursor_methods_are_noop");

	const String path = "./logfile_test_noop.log";
	ostd::FileSystem::deleteFile(path);

	ostd::LogFileOutputHandler log;
	CHECK(log.openFile(path));

	// None of these should throw, crash, or leak anything into the buffer.
	log.fg(ostd::ConsoleColors::Red).bg(ostd::ConsoleColors::Black)
	   .pStyled("plain styled text").xy(3, 4).x(1).y(2).nl().reset();
	log.p("only this is printed").flush();

	CHECK(log.getCursorPosition().x == 0 && log.getCursorPosition().y == 0);
	CHECK(log.getConsoleSize().x == 0 && log.getConsoleSize().y == 0);

	stdvec<String> lines;
	CHECK(ostd::FileSystem::readTextFile(path, lines));
	CHECK(lines.size() == 1u);
	if (lines.size() == 1u)
		CHECK(lines[0].contains("]  ->  only this is printed"));

	ostd::FileSystem::deleteFile(path);
}

}  // anonymous namespace


// ============================================================
//  Driver
// ============================================================

int main(void)
{
	g_out.fg(ostd::ConsoleColors::BrightCyan)
		 .p("Running LogFileOutputHandler tests...").nl().reset();

	test_openFile_bare_filename_same_directory();
	test_openFile_invalid_path();
	test_flush_writes_prefixed_line_and_clears_buffer();
	test_clear_discards_unflushed_buffer();
	test_styling_and_cursor_methods_are_noop();

	g_out.nl().nl();

	if (g_stats.failed == 0) {
		g_out.fg(ostd::ConsoleColors::BrightGreen)
			 .p("ALL ").p(g_stats.passed).p(" CHECKS PASSED").reset().nl();
		return 0;
	}
	else {
		g_out.fg(ostd::ConsoleColors::BrightRed)
			 .p(g_stats.failed).p(" FAILURES out of ").p(g_stats.total).p(" checks:")
			 .reset().nl();
		for (const auto& f : g_stats.failures)
			g_out.p(f).nl();
		return 1;
	}
}
