#pragma once
#include "Crc32.h"

namespace Memory {
	inline std::uintptr_t trampHook(std::uintptr_t* src_addr, const std::uintptr_t dst_addr, const std::uintptr_t len)
	{
		auto* mem = VirtualAlloc(nullptr, len + 5, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		memcpy(mem, src_addr, len);

		const auto delta_addr = reinterpret_cast<std::uintptr_t>(src_addr) - reinterpret_cast<std::uintptr_t>(mem) - 5;

		//redirect to alloc'd memory
		*reinterpret_cast<char*>(reinterpret_cast<std::uintptr_t>(mem) + len) = static_cast<char>(0xE9);
		*reinterpret_cast<std::uintptr_t*>(reinterpret_cast<std::uintptr_t>(mem) + len + 1) = delta_addr;

		DWORD dw_old_protect = 0;
		VirtualProtect(src_addr, len, PAGE_EXECUTE_READWRITE, &dw_old_protect);
		const auto delta_addr_mem = static_cast<std::uintptr_t>(dst_addr - reinterpret_cast<std::uintptr_t>(src_addr)) - 5;

		//redirect to actual function
		*src_addr = 0xE9;
		*reinterpret_cast<std::uintptr_t*>(reinterpret_cast<std::uintptr_t>(src_addr) + 1) = delta_addr_mem;

		VirtualProtect(src_addr, len, dw_old_protect, &dw_old_protect);
		return reinterpret_cast<std::uintptr_t>(mem);
	}

	inline std::uintptr_t relativeAddress(const std::uintptr_t addr, const size_t offs)
	{
		if (!addr)
			return NULL;

		auto out = addr + offs;
		const auto ret = *reinterpret_cast<std::uint32_t*>(out);

		if (!ret)
			return NULL;

		out = out + 4 + ret;
		return out;
	}

	inline std::uintptr_t getSizeOfNt(IMAGE_NT_HEADERS* nt)
	{
		return offsetof(IMAGE_NT_HEADERS, OptionalHeader) + nt->FileHeader.SizeOfOptionalHeader + nt->FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER);
	}

	inline std::uintptr_t hashHeader(void* base)
	{
		auto* dos = static_cast<PIMAGE_DOS_HEADER>(base);
		if (!dos)
			return 0;

		auto* nt = reinterpret_cast<IMAGE_NT_HEADERS*>(reinterpret_cast<std::uintptr_t>(dos) + dos->e_lfanew);
		if (nt->Signature != 0x4550) // compilation mode
			return 0;

		return Crc32::hash(reinterpret_cast<PUCHAR>(nt), getSizeOfNt(nt), 0xFFFFFFFF);
	}

	inline std::uint8_t* patternScanIda(HMODULE h_module, const char* signature)
	{
		static auto pattern_to_byte = [](const char* pattern)
		{
			auto bytes = std::vector<int>{};
			auto* const start = const_cast<char*>(pattern);
			auto* const end = const_cast<char*>(pattern) + strlen(pattern);

			for (auto* current = start; current < end; ++current)
			{
				if (*current == '?')
				{
					++current;
					if (*current == '?')
						++current;

					bytes.push_back(-1);
				}
				else
					bytes.push_back(strtoul(current, &current, 16));
			}
			return bytes;
		};

		auto* const dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(h_module);
		auto* const nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<std::uint8_t*>(h_module) + dos_header->e_lfanew);

		const auto size_of_image = nt_headers->OptionalHeader.SizeOfImage;
		auto* const scan_bytes = reinterpret_cast<std::uint8_t*>(h_module);

		auto pattern_bytes = pattern_to_byte(signature);

		const auto s = pattern_bytes.size();
		auto* const d = pattern_bytes.data();

		for (auto i = 0ul; i < size_of_image - s; ++i)
		{
			auto found = true;
			for (auto j = 0ul; j < s; ++j)
			{
				if (scan_bytes[i + j] != d[j] && d[j] != -1)
				{
					found = false;
					break;
				}
			}

			if (found)
				return &scan_bytes[i];
		}
		return nullptr;
	}

	inline VOID hookIAT(PCWSTR moduleName, PCSTR importModuleName, PCSTR functionName, PVOID fun) {
		PBYTE module = (PBYTE)GetModuleHandleW(moduleName);

		if (module) {
			//get imports
			PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)(module + ((PIMAGE_DOS_HEADER)module)->e_lfanew);
			PIMAGE_IMPORT_DESCRIPTOR imports = (PIMAGE_IMPORT_DESCRIPTOR)(module + ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

			//find function
			for (PIMAGE_IMPORT_DESCRIPTOR import = imports; import->Name; import++) {
				if (_strcmpi((char*)DWORD(module) + import->Name, importModuleName))
					continue;

				//replace function
				for (PIMAGE_THUNK_DATA original_first_thunk = (PIMAGE_THUNK_DATA)(module + import->OriginalFirstThunk), first_thunk = (PIMAGE_THUNK_DATA)(module + import->FirstThunk); original_first_thunk->u1.AddressOfData; original_first_thunk++, first_thunk++) {
					if (strcmp((PCSTR)((PIMAGE_IMPORT_BY_NAME)(module + original_first_thunk->u1.AddressOfData))->Name, functionName))
						continue;

					auto functionAddress = &first_thunk->u1.Function;

					DWORD old;
					if (VirtualProtect(functionAddress, sizeof(fun), PAGE_READWRITE, &old)) {
						*functionAddress = (DWORD)fun;
						VirtualProtect(functionAddress, sizeof(fun), old, &old);
					}
					break;
				}
				break;
			}
		}
	}
}