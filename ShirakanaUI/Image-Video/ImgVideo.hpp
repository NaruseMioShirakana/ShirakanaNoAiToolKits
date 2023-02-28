#pragma once
#define IMAGEVIDEOCLASSHEADER namespace imgvideo{
#define IMAGEVIDEOCLASSEND }
#include <string>
#include <vector>

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
		return shape[0];
	}
	size_t height() const
	{
		return shape[1];
	}
	Image() = default;
	Image(const std::wstring&);
	Image(size_t, size_t, const float*);
	Image(const Image& _img)
	{
		_data = _img._data;
		shape[0] = _img.shape[0];
		shape[1] = _img.shape[1];
	}
	Image(Image&& _img) noexcept
	{
		_data = std::move(_img._data);
		shape[0] = _img.shape[0];
		shape[1] = _img.shape[1];
	}
	Image& operator=(const Image& _img)
	{
		if (&_img == this) 
			return*this;
		_data = _img._data;
		shape[0] = _img.shape[0];
		shape[1] = _img.shape[1];
		return*this;
	}
	Image& operator=(Image&& _img) noexcept {
		if (&_img == this)
		return*this;
		_data = std::move(_img._data);
		shape[0] = _img.shape[0];
		shape[1] = _img.shape[1];
		return*this;
	}
	static Image cut(size_t, size_t, const Image&);
	Image& cut(size_t, size_t);
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
	std::vector<float> _data;
};

IMAGEVIDEOCLASSEND