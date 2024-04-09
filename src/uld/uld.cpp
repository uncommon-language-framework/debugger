#include <StdULR.hpp>
#include <iostream>
#include <vector>
#include <memory>
#include <windows.h>
#include <psapi.h>
#include <dbghelp.h>
#include <tlhelp32.h>

#include "Lib/Process.hpp"

ULRAPIImpl* api;

std::vector<std::string> split_command(std::string command)
{
	std::vector<std::string> parts;

	size_t start = -1;

	do
	{
		size_t found = command.find(' ', start+1);

		parts.push_back(command.substr(start+1, found));

		start = found;
	} while (start < command.size());

	return parts;
}

bool process_command()
{
	std::string cmd;

	std::cout << "uld > ";

	std::getline(std::cin, cmd);

	std::vector<std::string> parts = split_command(cmd);

	if (parts.size() == 0) return true;

	if ((parts[0] == "continue") || (parts[0] == "cont") || (parts[0] == "c"))
		return false;

	if ((parts[0] == "exit") || (parts[0] == "quit") || (parts[0] == "q"))
		exit(0);
	
	if ((parts[0] == "print") || (parts[0] == "p"))
	{
		if (parts.size() < 2)
		{
			std::cerr << "No data specified to print!\n";
			return true;
		}

		if ((parts[1] == "allocated") || (parts[1] == "alloc") || (parts[1] == "alloced"))
		{			
			for (const auto& entry : api->allocated_objs)
			{
				std::cout << api->GetDisplayNameOf(api->GetTypeOf(entry.first)) << " (" << entry.second << " bytes)\n";
			}

			std::cout << api->allocated_objs.size() << " objects, " << api->allocated_size << " bytes allocated total.\n\n";

			for (void* addr : api->allocated_field_offsets)
			{
				std::cout << addr << " also allocated as a runtime generic static field offset\n";
			}

			return true;
		}

		if ((parts[1] == "asm") || (parts[1] == "asms") || (parts[1] == "assembly") || (parts[1] == "assemblies"))
		{
			size_t read_asms = 0;
			size_t loaded_asms = 0;

			for (const auto& entry : *api->read_assemblies)
			{
				if (api->assemblies->count(entry.first))
				{
					std::cout << entry.first << " (" << entry.second->types.size() << " types)\n";
				
					loaded_asms+=1;
				}
				else
				{
					std::cout << "(not fully loaded) " << entry.first << " (" << entry.second->types.size() << " types)\n";

					read_asms+=1;
				}
			}

			std::cout << read_asms << " read, " << loaded_asms << " loaded, " << read_asms+loaded_asms << " total.\n";

			return true;
		}

		if ((parts[1] == "type") || (parts[1] == "types"))
		{
			size_t read_types = 0;
			size_t loaded_types = 0;

			for (const auto& entry : *api->read_assemblies)
			{
				if (api->assemblies->count(entry.first))
				{
					for (const auto& type_entry : entry.second->types)
					{
						std::cout << api->GetDisplayNameOf(type_entry.second) << " (" << entry.first << ")\n";
					}

					loaded_types+=1;
				}
				else
				{
					for (const auto& type_entry : entry.second->types)
					{
						std::cout << "(not fully loaded) " << api->GetDisplayNameOf(type_entry.second) << " (" << entry.first << ")\n";
					}

					read_types+=1;
				}
			}

			std::cout << read_types << " read, " << loaded_types << " loaded, " << read_types+loaded_types << " total.\n";

			return true;
		}
	}

	return true;
}

BEGIN_ULR_EXPORT

void InitDebugger(ULRAPIImpl* api_inject)
{
	api = api_inject;
}

void StaticDebug(StaticDebugInfo& info)
{
	// HANDLE snapshot;
	// HANDLE proc;
	// DWORD attach_proc_id;

	// // TODO: err check
	// if (argc < 2)
	// {
	// 	std::cerr << "No process ID/assembly specified!" << std::endl;

	// 	return 1;
	// }
	// else
	// {
	// 	try
	// 	{
	// 		attach_proc_id = std::stol(argv[1]);

	// 		snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, attach_proc_id);
	// 		proc = OpenProcess(PROCESS_VM_READ, false, attach_proc_id);

	// 		if (snapshot == INVALID_HANDLE_VALUE || proc == INVALID_HANDLE_VALUE)
	// 		{
	// 			std::cerr << "Could not load process " << attach_proc_id << ".\n";

	// 			return 1;
	// 		}
	// 	}
	// 	catch (std::invalid_argument& exc)
	// 	{
	// 		std::cerr << "Invalid process ID!\n";

	// 		return 1;
	// 	}

	// }

	// MODULEENTRY32 mod_entry;
	// mod_entry.dwSize = sizeof(mod_entry);

	// HMODULE native_lib_handle = nullptr;


	// if (Module32First(snapshot, &mod_entry))
	// {
	// 	do
	// 	{
	// 		// apparently this is actually a char and not a wchar
	// 		if (std::string_view(mod_entry.szModule) == "ULR.NativeLib.dll")
	// 		{
	// 			native_lib_handle = mod_entry.hModule;
	// 			break;
	// 		}
	// 	} while (Module32Next(snapshot, &mod_entry));
	// }

	// if (!native_lib_handle)
	// {
	// 	std::cerr << "The ULR Native Library could not be found in process " << attach_proc_id << ". Double-check that the process is running a ULR application.\n";

	// 	return 1;
	// }

	// ULRAPIImpl** api_addr_ptr = (ULRAPIImpl**) GetProcAddress(native_lib_handle, "internal_api");

	// size_t tries = 1;

	// while (!api_addr_ptr)
	// {
	// 	tries+=1;

	// 	Sleep(10);
	// 	api_addr_ptr = (ULRAPIImpl**) GetProcAddress(native_lib_handle, "internal_api");
		

	// 	if (tries % 100 == 0)
	// 	{
	// 		std::cout << "Trying to load ULRAPI (try #" << tries << ") ...\n";
	// 	}
	// }

	std::cout << '\n'
		<< "ULD: Static Breakpoint " << info.breakpoint_no << " @ file '" << info.source_filename << "' line " << info.source_lineno << ":\n"
		<< info.source_lineno << ' ' << info.source_line << '\n';

	while (process_command()); // process_command will return false on exitx
}

END_ULR_EXPORT