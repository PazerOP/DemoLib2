#include "DemoViewpoint.hpp"
#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"

#define READ_VEC(vec, reader)   \
  vec.x = reader.ReadInline<float>(); \
  vec.y = reader.ReadInline<float>(); \
  vec.z = reader.ReadInline<float>();

#define WRITE_VEC(vec, writer)  \
  writer.Write(vec.x);          \
  writer.Write(vec.y);          \
  writer.Write(vec.z);

void DemoViewpoint::ReadElementInternal(BitIOReader& reader)
{
  reader.Read(m_Flags);

  READ_VEC(m_ViewOrigin1, reader);
  READ_VEC(m_ViewAngles1, reader);
  READ_VEC(m_LocalViewAngles1, reader);

  READ_VEC(m_ViewOrigin2, reader);
  READ_VEC(m_ViewAngles2, reader);
  READ_VEC(m_LocalViewAngles2, reader);
}

void DemoViewpoint::WriteElementInternal(BitIOWriter& writer) const
{
  writer.Write(m_Flags);

  WRITE_VEC(m_ViewOrigin1, writer);
  WRITE_VEC(m_ViewAngles1, writer);
  WRITE_VEC(m_LocalViewAngles1, writer);

  WRITE_VEC(m_ViewOrigin2, writer);
  WRITE_VEC(m_ViewAngles2, writer);
  WRITE_VEC(m_LocalViewAngles2, writer);
}