#pragma once
#include <functional>
#include <onnxruntime_cxx_api.h>
#include "../Helper/Helper.h"
#define INFERCLASSHEADER namespace InferClass{
#define INFERCLASSEND }

#define CUDAMOESS 1

namespace AiUI
{
	enum class FileType
	{
		Audio,
		Image,
		Video,
		ImageVideo
	};
}

INFERCLASSHEADER
class BaseModelType
{
public:
	using callback = std::function<void(size_t, size_t)>;
	using int64 = int64_t;
	using MTensor = Ort::Value;
	virtual void Infer(const std::wstring& _path, int64_t batch_size = 1) const = 0;
	BaseModelType();
protected:
	~BaseModelType();
	Ort::Env* env = nullptr;
	Ort::SessionOptions* session_options = nullptr;
	Ort::MemoryInfo* memory_info = nullptr;
	std::wstring _outputPath = GetCurrentFolder() + L"\\outputs";
};
INFERCLASSEND