#include "BitIO/BitIOReader.hpp"
#include "misc/Util.hpp"
#include "net/data/SourceConstants.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <stdexcept>
#include <cinttypes>

BitIOBase::BitIOBase(const std::shared_ptr<const std::byte[]>& baseHandle)
{
	m_SharedData = std::make_shared<SharedData>();
	if (baseHandle)
	{
		m_SharedData->m_MemoryHandle = std::static_pointer_cast<const std::byte>(baseHandle);
		m_SharedData->m_Base = m_SharedData->m_MemoryHandle.get();
	}
}

void BitIOBase::Seek(const BitPosition& offset, ::Seek seekDir, bool backwards)
{
	switch (seekDir)
	{
		case Seek::Set:
			if (backwards)
				throw std::invalid_argument("backwards mode not valid for Seek::Set");
			if (offset < StartPosition())
				throw std::invalid_argument("Attempted to seek beyond the start");
			if (offset > EndPosition())
				throw std::invalid_argument("Attempted to seek beyond the end");

			m_Position = offset;
			break;

		case Seek::Cur:
			if (backwards)
				m_Position -= offset;
			else
			{
				const auto newPosition = m_Position + offset;
				if (newPosition > EndPosition())
					throw std::invalid_argument("Attempted to seek beyond the end");

				m_Position = newPosition;
			}

			break;

		case Seek::End:
			if (backwards)
				throw std::invalid_argument("backwards mode not valid for Seek::End");

			m_Position = EndPosition() - offset;
			break;

		case Seek::Start:
			if (backwards)
				throw std::invalid_argument("backwards mode not valid for Seek::Start");

			m_Position = StartPosition() + offset;
			break;

		default:
			throw std::invalid_argument("seekDir was not Seek::Set, Seek::Cur, Seek::End, or Seek::Start.");
	}
}

void BitIOBase::SeekBits(intmax_t offset, ::Seek seekDir)
{
	if (offset < 0)
		Seek(BitPosition::FromBits(-offset), seekDir, true);
	else
		Seek(BitPosition::FromBits(offset), seekDir, false);
}

void BitIOBase::SeekBytes(intmax_t offset, ::Seek seekDir)
{
	if (offset < 0)
		Seek(BitPosition::FromBytes(-offset), seekDir, true);
	else
		Seek(BitPosition::FromBytes(offset), seekDir, false);
}