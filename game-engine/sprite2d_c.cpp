#include "stdafx.h"
#include "sprite2d_c.h"

sprite2d_c::sprite2d_c()
{
}


sprite2d_c::~sprite2d_c()
{
	if (m_source_dib)
		FreeImage_Unload(m_source_dib);
}


bool sprite2d_c::load(const std::string& path, FREE_IMAGE_FORMAT format)
{
	m_source_dib = FreeImage_Load(format, path.c_str(), PNG_DEFAULT);

	return m_source_dib ? true : false;
}


void sprite2d_c::rotate(int degrees)
{
	m_source_dib = FreeImage_Rotate(m_source_dib, degrees);
}


FIBITMAP* sprite2d_c::get_fibitmap() const
{
	return m_source_dib;
}
