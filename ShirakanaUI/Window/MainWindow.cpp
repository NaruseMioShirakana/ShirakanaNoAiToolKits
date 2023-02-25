/*
* file: MainWindow.cpp
* info: 应用程序主窗口实现 使用MiaoUI
*
* Author: Maplespe(mapleshr@icloud.com)
*
* date: 2022-9-19 Create.
*/

#include "MainWindow.h"

#include <commdlg.h>
#include "../resource.h"
#include "..\Helper\Helper.h"
#include <Uxtheme.h>
#include <fstream>
#include "../StringPreprocess.hpp"
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "uxtheme.lib")

#define _MNAME(x) (control->GetName() == (x))

namespace AiUI
{
	using namespace Mui;

	//主窗口尺寸
	const UISize m_wndSize = { 768, 480 };
	//主窗口标题
	std::wstring m_wndTitle = L"白叶的AI工具箱";

	//界面资源
	DMResources* m_uiRes = nullptr;
	auto constexpr m_resKey = L"12345678";

	bool MiaoUI_Initialize(std::wstring& error)
	{
		//注册MiaoUI系统控件
		CtrlMgr::RegisterMuiControl();
		//注册自定义控件
		IconButton::Register();

		//初始化渲染器
		if (!InitDirect2D(&error, -1))
			return false;
		//加载资源文件
		const std::wstring cfgpath = GetCurrentFolder() + L"\\ShirakanaUI.dmres";
		m_uiRes = new DMResources();
		if (!m_uiRes->LoadResource(cfgpath, false))
		{
			error = L"加载文件失败：" + cfgpath;
			return false;
		}

		return true;
	}

	void MiaoUI_Uninitialize()
	{
		if (m_uiRes) {
			m_uiRes->CloseResource();
			delete m_uiRes;
		}
		UninitDirect2D();
	}

	void MainWindow_CreateLoop()
	{
		MainWindow window(new MRender_D2D());
		auto func = std::bind(&MainWindow::AfterCreated, &window);
		if (window.Create(0, m_wndTitle, UIRect(0, 0, m_wndSize.width, m_wndSize.height), func, WS_CAPTION | WS_MINIMIZEBOX, 0))
		{
			window.SetMainWindow(true);
			//DPI缩放
			UINT dpiX, dpiY = 0;
			if (GetWNdMonitorDPI((HWND)window.GetWindowHandle(), dpiX, dpiY))
			{
				const float scaleX = (float)dpiX / 96.f;
				const float scaleY = (float)dpiY / 96.f;
				//scaleX = scaleY = 1.5f;
				window.ScaleWindow(int(float(m_wndSize.width) * scaleX), int(float(m_wndSize.height) * scaleY));
			}
			
			//垂直同步
			window.SetVerticalSync(true);
			//开启内存资源缓存
			window.SetResMode(true);

			//居中和显示窗口
			window.CenterWindow();
			window.ShowWindow(true);

			//ShowDebugRect = true;

			//窗口消息循环
			UIMessageLoop();
		}
	}

	MainWindow::~MainWindow()
	{
		delete m_uiXML;
		delete m_fontLoader;
	}

	bool MainWindow::EventProc(UINotifyEvent event, UIControl* control, _m_param param)
	{
		if(!hWnd)
			hWnd = (HWND)GetWindowHandle();

		switch (event)
		{
		case Event_Mouse_LClick:
			{
				if (_MNAME(L"title_close")) {
					::SendMessageW(hWnd, M_WND_CLOSE, 0, 0);
					return true;
				}
				if (_MNAME(L"title_minisize")) {
					::ShowWindow(hWnd, SW_MINIMIZE);
					return true;
				}
				if(_MNAME(L"modlist_folder"))
				{
					STARTUPINFO si; PROCESS_INFORMATION pi;
					ZeroMemory(&si, sizeof(STARTUPINFO)); ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
					si.cb = sizeof(STARTUPINFO);
					CreateProcess(nullptr,
						const_cast<LPWSTR>((L"Explorer.exe " + GetCurrentFolder() + L"\\Models").c_str()),
						nullptr, nullptr, TRUE, CREATE_NO_WINDOW,
						nullptr, nullptr, &si, &pi);
					CloseHandle(pi.hProcess); CloseHandle(pi.hThread);
					return true;
				}
				if(_MNAME(L"modlist_refresh"))
				{
					loadModels();
					return true;
				}
				if (_MNAME(L"button_launch"))
				{
					std::thread inferThread([&]() {Infer(); });
					inferThread.detach();
					return true;
				}
				break;
			}
		case Event_Mouse_LDown:
			{
				if (_MNAME(L"titlebar")) {
					::SendMessageW(hWnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, 0);
					return true;
				}
				break;
			}
		case Event_ListBox_ItemChanged:
			{
				if(_MNAME(L"model_list"))
				{
					loadModel(int64_t(param));
					return true;
				}
			}
		default:
			return false;
		}
		return false;
	}

	_m_result MainWindow::EventSource(_m_param message, _m_param param)
	{
		switch (message)
		{
			//自绘标题栏 扩展窗口客户区
			case WM_NCCALCSIZE:
			{
				const auto pm = (std::pair<_m_param, _m_param>*)param;

				typedef void (WINAPI* PGetNTVer)(DWORD*, DWORD*, DWORD*);
				static HMODULE hModule = GetModuleHandleW(L"ntdll.dll");
				static auto GetNTVer = (PGetNTVer)GetProcAddress(hModule, "RtlGetNtVersionNumbers");
				DWORD Major = 0;
				GetNTVer(&Major, nullptr, nullptr);

				//win10以下版本删除边框
				if (pm->first && Major < 10)
					return 1;

				if (!pm->second)
					return UIWindowBasic::EventSource(message, param);

				if (!hWnd)
					hWnd = (HWND)GetWindowHandle();

				UINT dpi = 96;
				GetWNdMonitorDPI(hWnd, dpi, dpi);

				const int frameX = GetThemeSysSize(nullptr, SM_CXFRAME);
				const int frameY = GetThemeSysSize(nullptr, SM_CYFRAME);
				const int padding = GetThemeSysSize(nullptr, SM_CXPADDEDBORDER);

				/*if (dpi > 131)
					padding += (dpi - 131) / 24 + 1;

				if (dpi > 143)
				{
					auto add = (dpi - 143) / 96 + 1;
					frameX += add;
					frameY += add;
				}*/
				const auto params = (NCCALCSIZE_PARAMS*)pm->second;
				RECT* rgrc = params->rgrc;

				rgrc->right -= frameX + padding;
				rgrc->left += frameX + padding;
				rgrc->bottom -= frameY + padding;

				WINDOWPLACEMENT placement = { 0 };
				placement.length = sizeof(WINDOWPLACEMENT);
				if (GetWindowPlacement(hWnd, &placement)) {
					if(placement.showCmd == SW_SHOWMAXIMIZED)
						rgrc->top += padding;
				}

				return true;
			}
			//重新计算框架 扩展到整窗
			case WM_CREATE:
			{
				const auto rect = GetWindowRect();
				SetWindowPos((HWND)GetWindowHandle(), nullptr, rect.left, rect.top,
					rect.GetWidth(), rect.GetHeight(), SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
				return UIWindowBasic::EventSource(message, param);
			}
			//限定窗口大小
			case WM_GETMINMAXINFO:
			{
				const auto pm = (std::pair<_m_param, _m_param>*)param;
				const auto p = (MINMAXINFO*)pm->second;
				p->ptMaxSize.x = m_wndSize.width;
				p->ptMaxSize.y = m_wndSize.height;
				return true;
			}
			//响应系统DPI更改
			case WM_DPICHANGED:
			{
				const auto pm = (std::pair<_m_param, _m_param>*)param;
				const UINT dpi = M_LOWORD((_m_long)pm->first);
				const float scale = (float)dpi / 96.f;
				ScaleWindow(int(float(m_wndSize.width) * scale), int(float(m_wndSize.height) * scale));
				return 0;
			}
			//窗口即将关闭
			case WM_CLOSE:
			{
				//先执行界面库内部释放方法
				const auto ret = UIWindowBasic::EventSource(message, param);
				//最后再销毁XML类
				if (m_uiXML) {
					delete m_uiXML;
					m_uiXML = nullptr;
				}
				return ret;
			}
		default:
			return UIWindowBasic::EventSource(message, param);
		}
	}

	bool MainWindow::AfterCreated()
	{
		//窗口必须有一个根控件
		const auto root = new UIControl();
		AddRootControl(root);
		//设置窗口背景色为白色
		UIBkgndStyle bgColor;
		bgColor.background = Color::M_RGBA(255, 255, 255, 255);
		root->SetBackground(bgColor);
		const HWND hWnd = (HWND)GetWindowHandle();
		HICON hIcon = LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_ICON1));
		SendMessageW(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
		SendMessageW(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		//创建界面
		return CreateControls(root);
	}

	bool MainWindow::CreateControls(UIControl* root)
	{
		m_uiXML = new MHelper::MuiXML(this);
		m_uiXML->LoadResource(m_uiRes, m_resKey);
		m_uiXML->LoadSkinList();

		//加载内存字体
		m_fontLoader = new MFontLoader(GetRender());
		UIResource font = m_uiRes->ReadResource(L"font_TSYuMo", m_resKey, DataRes).res;
		m_fontLoader->AddFontResources(font);
		font.Release();

		//导入内存字体样式
		UILabel::Property style;
		style.Font = L"TsangerYuMo";
		style.FontSize = 14;
		style.AutoSize = true;
		style.FontCustom = m_fontLoader->CreateLoader();

		m_uiXML->AddFontStyle(L"style", style);

		style.Font = L"TsangerYuMo";
		style.FontSize = 24;
		style.AutoSize = false;
		style.FontCustom = m_fontLoader->CreateLoader();
		style.Alignment = TextAlign(TextAlign_Center | TextAlign_VCenter);

		m_uiXML->AddFontStyle(L"ListStyle", style);

		//组合框列表字体样式
		style.AutoSize = false;
		style.FontSize = 12;
		style.Font = L"Microsoft YaHei UI";
		style.FontCustom = 0;
		style.Alignment = TextAlign_VCenter;

		m_uiXML->AddFontStyle(L"comList", style);

		const std::wstring xml = LR"(
		<root>
	    <!-- 组合框样式组 -->
	    <PropGroup textAlign="4" skin="skin_combox" skin1="skin_scroll" listSkin="skin_comlist" resIcon="icon_comlist"
	    itemSkin="skin_comlistitem" menuHeight="150" itemFontStyle="comList" barWidth="6" id="comstyle" />

		<PropGroup textAlign="4" skin="skin_comlist" skin1="skin_scroll" listSkin="skin_comlist" resIcon="icon_comlist"
	    itemSkin="skin_comlistitem" menuHeight="150" itemFontStyle="ListStyle" barWidth="6" id="liststyle"/>
	
	    <!-- 模块样式组 -->
	    <PropGroup blur="5" frameWidth="1" frameColor="165,165,165,255" frameRound="15" frameRound="15"
	    bgColor="255,255,255,180" clipRound="15" id="group" />
	
	    <!-- 图标按钮样式组 -->
	    <PropGroup skin="skin_button" fontStyle="style" autoSize="false" textAlign="5" fontSize="12" offset="14,6,5,5"
	    iconSize="16,16" animate="true" id="iconbutton" />

		<PropGroup fontStyle="style" skin="skin_checkbox" animate="true" aniAlphaType="true" textAlign="4" id="checkbox" />

		<UIImgBox align="5" img="img_defaultbg" imgStyle="3" name="background" />

		<UIProgressBar skin="skin_process" frame="0,b_8,=,8" max="100" value="0" name="process" visible="false" />
	
	    <!-- 标题栏 -->
	    <UIBlurLayer frame="0,0,=,30" blur="5" bgColor="255,255,255,200" name="titlebar">
	        <UIButton frame="r_32,0,33,30" skin="skin_btnclose" animate="true" aniAlphaType="true" name="title_close" />
	        <UIButton frame="_32,0,33,30" skin="skin_btnmini" animate="true" aniAlphaType="true" name="title_minisize" />
	        <UILabel pos="8,8" text=")" + m_wndTitle + LR"(" name="title_label" />
	    </UIBlurLayer>
	    <UIControl bgColor="150,150,150,255" frame="0,30,=,1" />

		<UIBlurLayer frame="40,50,286,74" prop="group">
	        <UIImgBox frame="20,27,22,22" img="icon_wave" />
	        <UILabel fontStyle="style" pos="51,29" text="批量大小" />
			<UIEditBox frame="r+40,28,102,20" skin="skin_editbox" name="batch_size" limitText="2" text="1" number="true"/>
	    </UIBlurLayer>

	    <!-- 转换组 -->
	    <UIBlurLayer frame="40,300,286,136" prop="group">
	        <UIImgBox frame="20,20,22,22" img="icon_wave" />
	        <UILabel fontStyle="style" pos="51,22" text="主功能" />
	        <IconButton prop="iconbutton" frame="25,50,95,25" resIcon="icon_begin" text="开始转换" name="button_launch" enable="false"/>
	        <IconButton prop="iconbutton" frame="165,50,95,25" resIcon="icon_plugin" text="启用插件" name="button_plugin" enable="false"/>
			<IconButton prop="iconbutton" frame="25,85,95,25" resIcon="icon_begin" text="预留按钮" name="button_add1" enable="false"/>
	        <IconButton prop="iconbutton" frame="165,85,95,25" resIcon="icon_plugin" text="预留按钮" name="button_add2" enable="false"/>
	    </UIBlurLayer>

		<!-- 模型列表组 -->
	    <UIBlurLayer frame="40,144,286,136" prop="group">
	        <UIImgBox frame="20,20,22,22" img="icon_modlist" />
	        <UILabel fontStyle="style" pos="51,22" text="模型列表" />
	        <UIComBox prop="comstyle" frame="20,50,246,25" text=" 模型选择" name="model_list" />
	        <IconButton prop="iconbutton" frame="20,b+12,117,25" resIcon="icon_refresh" text="刷新模型列表" name="modlist_refresh" />
	        <IconButton prop="iconbutton" frame="r+11,=,=,=" resIcon="icon_folder" text="打开模型目录" name="modlist_folder" />
	    </UIBlurLayer>

		<!-- 输出组 -->
		<UIBlurLayer frame="380,50,350,386" prop="group">
		    <UIImgBox frame="20,20,22,22" img="icon_wave" />
		    <UILabel fontStyle="style" pos="51,22" text="转换列表" />
			<UIListBox frame="25,50,300,320" name="list_box" itemHeight="25" prop="liststyle" frameWidth="1"/>
		</UIBlurLayer>
		</root>
		)";
		if (!m_uiXML->CreateUIFromXML(root, xml))
		{
			throw std::runtime_error("界面创建失败 XML代码有误");
		}
		batch_size = GetRootControl()->Child<UIEditBox>(L"batch_size");
		progress_list = GetRootControl()->Child<UIListBox>(L"list_box");
		modlist_folder = GetRootControl()->Child<IconButton>(L"modlist_folder");
		modlist_refresh = GetRootControl()->Child<IconButton>(L"modlist_refresh");
		button_add2 = GetRootControl()->Child<IconButton>(L"button_add2");
		button_add1 = GetRootControl()->Child<IconButton>(L"button_add1");
		button_launch = GetRootControl()->Child<IconButton>(L"button_launch");
		button_plugin = GetRootControl()->Child<IconButton>(L"button_plugin");
		model_list = GetRootControl()->Child<UIComBox>(L"model_list");
		background = GetRootControl()->Child<UIImgBox>(L"background");
		process = GetRootControl()->Child<UIProgressBar>(L"process");
		loadModels();
		return true;
	}

	void MainWindow::loadModels()
	{
		InferInstance.release();
		std::wstring prefix = L"Plugins";
		if (_waccess(prefix.c_str(), 0) == -1)
			if (_wmkdir(prefix.c_str()))
				fprintf_s(stdout, "[Info] Created Plugins Dir");
		prefix = L"Models";
		if (_waccess(prefix.c_str(), 0) == -1)
			if (_wmkdir(prefix.c_str()))
				fprintf_s(stdout, "[Info] Created Models Dir");
		_wfinddata_t file_info;
		std::wstring current_path = GetCurrentFolder() + L"\\Models";
		intptr_t handle = _wfindfirst((current_path + L"\\*.json").c_str(), &file_info);
		if (-1 == handle)
			return;
		if (model_list->GetCurSelItem() != -1)
			model_list->SetCurSelItem(-1);

		model_list->DeleteAllItem();
		_modelConfigs.clear();

		auto _tmpItem = new ListItem;
		_tmpItem->SetText(L"  None");
		model_list->AddItem(_tmpItem, -1);
		_modelConfigs.emplace_back(rapidjson::Document());
		do
		{
			std::string modInfo, modInfoAll;
			std::ifstream modfile((current_path + L"\\" + file_info.name).c_str());
			while (std::getline(modfile, modInfo))
				modInfoAll += modInfo;
			modfile.close();
			rapidjson::Document modConfigJson;
			modConfigJson.Parse(modInfoAll.c_str());
			if (modConfigJson["folder"].Empty() || modConfigJson["type"].Empty())
				continue;
			_tmpItem = new ListItem;
			auto _tmpText = L"  " + to_wide_string(modConfigJson["folder"].GetString());
			if (_tmpText.size() > 30)
				_tmpText = _tmpText.substr(0, 30) + L"...";
			_tmpItem->SetText(_tmpText);
			model_list->AddItem(_tmpItem, -1);
			_modelConfigs.push_back(std::move(modConfigJson));
		} while (!_wfindnext(handle, &file_info));
	}

	void MainWindow::loadModel(int64_t _index)
	{
		InferInstance.release();
		if (_index == 0)
		{
			button_launch->SetEnabled(false);
			return;
		}
		try
		{
			if (std::string(_modelConfigs[_index]["type"].GetString()) == "PianoTranscription")
			{
				InferInstance.piano_tran_scription = new InferClass::PianoTranScription(_modelConfigs[_index], InferCallback);
				file_t = FileType::Audio;
			}

		}
		catch (std::exception& _exception)
		{
			const std::string exceptionMessage = _exception.what();
			if (exceptionMessage.find("File doesn't exist") != std::wstring::npos)
				MessageBox(hWnd, (L"加载模型失败！模型不存在\n请将模型放置在正确的文件夹下\n"+to_wide_string(_exception.what())).c_str(), L"失败", MB_OK);
			else
				MessageBox(hWnd, (L"加载模型失败！配置文件错误或模型导出错误\n" + to_wide_string(_exception.what())).c_str(), L"失败", MB_OK);
			InferInstance.release();
			model_list->SetCurSelItem(0);
			return;
		}
		button_launch->SetEnabled(true);
	}

	std::vector<std::wstring> InsertMessageToEmptyEditBox(MainWindow::FileType _modelType)
	{
		std::vector<std::wstring> _outPut;
		std::vector<TCHAR> szFileName(MaxPath, 0);
		std::vector<TCHAR> szTitleName(MaxPath, 0);
		OPENFILENAME  ofn;
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lpstrFile = szFileName.data();
		ofn.nMaxFile = MaxPath;
		ofn.lpstrFileTitle = szTitleName.data();
		ofn.nMaxFileTitle = MaxPath;
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = nullptr;
		const auto curFolder = GetCurrentFolder();
		ofn.lpstrInitialDir = curFolder.c_str();
		if (_modelType == MainWindow::FileType::Audio)
		{
			constexpr TCHAR szFilter[] = TEXT("音频 (*.wav;*.mp3;*.ogg;*.flac;*.aac)\0*.wav;*.mp3;*.ogg;*.flac;*.aac\0");
			ofn.lpstrFilter = szFilter;
			ofn.lpstrTitle = L"打开音频";
			ofn.lpstrDefExt = TEXT("wav");
			ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
			if (GetOpenFileName(&ofn))
			{
				auto filePtr = szFileName.data();
				std::wstring preFix = filePtr;
				filePtr += preFix.length() + 1;
				if (!*filePtr)
					_outPut.emplace_back(preFix);
				else
				{
					preFix += L'\\';
					while (*filePtr != 0)
					{
						std::wstring thisPath(filePtr);
						_outPut.emplace_back(preFix + thisPath);
						filePtr += thisPath.length() + 1;
					}
				}
			}
		}
		return _outPut;
	}

	std::wstring GetOutPutPath()
	{
		std::vector<TCHAR> szFileName(MaxPath, 0);
		OPENFILENAME  ofn;
		ZeroMemory(&ofn, sizeof(ofn));
		const auto curFolder = GetCurrentFolder();
		ofn.lpstrFile = szFileName.data();
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrFilter = nullptr;
		ofn.lpstrDefExt = nullptr;
		ofn.lpstrTitle = L"另存为";
		ofn.Flags = OFN_PATHMUSTEXIST;
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = nullptr;
		ofn.lpstrInitialDir = curFolder.c_str();
		if (GetSaveFileName(&ofn))
		{
			return szFileName.data();
		}
		return GetCurrentFolder();
	}

	void InsertListBoxItems(const std::vector<std::wstring>& _folders, UIListBox* list)
	{
		for(const auto& it : _folders)
		{
			const auto _tmpItem = new ListItem;
			_tmpItem->SetText(it.substr(it.rfind('\\') + 1));
			list->AddItem(_tmpItem, -1);
		}
	}

	void MainWindow::Infer() const
	{
		const auto _outputPath = GetCurrentFolder() + L"\\outputs";
		if (_waccess(_outputPath.c_str(), 0) == -1)
			if (_wmkdir(_outputPath.c_str()))
				fprintf_s(stdout, "[Info] Created outputs Dir");
		SetEnable(false);
		const std::vector<std::wstring> _paths = InsertMessageToEmptyEditBox(file_t);
		if(_paths.empty())
		{
			SetEnable(true);
			MessageBox(hWnd, L"请指定输入文件", L"输入为空", MB_OK | MB_ICONINFORMATION);
			return;
		}
		InsertListBoxItems(_paths, progress_list);
		int64_t batchSize = _wtoi64(batch_size->GetCurText().c_str());
		if (batchSize < 1)
			batchSize = 1;
		try
		{
			if (InferInstance.piano_tran_scription)
			{
				for (const auto& it : _paths)
				{
					auto listTextTmp = L"转换中：" + progress_list->GetItem(0)->GetText();
					if (listTextTmp.length() > 30)
						listTextTmp = listTextTmp.substr(0, 30) + L" ...";
					progress_list->GetItem(0)->SetText(listTextTmp);

					const auto OMidi = InferInstance.piano_tran_scription->Infer(it, batchSize);
					OMidi.SaveAs(to_byte_string(_outputPath + it.substr(it.rfind(L'\\'), it.rfind(L'.')) + std::to_wstring(int(it.data())) + L".mid").c_str());

					process->SetCurValue(0);
					progress_list->DeleteItem(0);
				}
			}
		}
		catch(std::exception& _exception)
		{
			MessageBox(hWnd, (L"推理时错误，详细的错误信息如下：\n" + to_wide_string(_exception.what())).c_str(), L"推理失败", MB_OK);
			progress_list->DeleteAllItem();
		}
		SetEnable(true);
	}

	void MainWindow::SetEnable(bool _state) const
	{
		button_launch->SetEnabled(_state);
		modlist_refresh->SetEnabled(_state);
		if(_state && button_plugin->IsEnabled())
			button_plugin->SetEnabled(_state);
		model_list->SetEnabled(_state);
		process->SetVisible(!_state);
		if (_state)
			process->SetCurValue(0);
	}
}