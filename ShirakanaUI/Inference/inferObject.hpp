#pragma once
#include "../MusicTranscription/PianoTranscription.hpp"
#include "../ImageUpScale/ImageUpScale.hpp"
namespace AiUI
{
	class ModelInfer
	{
	public:
		ModelInfer() = default;

		~ModelInfer() { release(); }

		void release()
		{
			if (piano_tran_scription)
			{
				delete piano_tran_scription;
				piano_tran_scription = nullptr;
			}
			if (real_esr_gan)
			{
				delete real_esr_gan;
				real_esr_gan = nullptr;
			}
		}

		void infer(const std::wstring& _path, int64_t batch_size) const
		{
			if (piano_tran_scription)
				piano_tran_scription->Infer(_path, batch_size);
			else if (real_esr_gan)
				real_esr_gan->Infer(_path, batch_size);
			else
				throw std::exception("Model Empty!!!");
		}

		void load(const rapidjson::Document& _config, const InferClass::BaseModelType::callback& _cb, FileType& file_t, Mui::MRender* _render = nullptr)
		{
			if (std::string(_config["type"].GetString()) == "PianoTranscription")
			{
				piano_tran_scription = new InferClass::PianoTranScription(_config, _cb);
				file_t = FileType::Audio;
			}
			else if (std::string(_config["type"].GetString()) == "ESRGan")
			{
				real_esr_gan = new InferClass::RealESRGan(_config, _cb, _render);
				file_t = FileType::Image;
			}
			else
				throw std::exception("Config Error!!!");
		}
	private:
		InferClass::PianoTranScription* piano_tran_scription = nullptr;
		InferClass::RealESRGan* real_esr_gan = nullptr;
	};
}

