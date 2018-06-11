#pragma once

#include <string>
#include <deque>
#include <Windows.h>

void help(const std::string& processName);

int processCommandArgs(std::deque<std::string> args, uintptr_t& address, HANDLE& hProcess, bool verbose);