#include "MemoryManager.hpp"

#include <cassert>
#include <mutex>

#include <Windows.h>

#undef min
#undef max


struct MemoryBlock
{
	bool m_Allocated = false;
	bool m_PageBegin = false;
	bool m_PageEnd = false;
	std::byte* m_Address = nullptr;
	size_t m_Size = 0;

	MemoryBlock* m_Previous = nullptr;
	MemoryBlock* m_Next = nullptr;

	void InsertBefore(MemoryBlock* existing, MemoryBlock*& list);
	void InsertAfter(MemoryBlock* existing);
};

static constexpr auto s_MemoryBlockSize = sizeof(MemoryBlock);

class MemoryManager
{
public:
	MemoryManager();

	void* Alloc(size_t size);
	void Free(void* address);

	std::mutex m_MemoryManagerMutex;

private:
	void* AllocLargePage(size_t pageCount = 1) const;
	void AllocMoreBlocks();
	MemoryBlock* GetUnusedMemoryBlock();


	MemoryBlock* m_UsedBlocks = nullptr;
	MemoryBlock* m_UnusedBlocks = nullptr;

	const size_t LARGE_PAGE_SIZE;
};

static MemoryManager& GetMemoryManager()
{
	static MemoryManager s_MemoryManager;
	return s_MemoryManager;
}

void* MemoryManager::AllocLargePage(size_t pageCount) const
{
	auto newPage = VirtualAlloc(nullptr, LARGE_PAGE_SIZE * pageCount, MEM_LARGE_PAGES | MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	auto lastError = newPage ? 0 : GetLastError();

	return newPage;
}

void MemoryBlock::InsertBefore(MemoryBlock* insertBefore, MemoryBlock*& list)
{
	assert(insertBefore != this);

	assert(!insertBefore == !list);
	if (insertBefore == list)
		list = this;

	if (insertBefore)
	{
		if (insertBefore->m_Previous)
			insertBefore->m_Previous->m_Next = this;

		m_Previous = insertBefore->m_Previous;
		insertBefore->m_Previous = this;
		m_Next = insertBefore;
	}
	else
	{
		m_Previous = nullptr;
		m_Next = nullptr;
	}
}
void MemoryBlock::InsertAfter(MemoryBlock* insertAfter)
{
	assert(insertAfter != this);

	m_Next = insertAfter->m_Next;

	if (insertAfter->m_Next)
	{
		insertAfter->m_Next->m_Previous = this;
		insertAfter->m_Next = this;
	}

	m_Previous = insertAfter;
}

// Removes a MemoryBlock from its linked list
static void ExtractMemoryBlock(MemoryBlock* block, MemoryBlock*& list)
{
	if (block->m_Previous)
		block->m_Previous->m_Next = block->m_Next;
	if (block->m_Next)
		block->m_Next->m_Previous = block->m_Previous;
	if (block == list)
		list = block->m_Next;

	block->m_Next = nullptr;
	block->m_Previous = nullptr;
}

void MemoryManager::AllocMoreBlocks()
{
	MemoryBlock* newBlocks = (MemoryBlock*)AllocLargePage();

	const auto newBlockCount = LARGE_PAGE_SIZE / s_MemoryBlockSize;

	// Add all new blocks to the head of the list
	for (size_t i = 0; i < newBlockCount; i++)
	{
		MemoryBlock* newBlock = &newBlocks[i];
		newBlock->InsertBefore(m_UnusedBlocks, m_UnusedBlocks);
		m_UnusedBlocks = newBlock;
	}

	// Mark new page as allocated
	{
		MemoryBlock* pageBlock = m_UnusedBlocks;
		ExtractMemoryBlock(pageBlock, m_UnusedBlocks);
		pageBlock->InsertBefore(m_UsedBlocks, m_UsedBlocks);

		pageBlock->m_Address = reinterpret_cast<std::byte*>(newBlocks);
		pageBlock->m_Allocated = true;
		pageBlock->m_Size = newBlockCount * s_MemoryBlockSize;

		pageBlock->m_PageBegin = true;
		pageBlock->m_PageEnd = true;

		if (pageBlock->m_Size < LARGE_PAGE_SIZE)
		{
			// Extra space at the end of the page of MemoryBlocks
		}
	}
}

MemoryBlock* MemoryManager::GetUnusedMemoryBlock()
{
	if (m_UnusedBlocks)
	{
		MemoryBlock* unused = m_UnusedBlocks;
		ExtractMemoryBlock(unused, m_UnusedBlocks);
		return unused;
	}
	else
	{
		AllocMoreBlocks();
		assert(m_UnusedBlocks);
		return GetUnusedMemoryBlock();
	}
}

void* MemoryManager::Alloc(size_t size)
{
	for (auto block = m_UsedBlocks; block; block = block->m_Next)
	{
		if (block->m_Allocated)
			continue;

		if (block->m_Size < size)
			continue;

		if (block->m_Size > size)
		{
			MemoryBlock* trailingFree = GetUnusedMemoryBlock();
			trailingFree->m_Address = block->m_Address + size;
			trailingFree->m_Size = block->m_Size - size;
			trailingFree->m_Allocated = false;

			// Transfer "page end" status
			trailingFree->m_PageEnd = block->m_PageEnd;

			// Trailing free can never be page begin, and newly alloc'd block can never be page end
			trailingFree->m_PageBegin = false;
			block->m_PageEnd = false;

			trailingFree->InsertAfter(block);
		}

		block->m_Size = size;
		block->m_Allocated = true;

		return block->m_Address;
	}

	// Create new large page, then try again
	{
		const auto pageCount = (size + LARGE_PAGE_SIZE - 1) / LARGE_PAGE_SIZE;

		auto newBlock = GetUnusedMemoryBlock();
		newBlock->m_Address = reinterpret_cast<std::byte*>(AllocLargePage(pageCount));
		newBlock->m_PageBegin = newBlock->m_PageEnd = true;
		newBlock->m_Size = LARGE_PAGE_SIZE * pageCount;
		newBlock->m_Allocated = false;

		newBlock->InsertBefore(m_UsedBlocks, m_UsedBlocks);

		return Alloc(size);
	}
}

void MemoryManager::Free(void* address)
{
	for (auto block = m_UsedBlocks; block; block = block->m_Next)
	{
		if (block->m_Address != address)
			continue;

		if (!block->m_Allocated)
			throw MemoryException("Block was already freed");

		block->m_Allocated = false;

		if (!block->m_PageBegin && block->m_Previous && !block->m_Previous->m_Allocated)
		{
			// Merge with previous
			MemoryBlock* prev = block->m_Previous;
			prev->m_Size += block->m_Size;
			prev->m_PageEnd = block->m_PageEnd;

			ExtractMemoryBlock(block, m_UsedBlocks);
			block->InsertBefore(m_UnusedBlocks, m_UnusedBlocks);

			block = prev;
		}
		if (!block->m_PageEnd && block->m_Next && !block->m_Next->m_Allocated)
		{
			// Merge with next
			MemoryBlock* next = block->m_Next;
			block->m_Size += next->m_Size;
			block->m_PageEnd = next->m_PageEnd;

			ExtractMemoryBlock(next, m_UsedBlocks);
			next->InsertBefore(m_UnusedBlocks, m_UnusedBlocks);
		}

		return;
	}

	throw MemoryException("Attempted to delete unmanaged address");
}

#if 0
void* operator new(size_t size)
{
	std::lock_guard<decltype(GetMemoryManager().m_MemoryManagerMutex)> lock(GetMemoryManager().m_MemoryManagerMutex);
	return GetMemoryManager().Alloc(size);
}
void operator delete(void* address)
{
	if (!address)
		return;

	std::lock_guard<decltype(GetMemoryManager().m_MemoryManagerMutex)> lock(GetMemoryManager().m_MemoryManagerMutex);
	GetMemoryManager().Free(address);
}
#endif

MemoryManager::MemoryManager() : LARGE_PAGE_SIZE(GetLargePageMinimum())
{
	LUID lockMemoryLUID;
	if (!LookupPrivilegeValueA(nullptr, "SeLockMemoryPrivilege", &lockMemoryLUID))
		throw MemoryException("Failed to find SeLockMemoryPrivilege LUID");

	TOKEN_PRIVILEGES tp;
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = lockMemoryLUID;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	auto currentProcess = GetCurrentProcess();

	HANDLE processToken;
	if (!OpenProcessToken(currentProcess, TOKEN_ADJUST_DEFAULT | TOKEN_ADJUST_PRIVILEGES, &processToken))
		throw MemoryException("Unable to OpenProcessToken");

	if (!AdjustTokenPrivileges(processToken, FALSE, &tp, sizeof(tp), nullptr, nullptr))
		throw MemoryException("Failed to AdjustTokenPrivileges to add SeLockMemoryPrivilege");

	if (!CloseHandle(processToken))
		throw MemoryException("Failed to close process access token");
}
