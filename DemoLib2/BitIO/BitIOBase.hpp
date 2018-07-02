#pragma once
#include "BitIOCore.hpp"
#include "BitPosition.hpp"
#include "misc/Vector.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>

enum class Seek : uint_fast8_t
{
	// Sets the absolute position. GetPosition() should return the same value you passed into Seek().
	Set = SEEK_SET,
	Cur = SEEK_CUR,
	End = SEEK_END,

	// Seeks from the StartPosition(). GetPosition() should return StartPosition() after a call to Seek(0).
	Start,
};
static_assert(Seek::Start != Seek::Set && Seek::Start != Seek::Cur && Seek::Start != Seek::End);

// Only stores information related to position. Doesn't store any data pointers.
class BitIOBase : protected BitIOCore
{
public:
	BitIOBase(const std::shared_ptr<const std::byte[]>& baseHandle);

	const BitPosition& GetPosition() const { return m_Position; }
	BitPosition GetLocalPosition() const { return m_Position - m_StartPosition; }
	const BitPosition& StartPosition() const { return m_StartPosition; }
	const BitPosition& EndPosition() const { return m_EndPosition; }
	bool IsAtEnd() const { return GetPosition() == EndPosition(); }

	BitPosition Length() const { return EndPosition() - StartPosition(); }
	BitPosition Remaining() const { return EndPosition() - GetPosition(); }

	void Seek(const BitPosition& offset, ::Seek seekMode = ::Seek::Cur, bool backwards = false);
	void SeekBits(intmax_t offset, ::Seek seekMode = Seek::Cur);
	void SeekBytes(intmax_t offset, ::Seek seekMode = Seek::Cur);

protected:
	// Only subclasses are allowed to directly modify these
	//BitPosition& GetPosition() { return m_Position; }
	//BitPosition& EndPosition() { return const_cast<BitPosition&>(const_cast<const BitIOBase*>(this)->EndPosition()); }

	const std::byte* GetBase() const { return m_SharedData->m_Base; }

	static void ArrayDeleter(const std::byte* p) { delete[] p; }
	static void PointerDeleter(const std::byte* p) { delete p; }
	static void PointerFree(const std::byte* p) { free(const_cast<std::byte*>(p)); }

	BitPosition m_StartPosition;
	BitPosition m_Position;       // Current position
	BitPosition m_EndPosition;

	template<class T> struct pointer_free
	{
		constexpr pointer_free() noexcept = default;
		void operator()(T* ptr) const { free((void*)ptr); }
	};

	struct SharedData
	{
		SharedData() : m_AutoGrowHandle(nullptr, pointer_free<const std::byte>())
		{
			m_Base = nullptr;
			m_AutoGrowCapacity = 0;
		}

		// Debatabely faster access. Code that updates m_MemoryHandle or m_AutoGrowHandle
		// is expected to also update m_Base.
		const std::byte* m_Base;

		std::shared_ptr<const std::byte> m_MemoryHandle;
		std::unique_ptr<const std::byte, pointer_free<const std::byte>> m_AutoGrowHandle;
		size_t m_AutoGrowCapacity;
	};
	SharedData& GetSharedData() { return *m_SharedData; }
	const SharedData& GetSharedData() const { return *m_SharedData; }

private:
	std::shared_ptr<SharedData> m_SharedData;
};