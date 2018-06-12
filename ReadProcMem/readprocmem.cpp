#include "../implementation.h"

#include <iostream>
#include <deque>
#include <string>
#include <sstream>
#include <Windows.h>
#include <tlhelp32.h>
#include <psapi.h>

void help(const std::string& processName) {
	std::cerr << "  -f | --float   reads single precision float" << std::endl;
	std::cerr << "  -d | --double  reads double precision float" << std::endl;
	std::cerr << "  -i[8|16|32|64] | --integer[8|16|32|64]   reads 8|16|32|64 bit integer respectively" << std::endl;
	std::cerr << "  -u[8|16|32|64] | --unsigned[8|16|32|64]  reads 8|16|32|64 bit unsigned integer respectively" << std::endl;
	std::cerr << "  -s n  skips n bytes e.g. -f -s 4 -f skips 1 float in the middle" << std::endl;
}

int processCommandArgs(std::deque<std::string> args, uintptr_t& address, HANDLE& hProcess, bool verbose) {
	bool first = true;

	while (args.size() != 0) {
		if (!first)
			std::cout << " ";

		first = false;

		std::string type = args.front();
		args.pop_front();

		bool error = false;

		const auto& readFromProcess = [&verbose, &error, &address, &hProcess](const auto& data) {
			if (ReadProcessMemory(hProcess, (LPVOID)address, (LPVOID)&data, sizeof(data), NULL) == FALSE) {
				std::cerr << "failed to read process memory at address " << std::hex << address << std::dec << std::endl;
				if (!verbose) std::cerr << "rerun with -v to see how far it got" << std::endl;
				error = true;
			}

			address += sizeof(data);
		};

		//TODO: make this prettier (maybe in a list with lambdas?)

		if (type == "-i8" || type == "--int8") {
			char data;
			readFromProcess(data);
			std::cout << (int)data;
		} else if (type == "-u8" || type == "--unsigned8") {
			unsigned char data;
			readFromProcess(data);
			std::cout << (unsigned)data;
		} else if (type == "-i16" || type == "--int16") {
			short data;
			readFromProcess(data);
			std::cout << data;
		} else if (type == "-u16" || type == "--unsigned16") {
			unsigned short data;
			readFromProcess(data);
			std::cout << data;
		} else if (type == "-i32" || type == "--int32") {
			int data;
			readFromProcess(data);
			std::cout << data; // sizeof(int) = 4
		} else if (type == "-u32" || type == "--unsigned32") {
			unsigned int data;
			readFromProcess(data);
			std::cout << data; // sizeof(unsigned int) = 4
		} else if (type == "-i64" || type == "--int64") {
			long long data;
			readFromProcess(data);
			std::cout << data; // sizeof(long long) = 8
		} else if (type == "-u64" || type == "--unsigned64") {
			unsigned long long data;
			readFromProcess(data);
			std::cout << data; // sizeof(unsigned long long) = 8
		} else if (type == "-f" || type == "--float") {
			//TODO: for float and double investigate precision

			float data;
			readFromProcess(data);
			std::cout << data;
		} else if (type == "-d" || type == "--double") {
			double data;
			readFromProcess(data);
			std::cout << data;
		} else if (type == "-s" || type == "--skip") {
			if (args.size() == 0) {
				std::cerr << "expected skip count after -s" << std::endl;
				return 1;
			}
			try {
				address += std::stoi(args.front(), 0, 0);	// supporting negative offset because why not
			} catch (const std::invalid_argument&) {
				std::cerr << "failed to parse " << type << " with value " << args.front() << std::endl;
				return 4;
			} catch (const std::out_of_range&) {
				std::cerr << "failed to parse " << type << " with value " << args.front() << " is out of range" << std::endl;
				return 5;
			}

			args.pop_front();
		} else {
			std::cerr << "unknown type \"" << type << "\"" << std::endl;
			return 3;
		}

		if (error) return 2;
	}

	return 0;
}