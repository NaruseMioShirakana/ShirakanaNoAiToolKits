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
	auto img = imgvideo::ImageSlicer(_path, s_width, s_height, tile_pad, 0.0, false);

	const auto s_len = s_width * s_height;
	auto& imgRGB = img.data.rgb;
	auto& imgAlpha = img.data.alpha;
	const size_t progressMax = imgAlpha.size() / s_len;
	const size_t progressX = img.shape[2];
	const size_t progressY = progressMax / progressX;
	const size_t XLen = progressX * s_width;
	const size_t YLen = progressY * s_width;

	std::vector<float> _imgRGB;
	_imgRGB.reserve(imgRGB.size());
	std::vector<float> _imgALPHA;
	_imgALPHA.reserve(imgAlpha.size());

	const auto rgbData = imgRGB.data();
	const auto AlphaData = imgAlpha.data();

	long class_num = 3;
	for (size_t j = 0; j < progressY; ++j)
		for (size_t i = 0; i < progressX; ++i)
		{
			const size_t x_beg = i * s_width * class_num;
			const size_t y_beg = j * s_height;
			const size_t x_end = (i + 1) * s_width * class_num;
			const size_t y_end = (j + 1) * s_height;
			for (size_t y = y_beg; y < y_end; ++y)
			{
				_imgRGB.insert(_imgRGB.end(), rgbData + (XLen * class_num * y + x_beg), rgbData + (XLen * class_num * y + x_end));
			}
		}

	class_num = 1;
	for (size_t j = 0; j < progressY; ++j)
		for (size_t i = 0; i < progressX; ++i)
		{
			const size_t x_beg = i * s_width * class_num;
			const size_t y_beg = j * s_height;
			const size_t x_end = (i + 1) * s_width * class_num;
			const size_t y_end = (j + 1) * s_height;
			for (size_t y = y_beg; y < y_end; ++y)
			{
				_imgALPHA.insert(_imgALPHA.end(), AlphaData + (XLen * class_num * y + x_beg), AlphaData + (XLen * class_num * y + x_end));
			}
		}

	_callback(progress, progressMax);

	std::vector<std::vector<float>> _imgOutRGB, _imgOutAlpha;
	_imgOutRGB.reserve(progressMax); _imgOutAlpha.reserve(progressMax);

	for (int64_t i = 0; i < int64_t(progressMax);)
	{
		if (progress + batch_size > progressMax)
			batch_size = int64_t(progressMax - progress);
		if (batch_size == 0)
			break;

		int64_t shape[4] = { batch_size,s_height,s_width,3 };
		std::vector<float> ImageI(_imgRGB.data() + (s_len * 3ll * progress), _imgRGB.data() + (s_len * 3ll * (progress + batch_size)));
		std::vector<Ort::Value> Tensors, outTensors;
		Tensors.emplace_back(Ort::Value::CreateTensor(*memory_info, ImageI.data(), ImageI.size(), shape, 4));
		try
		{
			outTensors = model->Run(Ort::RunOptions{ nullptr },
				inputNames.data(),
				Tensors.data(),
				Tensors.size(),
				outputNames.data(),
				outputNames.size()
			);
		}
		catch (Ort::Exception& e)
		{
			throw std::exception(e.what());
		}
		const auto outShape = outTensors[0].GetTensorTypeAndShapeInfo().GetShape();
		const auto outData = outTensors[0].GetTensorData<float>();

		for (int64_t j = 0; j < batch_size; j++)
			_imgOutRGB.emplace_back(outData + j * outShape[1] * outShape[2] * 3, outData + (j + 1) * outShape[1] * outShape[2] * 3);

		shape[3] = 1;
		Tensors.clear();
		ImageI = std::vector<float>(_imgALPHA.data() + (s_len * progress), _imgALPHA.data() + (s_len * (progress + batch_size)));

		Tensors.emplace_back(Ort::Value::CreateTensor(*memory_info, ImageI.data(), ImageI.size(), shape, 4));

		try
		{
			outTensors = model_alpha->Run(Ort::RunOptions{ nullptr },
				inputNames.data(),
				Tensors.data(),
				Tensors.size(),
				outputNames.data(),
				outputNames.size()
			);
		}
		catch (Ort::Exception& e)
		{
			throw std::exception(e.what());
		}
		const auto outShapeAlpha = outTensors[0].GetTensorTypeAndShapeInfo().GetShape();
		const auto outDataAlpha = outTensors[0].GetTensorData<float>();

		for (int64_t j = 0; j < batch_size; j++)
			_imgOutAlpha.emplace_back(outDataAlpha + j * outShapeAlpha[1] * outShapeAlpha[2], outDataAlpha + (j + 1) * outShapeAlpha[1] * outShapeAlpha[2]);

		progress += batch_size;
		i += batch_size;
		_callback(progress, progressMax);
	}
	imgRGB.clear(); imgAlpha.clear();
	imgRGB.reserve(3ll * s_width * s_height * scale * scale * progressMax);
	imgAlpha.reserve(1ll * s_width * s_height * scale * scale * progressMax);

	for (size_t j = 0; j < progressY; ++j)
	{
		for(size_t y = 0; y < 1ull * s_height * scale; ++y)
			for (size_t i = 0; i < progressX; ++i)
			{
				const auto pos = j * progressX + i;
				imgRGB.insert(imgRGB.end(), _imgOutRGB[pos].data() + y * 3ull * s_width * scale, _imgOutRGB[pos].data() + (y + 1) * 3ull * s_width * scale);
				imgAlpha.insert(imgAlpha.end(), _imgOutAlpha[pos].data() + y * 1ull * s_width * scale, _imgOutAlpha[pos].data() + (y + 1) * 1ull * s_width * scale);
			}
	}

	if (!img.MergeWrite((_outputPath + _path.substr(_path.rfind(L'\\'), _path.rfind(L'.')) + std::to_wstring(unsigned long long(_path.data())) + L".png").c_str(), 1))
		throw std::exception("error when write image");

	/*
	for (int64_t i = 0; i < int64_t(s_width);)
	{
		if (progress + batch_size > progressMax)
			batch_size = int64_t(progressMax - progress);
		if (batch_size == 0)
			break;
		std::vector<float> rgb, alpha;
		rgb.reserve(s_height * int64_t(s_width) * 3);
		alpha.reserve(s_height * int64_t(s_width));
		int64_t shape[4] = { batch_size,s_height,s_width,3 };
		std::vector<float> ImageI;
		ImageI.reserve(int64_t(s_height) * s_width * 3ll * batch_size);

		for (int64_t j = 0; j < s_height; j++)
			ImageI.insert(ImageI.end(), imgRGB[j + i].begin(), imgRGB[j + i].end());

		std::vector<Ort::Value> Tensors;
		
		
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
		}catch (Ort::Exception& e)
		{
			throw std::exception(e.what());
		}
	}
	img.write((_outputPath + _path.substr(_path.rfind(L'\\'), _path.rfind(L'.')) + std::to_wstring(unsigned long long(_path.data())) + L".mid").c_str(), scale);
	
	 */
	
}

INFERCLASSEND