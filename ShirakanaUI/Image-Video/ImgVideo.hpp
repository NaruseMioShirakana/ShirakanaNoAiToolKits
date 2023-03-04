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
	size_t size() const
	{
		return _data.size();
	}
	size_t numpix() const
	{
		return shape[0] * shape[1];
	}
	size_t width() const
	{
		return shape[1];
	}
	size_t height() const
	{
		return shape[0];
	}
	Image() = delete;
	Image(const std::wstring&, const long, const long, const float = 0.0);
	bool write(std::wstring&);
	~Image() = default;
	//static Image cut(size_t, size_t, const Image&);
	//Image& cut(size_t, size_t);
	// Image& pad(int64_t, int64_t, float value = 0, const std::string& mode = "constant");
	// Image& pad(int64_t, int64_t, int64_t, int64_t, float value = 0, const std::string& mode = "constant");
	float* data()
	{
		return _data.data();
	}
	const float* data() const
	{
		return _data.data();
	}
private:
	std::vector<std::vector<float>> _RGB;
	std::vector<std::vector<float>> _Alpha;
};

IMAGEVIDEOCLASSEND