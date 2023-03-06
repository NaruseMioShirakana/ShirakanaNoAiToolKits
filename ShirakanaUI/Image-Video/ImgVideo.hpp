﻿/*
* file: ImgVideo.hpp
* info: 图片数据切片类实现
*
* Author: Maplespe(mapleshr@icloud.com)
*
* date: 2023-3-4 Create.
*/
#pragma once
#define IMAGEVIDEOCLASSHEADER namespace imgvideo {
#define IMAGEVIDEOCLASSEND }
#include <string>
#include <vector>
#include "../Helper/Helper.h"

IMAGEVIDEOCLASSHEADER

class ImageSlicer
{
public:
	//[0] - width [1] - height [2] - 行切片数
	int shape[3] = { 0,0,4 };
	struct Data
	{
		std::vector<float> rgb;
		std::vector<float> alpha;
	} data;

	ImageSlicer() = delete; //禁止默认构造
	~ImageSlicer() = default;

	/*图像切片器
	* @param input - 输入文件路径
	* @param width - 切片宽度
	* @param height - 切片高度
	* @param len - offset
	* @param pad - 填充
	* @param line - 显示切片网格线
	*
	* @return 如果构造失败将抛出异常 包括但不限于 文件损坏、不存在、参数错误、内存不足
	*/
	ImageSlicer(const std::wstring&, int, int, int, float pad = 0.f, bool line = true);

	/*合并切片数据并写出
	* @param path - 保存路径
	* @param scale - 缩放倍数
	* @param quality - 图片压缩质量 (0-100) 100为最好
	*
	* @return 异常: std::bad_alloc,std::runtime_error
	*/
	bool MergeWrite(std::wstring path, int scale, UINT quality = 100);

private:
	//原切片尺寸信息
	struct ClipData
	{
		std::pair<int, int> clipSize; //已切片canvas尺寸
		std::pair<int, int> blockSize;//切片尺寸
		int clipLength = 0;			  //切片offset长度
	} m_clip;
};

IMAGEVIDEOCLASSEND