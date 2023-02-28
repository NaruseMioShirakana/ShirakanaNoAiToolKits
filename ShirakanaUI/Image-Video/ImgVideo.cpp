#include "ImgVideo.hpp"

IMAGEVIDEOCLASSHEADER

Image::Image(const std::wstring& path)
{
	
}

Image::Image(size_t w, size_t h, const float* data_)
{
	shape[0] = w;
	shape[1] = h;
	_data = { data_ ,data_ + w * h * 4 };
}

Image Image::cut(size_t w, size_t h, const Image& _img)
{
	return { w,h,_img.data() };
}

Image& Image::cut(size_t w, size_t h)
{
	return *this = { w,h,_data.data() };
}

IMAGEVIDEOCLASSEND