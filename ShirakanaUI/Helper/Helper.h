/*
* file: Helper.h
* info: Ӧ�ó���WINAPI����
*
* Author: Maplespe(mapleshr@icloud.com)
*
* date: 2022-9-19 Create
*/
#pragma once
#include "..\framework.h"


//��ȡ��ǰӦ�ó�������Ŀ¼
extern std::wstring GetCurrentFolder();

//��ȡ��ǰ����������ʾ��DPI
extern bool GetWNdMonitorDPI(HWND hWNd, UINT& dpiX, UINT& dpiY);