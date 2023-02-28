#include "ImageUpScale.hpp"
#include "../StringPreprocess.hpp"

INFERCLASSHEADER

RealESRGan::RealESRGan(const rapidjson::Document& _config, const callback& _cb)
{
	const std::wstring _path = GetCurrentFolder() + L"\\Models\\" + to_wide_string(_config["folder"].GetString()) + L"\\";
	if (!_config["scale"].IsNull())
		scale = _config["scale"].GetInt();
	if (!_config["tile_size"].IsNull())
		tile_size = _config["tile_size"].GetInt();
	if (!_config["tile_pad"].IsNull())
		tile_pad = _config["tile_pad"].GetInt();
	if (!_config["pre_pad"].IsNull())
		pre_pad = _config["pre_pad"].GetInt();
	if (!_config["mod_scale"].IsNull())
		mod_scale = _config["mod_scale"].GetInt();
	else
		mod_scale = -1;
	if (scale == 2)
		mod_scale = 2;
	if (scale == 1)
		mod_scale = 4;
	try
	{
		model = new Ort::Session(*env, (_path + L"model.onnx").c_str(), *session_options);
	}
	catch (Ort::Exception& e)
	{
		throw std::exception(e.what());
	}
	_callback = _cb;
}

RealESRGan::~RealESRGan()
{
	delete model;
	model = nullptr;
}

void RealESRGan::Infer(const std::wstring& _path, int64_t batch_size) const
{
	size_t progressMax = 0;
	size_t progress = 0;
	_callback(progress, progressMax);
}

std::vector<float> RealESRGan::pre_process(std::vector<float>& image)
{
	if (mod_scale != -1)
	{
		mod_pad_h = mod_pad_w = 0;

	}
}

INFERCLASSEND