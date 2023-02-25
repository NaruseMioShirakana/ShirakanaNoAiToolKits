/*
* file: MainWindow.h
* info: 应用程序主窗口定义 使用MiaoUI
*
* Author: Maplespe(mapleshr@icloud.com)
* 
* date: 2022-9-19 Create
*/
#pragma once
#include "..\framework.h"
#include "../Lib/rapidjson/document.h"
#include "../Inference/inferObject.hpp"

namespace AiUI
{
	constexpr size_t MaxPath = 32000;

	using namespace Mui;

	//主窗口尺寸
	extern const UISize m_wndSize;
	//主窗口标题
	extern std::wstring m_wndTitle;

	//初始化and反初始化
	extern bool MiaoUI_Initialize(std::wstring& error);
	extern void MiaoUI_Uninitialize();

	//应用程序主窗口 WIndows平台
	class MainWindow : public Window::UIWindowsWnd
	{
	public:
		enum class FileType
		{
			Audio,
			Image
		};

		MainWindow(MRender* _render_) : UIWindowsWnd(_render_) {}
		~MainWindow() override;
		//界面事件回调
		virtual bool EventProc(UINotifyEvent, UIControl*, _m_param) override;
		//窗口事件源 WIndows窗口事件回调
		virtual _m_result EventSource(_m_param, _m_param) override;
		HWND hWnd = nullptr;

		UIListBox* progress_list = nullptr;
		IconButton* button_launch = nullptr;
		IconButton* button_plugin = nullptr;
		IconButton* button_add1 = nullptr;
		IconButton* button_add2 = nullptr;
		IconButton* modlist_refresh = nullptr;
		IconButton* modlist_folder = nullptr;
		UIComBox* model_list = nullptr;
		UIImgBox* background = nullptr;
		UIProgressBar* process = nullptr;

		InferClass::BaseModelType::callback InferCallback = [&](size_t _cur, size_t _max) -> void
		{
			if (!_cur)
				process->SetMaxValue(_m_uint(_max));
			process->SetCurValue(_m_uint(_cur));
		};

		void loadModels();

		void loadModel(int64_t);

		void Infer() const;

		void SetEnable(bool) const;
	private:
		FileType file_t = FileType::Audio;
		UIEditBox* batch_size = nullptr;
		std::vector<rapidjson::Document> _modelConfigs;
		bool AfterCreated();
		bool CreateControls(UIControl* root);
		MHelper::MuiXML* m_uiXML = nullptr;
		MFontLoader* m_fontLoader = nullptr;
		friend void MainWindow_CreateLoop();
		ModelInfer InferInstance;
	};

	//创建主窗口
	extern void MainWindow_CreateLoop();
}