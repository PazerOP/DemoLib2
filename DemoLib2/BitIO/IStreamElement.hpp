#pragma once

class BitIOReader;
class BitIOWriter;

class IStreamElement
{
public:
	virtual ~IStreamElement() = default;

	void ReadElement(BitIOReader& reader);
	void WriteElement(BitIOWriter& writer) const;

protected:
	virtual IStreamElement* CreateNewInstance() const = 0;
	virtual void ReadElementInternal(BitIOReader& reader) = 0;
	virtual void WriteElementInternal(BitIOWriter& writer) const = 0;
};