
// This file contains all the common logic (no read and write)

#include <iostream>
#include <deque>
#include <string>
#include <sstream>
#include <Windows.h>
#include <tlhelp32.h>
#include <psapi.h>

#include "implementation.h"

void defaultHelp(const std::string& processName) {
	std::cerr << "[Usage] " << processName << " [-v|--verbose|-h|--help] exe|PID|Window \"BaseModule: baseoffset offset1 offset2 offsetn\" data commands..." << std::endl;
	std::cerr << std::endl;
	std::cerr << "  -v | --verbose         Helpful for debugging and to comprehend what is happening" << std::endl;
	std::cerr << "  -h | --help            Print this help" << std::endl;
	std::cerr << "  exe | PID | Window     e.g. FarCry5.exe or 15573 or \"Minesweeper\"" << std::endl;
	std::cerr << std::endl;
	std::cerr << "  address definition: Must be a string in the format specified above" << std::endl;
	std::cerr << "    The first baseoffset directly offsets the BaseModule and all following offsets apply to the address pointed by the previous address." << std::endl;
	std::cerr << std::endl;
	std::cerr << "\"CT_FC5.dll: 0x139830 0x8 0x10\"" << std::endl;
	std::cerr << "    Here can be observed that first 0x139830 gets added to the module address following a dereferencing." << std::endl;
	std::cerr << "    The resulting address gets offset by 0x8 and dereferenced again and offset by 0x10 one last time (no dereference here because that only applies to the previous address):" << std::endl;
		
	std::cerr << "    With the -v flag the following address computation can be observed." << std::endl;
	std::cerr << "        Note: because we did not supply any data no writes are happening and we can freely experiment." << std::endl;
	std::cerr << std::endl;
	std::cerr << "Found module \"CT_FC5.dll\" at 0x7ffde4710000" << std::endl;
	std::cerr << "addr = \"CT_FC5.dll\" + 0x139830 = 0x7ffde4710000 + 0x139830" << std::endl;
	std::cerr << "addr = [\"CT_FC5.dll\" + 0x139830] + 0x8 = 0x14a8a4823c0 + 0x8" << std::endl;
	std::cerr << "addr = [[\"CT_FC5.dll\" + 0x139830] + 0x8] + 0x10 = 0x14a89903ff0 + 0x10" << std::endl;
	std::cerr << "Final address: 0x14a89904000" << std::endl;
	std::cerr << std::endl;

	std::cerr << std::endl;
	std::cerr << "data commands:" << std::endl;
}

int main(int argc, char* argv[]) {
	std::deque<std::string> args(argv, argv + argc);

	std::string processName = args.front();
	args.pop_front();

	if (args.size() < 2 || args.front() == "-h" || args.front() == "--help") {
		defaultHelp(processName);
		help(processName);
		return 1;
	}


	bool verbose = false;
	if (args.front() == "-v" || args.front() == "--verbose") {
		verbose = true;
		args.pop_front();
	}

	if (args.size() < 2) {
		std::cerr << "expected executable name / PID / window name and address definition" << std::endl;
		return 2;
	}

	std::string executableNameOrPIDOrWindowName = args.front();
	args.pop_front();

	std::string addressDefinition = args.front();
	args.pop_front();

	// https://stackoverflow.com/questions/4654636/how-to-determine-if-a-string-is-a-number-with-c/4654718#comment5126943_4654718
	const auto& is_number = [](const std::string& s) {
		return !s.empty() && s.find_first_not_of("0123456789") == std::string::npos;
	};

	// https://stackoverflow.com/a/2072890/2607571
	const auto& ends_with = [](const std::string& value, const std::string& ending) {
		if (ending.size() > value.size()) return false;
		return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
	};


	DWORD pid = 0;

	if (is_number(executableNameOrPIDOrWindowName)) {
		pid = std::stoi(executableNameOrPIDOrWindowName);

	} else if (ends_with(executableNameOrPIDOrWindowName, ".exe")) {
		PROCESSENTRY32 entry;
		entry.dwSize = sizeof(PROCESSENTRY32);

		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

		if (Process32First(snapshot, &entry)) {
			while (Process32Next(snapshot, &entry)) {

				if (executableNameOrPIDOrWindowName == entry.szExeFile) {
					pid = entry.th32ProcessID;
					break;
				}
			}
		}

		CloseHandle(snapshot);

		if (pid == 0) {
			std::cerr << "could not find process with executable \"" << executableNameOrPIDOrWindowName << "\"" << std::endl;
			return 3;
		}
	} else {
		HWND hWnd = FindWindowA(0, executableNameOrPIDOrWindowName.c_str());
		if (hWnd == NULL) {
			std::cerr << "could not find window with name \"" << executableNameOrPIDOrWindowName << "\"" << std::endl;
			return 4;
		}

		GetWindowThreadProcessId(hWnd, &pid);
	}

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

	if (hProcess == NULL) {
		std::cerr << "failed to open process with pid " << pid << std::endl;
		return 5;
	}

	if (verbose) std::cerr << "Successfully opened process" << std::endl;

	size_t firstColonPos = addressDefinition.find_first_of(':');

	if (firstColonPos == std::string::npos) {
		std::cerr << "address definition string incorrectly formated" << std::endl;
		return 6;
	}

	std::string offsetModuleName = addressDefinition.substr(0, firstColonPos);
	addressDefinition = addressDefinition.substr(firstColonPos + 1);

	HMODULE hMods[1024];
	DWORD cbNeeded;

	uintptr_t address = 0;

	if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
		for (unsigned i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
			char modName[MAX_PATH];

			if (GetModuleBaseNameA(hProcess, hMods[i], modName, sizeof(modName)) != NULL) {
				if (modName == offsetModuleName) {
					address = (uintptr_t)hMods[i];
					break;
				}
			}
		}
	}

	if (address == 0) {
		std::cerr << "could not find module \"" << offsetModuleName << "\" in process" << std::endl;
		return 7;
	}

	if (verbose) std::cerr << "Found module \"" << offsetModuleName << "\" at 0x" << std::hex << address << std::dec << std::endl;

	std::istringstream offsetStringStream(addressDefinition);
	std::string offsetToken;
	bool firstOffset = true;

	std::string addressString;

	while (offsetStringStream >> offsetToken) {
		long long offset;
		try {
			offset = std::stoll(offsetToken, 0, 0);
		} catch (const std::invalid_argument&) {
			std::cerr << "failed to parse token \"" << offsetToken << "\" in address definition" << std::endl;
			return 8;
		} catch (const std::out_of_range&) {
			std::cerr << "failed to parse token \"" << offsetToken << "\" in address definition (out of range)" << std::endl;
			return 9;
		}

		if (firstOffset) {
			firstOffset = false;

			std::ostringstream offsetModuleNameSS;

			offsetModuleNameSS << "\"" + offsetModuleName + "\" + 0x" << std::hex << offset;
			addressString = offsetModuleNameSS.str();

			// if (verbose) std::cerr << "addr = " << addressString << " = 0x" << std::hex << address << " + 0x" << offset << std::endl;
		} else {

			std::ostringstream offsetModuleNameSS;

			offsetModuleNameSS << "[" << addressString << "] + 0x" << std::hex << offset;
			addressString = offsetModuleNameSS.str();

			if (ReadProcessMemory(hProcess, (LPVOID)address, &address, sizeof(address), NULL) == FALSE) {
				std::cerr << "failed to read process memory at address " << std::hex << address << std::dec << std::endl;
				if (!verbose) std::cerr << "rerun with -v to see how far it got" << std::endl;
				return 10;
			}
		}

		if (verbose) std::cerr << "addr = " << addressString << " = 0x" << std::hex << address << " + 0x" << offset << std::endl;

		address += offset;
	}

	if (verbose) std::cerr << "Final address: 0x" << std::hex << address << std::dec << std::endl;

	int ret = processCommandArgs(args, address, hProcess, verbose);

	if (ret != 0) {
		return 10 + ret;	// ret > 0 -> error codes
	}

	CloseHandle(hProcess);
	return 0;
}