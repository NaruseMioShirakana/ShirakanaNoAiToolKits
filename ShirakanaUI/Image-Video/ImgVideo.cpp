#include "ImgVideo.hpp"

IMAGEVIDEOCLASSHEADER

// 构造函数，从image读取长宽，存储到shape数组的一二维，然后将RGB数据按照 h*w 的大小切片，按照RGB的顺序存储到_RGB中，如果不够h或w就补为pad，Alpha同理（如果不存在Alpha通道则全部设置为0
Image::Image(const std::wstring& path, long height, long width, long len, float pad)
{
	
}

// 使用压缩比较高的格式从_RGB和_Alpha保存图像
bool Image::write(const std::wstring& _path, long len)
{
	
}


IMAGEVIDEOCLASSEND