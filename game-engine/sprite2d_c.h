#pragma once

#include "FreeImage.h"

class sprite2d_c
{
public:
	sprite2d_c();

	~sprite2d_c();

	bool load(const std::string& path, FREE_IMAGE_FORMAT format = FIF_PNG);

	void rotate(int degrees);

	FIBITMAP* get_fibitmap() const;

private:
	FIBITMAP* m_source_dib = nullptr;
};