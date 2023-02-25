#pragma once
#include <functional>
#include <onnxruntime_cxx_api.h>
#define INFERCLASSHEADER namespace InferClass{
#define INFERCLASSEND }

INFERCLASSHEADER
class BaseModelType
{
public:
	using callback = std::function<void(size_t, size_t)>;
	using int64 = int64_t;
	using MTensor = Ort::Value;
	BaseModelType();
protected:
	~BaseModelType();
	Ort::Env* env = nullptr;
	Ort::SessionOptions* session_options = nullptr;
	Ort::MemoryInfo* memory_info = nullptr;
};
INFERCLASSEND