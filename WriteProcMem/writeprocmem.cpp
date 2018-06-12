#include "../implementation.h"

#include <iostream>
#include <deque>
#include <string>
#include <sstream>
#include <Windows.h>
#include <tlhelp32.h>
#include <psapi.h>

void help(const std::string& processName) {
	std::cerr << "  -f | --float   writes single precision float e.g. -f 22.5 or -f Nan or -f inf or -f 0X1.BC70A3D70A3D7P+6" << std::endl;
	std::cerr << "  -d | --double  writes double precision float e.g. same as float" << std::endl;
	std::cerr << "  -i[8|16|32|64] | --integer[8|16|32|64]   writes 8|16|32|64 bit integer respectively e.g. -i32 -123 or -i16 0xcaffe or -i64 -0xe" << std::endl;
	std::cerr << "  -u[8|16|32|64] | --unsigned[8|16|32|64]  writes 8|16|32|64 bit unsigned integer respectively e.g. same as integer" << std::endl;
	std::cerr << "  -s n  skips n bytes e.g. -f 1 -s 4 -f 0 skips 1 float in the middle" << std::endl;
}

int processCommandArgs(std::deque<std::string> args, uintptr_t& address, HANDLE& hProcess, bool verbose) {

	while (args.size() != 0) {
		std::string type = args.front();
		args.pop_front();

		if (args.size() == 0) {
			std::cerr << "expected data after type" << std::endl;
			return 1;
		}

		std::string data = args.front();
		args.pop_front();

		bool error = false;

		const auto& writeToProcess = [&verbose, &error, &address, &hProcess](const auto& data) {
			if (WriteProcessMemory(hProcess, (LPVOID)address, &data, sizeof(data), NULL) == FALSE) {
				std::cerr << "failed to write process memory at address " << std::hex << address << std::dec << std::endl;
				if (!verbose) std::cerr << "rerun with -v to see how far it got" << std::endl;
				error = true;
			}

			address += sizeof(data);
		};

		//TODO: make this prettier (maybe in a list with lambdas?)
		try {
			if (type == "-i8" || type == "--int8") {
				writeToProcess((char)std::stoi(data, 0, 0));
			} else if (type == "-u8" || type == "--unsigned8") {
				writeToProcess((unsigned char)std::stoul(data, 0, 0));
			} else if (type == "-i16" || type == "--int16") {
				writeToProcess((short)std::stoi(data, 0, 0));
			} else if (type == "-u16" || type == "--unsigned16") {
				writeToProcess((unsigned short)std::stoul(data, 0, 0));
			} else if (type == "-i32" || type == "--int32") {
				writeToProcess(std::stoi(data, 0, 0)); // sizeof(int) = 4
			} else if (type == "-u32" || type == "--unsigned32") {
				writeToProcess(std::stoul(data, 0, 0)); // sizeof(long) = 4
			} else if (type == "-i64" || type == "--int64") {
				writeToProcess(std::stoll(data, 0, 0)); // sizeof(long long) = 8
			} else if (type == "-u64" || type == "--unsigned64") {
				writeToProcess(std::stoull(data, 0, 0)); // sizeof(unsigned long long) = 8
			} else if (type == "-f" || type == "--float") {
				writeToProcess(std::stof(data));
			} else if (type == "-d" || type == "--double") {
				writeToProcess(std::stod(data));
			} else if (type == "-s" || type == "--skip") {
				address += std::stoi(data, 0, 0);	// supporting negative offset because why not
			} else {
				std::cerr << "unknown type \"" << type << "\"" << std::endl;
				return 3;
			}
		} catch (const std::invalid_argument&) {
			std::cerr << "failed to parse " << type << " with value " << data << std::endl;
			return 4;
		} catch (const std::out_of_range&) {
			std::cerr << "failed to parse " << type << " with value " << data << " is out of range" << std::endl;
			return 5;
		}

		if (error) return 2;
	}

	return 0;
}