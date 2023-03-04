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
	Image(const std::wstring&, long, long, long, float = 0.0);
	bool write(const std::wstring&, long);
	~Image() = default;
};

IMAGEVIDEOCLASSEND