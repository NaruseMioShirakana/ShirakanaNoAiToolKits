#pragma once
#include "../MusicTranscription/PianoTranscription.hpp"

namespace AiUI
{
	class ModelInfer
	{
	public:
		ModelInfer() = default;

		~ModelInfer() { release(); }

		InferClass::PianoTranScription* piano_tran_scription = nullptr;

		void release()
		{
			if (piano_tran_scription)
			{
				delete piano_tran_scription;
				piano_tran_scription = nullptr;
			}
		}
	};
}

