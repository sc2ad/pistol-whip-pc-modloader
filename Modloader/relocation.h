#pragma once

#include "framework.h"
#include "hook.h"
#include "assert_util.h"
#include "crt.h"

// bc i don't know where this is defined
#define EXIT_FAILURE 1

#pragma warning( disable : 4267 6387 6386 26451 )

typedef struct BASE_RELOCATION_BLOCK {
	DWORD PageAddress;
	DWORD BlockSize;
} BASE_RELOCATION_BLOCK, * PBASE_RELOCATION_BLOCK;

typedef struct BASE_RELOCATION_ENTRY {
	USHORT Offset : 12;
	USHORT Type : 4;
} BASE_RELOCATION_ENTRY, * PBASE_RELOCATION_ENTRY;

#define CountRelocationEntries(dwBlockSize)           \
      (dwBlockSize -                                  \
      sizeof(BASE_RELOCATION_BLOCK)) /                \
      sizeof(BASE_RELOCATION_ENTRY)

inline void copy_relocate(HMODULE module, void* source, void* dest, size_t size) {
	constexpr auto page_size = 0x1000;

	IMAGE_DOS_HEADER* mz = (PIMAGE_DOS_HEADER)module;
	IMAGE_NT_HEADERS* nt = RVA2PTR(PIMAGE_NT_HEADERS, mz, mz->e_lfanew);

	auto base = nt->OptionalHeader.ImageBase;
	auto sourceOffs = reinterpret_cast<size_t>(source) - base;
	auto destBase = reinterpret_cast<size_t>(dest) - sourceOffs;
	auto destOffs = reinterpret_cast<size_t>(dest) - destBase;
	auto sourceEndOffs = sourceOffs + size;
	auto destEndOffs = destOffs + size;

	auto sectionsCount = nt->FileHeader.NumberOfSections;
	PIMAGE_SECTION_HEADER sections = reinterpret_cast<PIMAGE_SECTION_HEADER>(&nt[1]);

	PIMAGE_SECTION_HEADER targetSection = nullptr;
	// find section the source is in
	for (auto s = 0; s < sectionsCount; ++s) {
		auto section = &sections[s];

		if (sourceOffs > section->VirtualAddress
			&& sourceOffs < (section->VirtualAddress) + (section->SizeOfRawData)) {
			targetSection = section;
			break;
		}
	}

	// find relocation block(s) for our section
	PBASE_RELOCATION_BLOCK targetPage = nullptr;
	PBASE_RELOCATION_BLOCK targetPage2 = nullptr;

	IMAGE_DATA_DIRECTORY relocData = nt->
		OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];

	size_t offs = 0;
	while (offs < relocData.Size) {
		auto blockHeader =
			reinterpret_cast<PBASE_RELOCATION_BLOCK>(base + relocData.VirtualAddress + offs);
		offs += blockHeader->BlockSize;

		auto pageStart = blockHeader->PageAddress;
		auto pageEnd = pageStart + page_size;

		if (sourceOffs > pageStart&& sourceOffs < pageEnd) {
			targetPage = blockHeader;
			continue;
		}
		if (sourceEndOffs > pageStart&& sourceEndOffs < pageEnd) {
			targetPage2 = blockHeader;
			continue;
		}
	}

	ASSERT(targetPage, L"Could not find source relocation page for %#p", source);

	// copy data
	LOG("Copying data of size: %lu from: %p to: %p", size, source, dest);
	memcpy(dest, source, size);

	// calculate offsets
	auto sourcePageStartOffs = sourceOffs - targetPage->PageAddress;
	size_t sourcePageEndOffs;
	{
		auto preloc = targetPage2 ? targetPage2 : targetPage;
		sourcePageEndOffs = sourceEndOffs - preloc->PageAddress;
	}

	// actually apply the relocations

	auto applyReloc = [&](PBASE_RELOCATION_BLOCK targetPage) {
		auto entryCount = CountRelocationEntries(targetPage->BlockSize);
		auto entries = reinterpret_cast<PBASE_RELOCATION_ENTRY>(reinterpret_cast<size_t>(targetPage) + sizeof(BASE_RELOCATION_BLOCK));

		auto diff = destBase - base;

		for (auto i = 0; i < entryCount; ++i) {
			auto entry = entries[i];

			auto fullOffs = targetPage->PageAddress + entry.Offset;

			if (fullOffs < destOffs || fullOffs >= destEndOffs) continue;

			void* ptr = reinterpret_cast<void*>(destBase + fullOffs);

			switch (entry.Type) {
			case IMAGE_REL_BASED_DIR64: {
				auto loc = reinterpret_cast<size_t*>(ptr); // size_t is 64 bits
				*loc += diff;
			}

			default:
				break;
			}
		}
	};

	applyReloc(targetPage);
	if (targetPage2) applyReloc(targetPage2);
}