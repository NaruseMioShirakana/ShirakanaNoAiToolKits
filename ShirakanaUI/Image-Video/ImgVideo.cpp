#include "ImgVideo.hpp"

IMAGEVIDEOCLASSHEADER

// 构造函数，从image读取长宽，存储到shape数组的一二维，然后将RGB数据按照 h*w 的大小切片，按照RGB的顺序存储到_RGB中，如果不够h或w就补为pad，Alpha同理
Image::Image(const std::wstring& path, const long h, const long w, const float pad)
{
	
}

// 使用压缩比较高的格式保存图像
bool Image::write(std::wstring& _path)
{
	
}


IMAGEVIDEOCLASSEND