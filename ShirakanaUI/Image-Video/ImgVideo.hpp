#pragma once
#define IMAGEVIDEOCLASSHEADER namespace imgvideo{
#define IMAGEVIDEOCLASSEND }
#include <string>
#include <vector>
#include "../Helper/Helper.h"

IMAGEVIDEOCLASSHEADER

class Image
{
public:
	size_t shape[3] = { 0,0,4 };
	std::vector<std::vector<float>> _RGB;
	std::vector<std::vector<float>> _Alpha;

	Image() = delete; //½ûÖ¹Ä¬ÈÏ¹¹Ôì
	Image(const std::wstring&, const long, const long, const float = 0.0);
	bool write(std::wstring&);
	~Image() = default;
};

IMAGEVIDEOCLASSEND