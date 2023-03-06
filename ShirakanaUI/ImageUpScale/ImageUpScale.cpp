#include "ImageUpScale.hpp"
#include "../StringPreprocess.hpp"
#include "../Image-Video/ImgVideo.hpp"

INFERCLASSHEADER

RealESRGan::RealESRGan(const rapidjson::Document& _config, const callback& _cb, Mui::MRender* _render_ptr)
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
		model_alpha = new Ort::Session(*env, (_path + L"model_alpha.onnx").c_str(), *session_options);
	}
	catch (Ort::Exception& e)
	{
		throw std::exception(e.what());
	}
	_callback = _cb;
	_render = _render_ptr;
}

RealESRGan::~RealESRGan()
{
	delete model;
	delete model_alpha;
	model_alpha = nullptr;
	model = nullptr;
}

void RealESRGan::Infer(const std::wstring& _path, int64_t batch_size) const
{
	size_t progress = 0;
	auto img = imgvideo::Image(_path, 64, 64, tile_pad, 0.0);
	const size_t progressMax = img._RGB.size();
	auto& imgRGB = img._RGB;
	auto& imgAlpha = img._Alpha;
	_callback(progress, progressMax);
	for (int64_t i = 0; i < int64_t(progressMax);)
	{
		if (progress + batch_size > progressMax)
			batch_size = int64_t(progressMax - progress);
		if (batch_size == 0)
			break;
		int64_t shape[4] = { batch_size,64,64,3 };
		std::vector<Ort::Value> Tensors;
		std::vector<float> ImageI;
		ImageI.reserve(64ll * 64ll * 3ll * batch_size);
		for (int64_t j = 0; j < batch_size; j++)
			ImageI.insert(ImageI.end(), imgRGB[j + i].begin(), imgRGB[j + i].end());
		Tensors.emplace_back(Ort::Value::CreateTensor(*memory_info, ImageI.data(), ImageI.size(), shape, 4));
		try
		{
			const auto outTensors = model->Run(Ort::RunOptions{ nullptr },
				inputNames.data(),
				Tensors.data(),
				Tensors.size(),
				outputNames.data(),
				outputNames.size()
			);
			const auto outShape = outTensors[0].GetTensorTypeAndShapeInfo().GetShape();
			const auto outData = outTensors[0].GetTensorData<float>();
			for (int64_t j = 0; j < batch_size; j++)
				imgRGB[j + i] = { outData + j * outShape[1] * outShape[2] * 3,outData + (j + 1) * outShape[1] * outShape[2] * 3 };

			shape[3] = 1;
			Tensors.clear();
			ImageI.clear();
			ImageI.reserve(64ll * 64ll * 1ll * batch_size);
			for (int64_t j = 0; j < batch_size; j++)
				ImageI.insert(ImageI.end(), imgAlpha[j + i].begin(), imgAlpha[j + i].end());
			Tensors.emplace_back(Ort::Value::CreateTensor(*memory_info, ImageI.data(), ImageI.size(), shape, 4));

			const auto outTensorsAlpha = model->Run(Ort::RunOptions{ nullptr },
				inputNames.data(),
				Tensors.data(),
				Tensors.size(),
				outputNames.data(),
				outputNames.size()
			);
			const auto outShapeAlpha = outTensors[0].GetTensorTypeAndShapeInfo().GetShape();
			const auto outDataAlpha = outTensors[0].GetTensorData<float>();
			for (int64_t j = 0; j < batch_size; j++)
				imgAlpha[j + i] = { outDataAlpha + j * outShapeAlpha[1] * outShapeAlpha[2] * 3,outDataAlpha + (j + 1) * outShapeAlpha[1] * outShapeAlpha[2] * 3 };

			progress += batch_size;
			i += batch_size;
			_callback(progress, progressMax);
		}catch (Ort::Exception e)
		{
			throw std::exception(e.what());
		}
	}
	img.write((_outputPath + _path.substr(_path.rfind(L'\\'), _path.rfind(L'.')) + std::to_wstring(unsigned long long(_path.data())) + L".mid").c_str(), scale);
}

INFERCLASSEND