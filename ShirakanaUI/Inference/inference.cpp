#include "inference.hpp"
#include <thread>

InferClass::BaseModelType::BaseModelType()
{
#ifdef CUDAMOESS
	const auto AvailableProviders = Ort::GetAvailableProviders();
	bool ret = true;
	for (const auto& it : AvailableProviders)
		if (it.find("CUDA") != std::string::npos)
			ret = false;
	if (ret)
		throw std::exception("CUDA Provider Not Found");
	OrtCUDAProviderOptions cuda_option;
	cuda_option.device_id = 0;
#endif
#ifdef DMLMOESS
	const auto AvailableProviders = Ort::GetAvailableProviders();
	std::string ret;
	for (const auto& it : AvailableProviders)
		if (it.find("Dml") != std::string::npos)
			ret = it;
	if (ret.empty())
		throw std::exception("DML Provider Not Found");
	const OrtApi& ortApi = Ort::GetApi();
	const OrtDmlApi* ortDmlApi = nullptr;
	ortApi.GetExecutionProviderApi("DML", ORT_API_VERSION, reinterpret_cast<const void**>(&ortDmlApi));
#endif
	session_options = new Ort::SessionOptions;
#ifdef CUDAMOESS
	env = new Ort::Env(ORT_LOGGING_LEVEL_WARNING, "OnnxModel");
	session_options->AppendExecutionProvider_CUDA(cuda_option);
	session_options->SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_BASIC);
	session_options->SetIntraOpNumThreads(1);
	memory_info = new Ort::MemoryInfo(Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault));
#else
#ifdef DMLMOESS
	ThreadingOptions threadingOptions;
	env = new Ort::Env(threadingOptions.threadingOptions, LoggingFunction, nullptr, ORT_LOGGING_LEVEL_VERBOSE, "");
	env->DisableTelemetryEvents();
	ortDmlApi->SessionOptionsAppendExecutionProvider_DML(*session_options, 0);
	session_options->SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_BASIC);
	session_options->DisablePerSessionThreads();
	session_options->SetExecutionMode(ExecutionMode::ORT_SEQUENTIAL);
	session_options->DisableMemPattern();
	memory_info = new Ort::MemoryInfo(Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtDeviceAllocator, OrtMemType::OrtMemTypeCPU));
#else
	env = new Ort::Env(ORT_LOGGING_LEVEL_WARNING, "OnnxModel");
	session_options->SetIntraOpNumThreads(static_cast<int>(std::thread::hardware_concurrency()));
	session_options->SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
	memory_info = new Ort::MemoryInfo(Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault));
#endif
#endif
}

InferClass::BaseModelType::~BaseModelType()
{
	delete session_options;
	delete env;
	delete memory_info;
	env = nullptr;
	session_options = nullptr;
	memory_info = nullptr;
}
