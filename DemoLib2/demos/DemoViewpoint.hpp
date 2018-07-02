#pragma once

#include "BitIO/IStreamElement.hpp"
#include "demos/DemoViewpointFlags.hpp"
#include "misc/Vector.hpp"

class DemoViewpoint final : public IStreamElement
{
public:
	const Vector& GetViewPos() const { return ((int)m_Flags & (int)DemoViewpointFlags::UseOrigin2) ? m_ViewOrigin2 : m_ViewOrigin1; }

protected:
	void ReadElementInternal(BitIOReader& reader) override;
	void WriteElementInternal(BitIOWriter& writer) const override;
	IStreamElement* CreateNewInstance() const override { return new DemoViewpoint(); }

private:
	DemoViewpointFlags m_Flags;

	Vector m_ViewOrigin1;
	Vector m_ViewAngles1;
	Vector m_LocalViewAngles1;

	Vector m_ViewOrigin2;
	Vector m_ViewAngles2;
	Vector m_LocalViewAngles2;
};