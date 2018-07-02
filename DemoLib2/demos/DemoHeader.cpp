#include "DemoHeader.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"

#include <iterator>

void DemoHeader::ReadElementInternal(BitIOReader& reader)
{
	reader.ReadArray(m_MagicToken, std::size(m_MagicToken));
	reader.Read(m_DemoProtocol);
	reader.Read(m_NetworkProtocol);

	reader.ReadArray(m_ServerName, std::size(m_ServerName));
	reader.ReadArray(m_ClientName, std::size(m_ClientName));
	reader.ReadArray(m_MapName, std::size(m_MapName));
	reader.ReadArray(m_GameDirectory, std::size(m_GameDirectory));

	reader.Read(m_PlaybackTime);
	reader.Read(m_PlaybackTicks);
	reader.Read(m_PlaybackFrames);
	reader.Read(m_SignonLength);
}

void DemoHeader::WriteElementInternal(BitIOWriter& writer) const
{
	writer.WriteChars(m_MagicToken, std::size(m_MagicToken));
	writer.Write(m_DemoProtocol);
	writer.Write(m_NetworkProtocol);

	writer.WriteChars(m_ServerName, std::size(m_ServerName));
	writer.WriteChars(m_ClientName, std::size(m_ClientName));
	writer.WriteChars(m_MapName, std::size(m_MapName));
	writer.WriteChars(m_GameDirectory, std::size(m_GameDirectory));

	writer.Write(m_PlaybackTime);
	writer.Write(m_PlaybackTicks);
	writer.Write(m_PlaybackFrames);
	writer.Write(m_SignonLength);
}