/*
* file: ImgVideo.hpp
* info: ͼƬ������Ƭ��ʵ��
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
	//[0] - width [1] - height [2] - ����Ƭ��
	int shape[3] = { 0,0,4 };
	struct Data
	{
		std::vector<float> rgb;
		std::vector<float> alpha;
	} data;

	ImageSlicer() = delete; //��ֹĬ�Ϲ���
	~ImageSlicer() = default;

	/*ͼ����Ƭ��
	* @param input - �����ļ�·��
	* @param width - ��Ƭ���
	* @param height - ��Ƭ�߶�
	* @param len - offset
	* @param pad - ���
	* @param line - ��ʾ��Ƭ������
	*
	* @return �������ʧ�ܽ��׳��쳣 ������������ �ļ��𻵡������ڡ����������ڴ治��
	*/
	ImageSlicer(const std::wstring&, int, int, int, float pad = 0.f, bool line = true);

	/*�ϲ���Ƭ���ݲ�д��
	* @param path - ����·��
	* @param scale - ���ű���
	* @param quality - ͼƬѹ������ (0-100) 100Ϊ���
	*
	* @return �쳣: std::bad_alloc,std::runtime_error
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