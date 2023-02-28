#include "PianoTranscription.hpp"
#include <algorithm>

INFERCLASSHEADER

PianoTranScription::PianoTranScription(const rapidjson::Document& _config, const callback& _cb)
{
	const std::wstring _path = GetCurrentFolder() + L"\\Models\\" + to_wide_string(_config["folder"].GetString()) + L"\\";
	if(!_config["sample_rate"].IsNull())
		sample_rate = _config["sample_rate"].GetInt();
	if (!_config["classes_num"].IsNull())
		classes_num = _config["classes_num"].GetInt();
	if (!_config["begin_note"].IsNull())
		begin_note = _config["begin_note"].GetInt();
	if (!_config["segment_seconds"].IsNull())
		segment_seconds = _config["segment_seconds"].GetFloat();
	if (!_config["hop_seconds"].IsNull())
		hop_seconds = _config["hop_seconds"].GetFloat();
	if (!_config["frames_per_second"].IsNull())
		frames_per_second = _config["frames_per_second"].GetFloat();
	if (!_config["velocity_scale"].IsNull())
		velocity_scale = _config["velocity_scale"].GetInt();
	if (!_config["segment_len"].IsNull())
		segment_samples = _config["segment_len"].GetInt();
	if (!_config["onset_threshold"].IsNull())
		onset_threshold = _config["onset_threshold"].GetDouble();
	if (!_config["offset_threshold"].IsNull())
		offset_threshold = _config["offset_threshold"].GetDouble();
	if (!_config["frame_threshold"].IsNull())
		frame_threshold = _config["frame_threshold"].GetDouble();
	if (!_config["use_org_method"].IsNull())
		use_org_method = _config["use_org_method"].GetBool();
	if (!_config["onset_ali"].IsNull())
		onset_ali = _config["onset_ali"].GetInt();
	if (!_config["offset_ali"].IsNull())
		offset_ali = _config["offset_ali"].GetInt();
	try
	{
		PianoTranScriptionModel = new Ort::Session(*env, (_path + L"model.onnx").c_str(), *session_options);
	}
	catch(Ort::Exception& e)
	{
		throw std::exception(e.what());
	}
	_callback = _cb;
}

PianoTranScription::~PianoTranScription()
{
	delete PianoTranScriptionModel;
	PianoTranScriptionModel = nullptr;
}

// debug fn
std::vector<float> PianoTranScription::load_debug_npy()
{
	FILE* debug_file = nullptr;
	const auto path = GetCurrentFolder() + L"\\debug.npy";
		_wfopen_s(&debug_file, path.c_str(), L"rb");
	std::vector<float> debug_data(1441429, 0.0);
	if (debug_file)
	{
		fseek(debug_file, 128, SEEK_SET);
		debug_data.resize(fread_s(debug_data.data(), 1441429 * sizeof(float), 1, 1441429 * sizeof(float), debug_file) / sizeof(float));
	}
	return debug_data;
}

// write midi events vector to midi file
cxxmidi::File write_events_to_midi(long start_time, const PianoTranScription::midi_events& _midi_events)
{
	constexpr long ticks_per_beat = 384;
	constexpr long beats_per_second = 2;
	constexpr long ticks_per_second = ticks_per_beat * beats_per_second;

	cxxmidi::File my_file;
	cxxmidi::Track& track1 = my_file.AddTrack();
	my_file.SetTimeDivision(ticks_per_beat);
	std::vector<PianoTranScription::MidiEvent> _events;
	for (const auto& it : _midi_events.note)
	{
		_events.emplace_back(it.onset_time, it.midi_note, it.velocity);
		_events.emplace_back(it.offset_time, it.midi_note, 0);
	}
	std::sort(_events.begin(), _events.end());
	long previous_ticks = start_time;
	for (const auto& it : _events) {
		const long this_ticks = long((it.time - start_time) * ticks_per_second);
		if (this_ticks >= 0)
		{
			long diff_ticks = this_ticks - previous_ticks;
			if (diff_ticks < 0)
				diff_ticks = 0;
			previous_ticks = this_ticks;
			if (it.velocity)
				track1.push_back(cxxmidi::Event(diff_ticks, cxxmidi::Message::kNoteOn, uint8_t(unsigned long(it.midi_note)), uint8_t(unsigned long(it.velocity))));
			else
				track1.push_back(cxxmidi::Event(diff_ticks, cxxmidi::Message::kNoteOff, uint8_t(unsigned long(it.midi_note)), 0));
		}
	}
	track1.push_back(cxxmidi::Event(0,
		cxxmidi::Message::kMeta,
		cxxmidi::Message::kEndOfTrack));
	return my_file;
}

// Infer Function
void PianoTranScription::Infer(const std::wstring& _path, int64_t batch_size) const
{
#ifdef MDEBUGFILE
	auto _data = load_debug_npy();
#else
	auto _data = load_audio(_path);
#endif
	const size_t audio_len = _data.size();
	const size_t pad_len = size_t(ceil(double(audio_len) / double(segment_samples))) * segment_samples;
	if (audio_len < pad_len)
	{
		std::vector<float> pad(pad_len - audio_len, 0.0f);
		_data.insert(_data.end(), pad.begin(), pad.end());
	}
	size_t progress = 0;
	const size_t progressMax = (pad_len / segment_samples) * 2 - 1;
	std::vector<std::vector<std::vector<float>>> output[7];

	std::vector<float> segments;
	segments.reserve(segment_samples * progressMax);
	for (size_t i = 0; i + segment_samples <= pad_len; i += segment_samples / 2)
		segments.insert(segments.end(), _data.data() + i, _data.data() + i + segment_samples);

	for (size_t i = 0; i < segments.size(); i += segment_samples * batch_size)
	{
		_callback(progress, progressMax);
		std::vector<Ort::Value> inputTensors;
		if (progress + batch_size > progressMax)
			batch_size = int64_t(progressMax - progress);
		const int64_t inputShape[] = { batch_size, int64_t(segment_samples) };
		inputTensors.emplace_back(Ort::Value::CreateTensor(*memory_info,
			segments.data() + i,
			segment_samples * batch_size,
			inputShape,
			2));
		const auto outTensors = PianoTranScriptionModel->Run(Ort::RunOptions{ nullptr },
			inputNames.data(),
			inputTensors.data(),
			inputTensors.size(),
			outputNames.data(),
			outputNames.size()
		);
		progress += batch_size;
		for (size_t ita = 0; ita < 7; ++ita)
		{
			const auto Tdata = outTensors[ita].GetTensorData<float>();
			const auto TShape = outTensors[ita].GetTensorTypeAndShapeInfo().GetShape();
			for (int64_t it = 0; it < batch_size; ++it)
			{
				std::vector<std::vector<float>> tmp;
				tmp.reserve(TShape[1]);
				for (int64_t iter = 0; iter < TShape[1]; ++iter)
					tmp.emplace_back(Tdata + (it * TShape[1] + iter) * TShape[2], Tdata + (it * TShape[1] + iter + 1) * TShape[2]);
				output[ita].emplace_back(tmp);
			}
		}
	}
	std::vector<std::vector<float>> routput[7];
	if (progressMax > 1)
	{
		for (size_t i = 0; i < 7; ++i)
		{
			const auto b_size = int64_t((output[i][0].size() - 1) / 4);
			const auto e_size = int64_t((output[i][0].size() - 1) * 3 / 4);
			routput[i].reserve((progressMax + 2) * 2 * b_size);

			size_t it = 0;
			routput[i].insert(routput[i].end(), output[i][it].begin(), output[i][it].begin() + e_size);
			++it;
			for (; it < output[i].size() - 1; ++it)
				routput[i].insert(routput[i].end(), output[i][it].begin() + b_size, output[i][it].begin() + e_size);
			it = output[i].size() - 1;
			routput[i].insert(routput[i].end(), output[i][it].begin() + b_size, output[i][it].begin() + e_size + b_size);
		}
	}
#ifdef MDEBUG

	for (long pitch = 0; pitch < routput[0][0].size(); ++pitch)
	{
		for (long duration = 0; duration < routput[0].size(); ++duration)
		{
			std::cout << routput[0][duration][pitch] << ' ';
		}
		std::cout << '\n';
	}

#endif
	NetOutPuts netOutputs(routput);
	midi_events midiEvents;
	if (!use_org_method)
		midiEvents = frame_to_note_info(netOutputs.frame_output, netOutputs.reg_offset_output, netOutputs.velocity_output);
	else
		midiEvents = toMidiEvents(netOutputs);
	write_events_to_midi(0, midiEvents).SaveAs(to_byte_string(_outputPath + _path.substr(_path.rfind(L'\\'), _path.rfind(L'.')) + std::to_wstring(unsigned long long(_path.data())) + L".mid").c_str());
	_callback(progress, progressMax);
}

// detect note info with output dict (MyMethod, and It works better in my software)
PianoTranScription::midi_events PianoTranScription::frame_to_note_info(const std::vector<std::vector<float>>& frame_output, const std::vector<std::vector<float>>& reg_offset_output, const std::vector<std::vector<float>>& velocity_output) const
{
	const auto Temp = get_binarized_output_from_regression(reg_offset_output, float(offset_threshold), offset_ali);
	const auto offset = std::get<0>(Temp);
	std::vector<est_note_events> outputs;
	double onset = 0.0;
	const long class_size = long(frame_output[0].size()), duration_size = long(frame_output.size());
	for (long pitch = 0; pitch < class_size; ++pitch)
	{
		bool begin = false;
		for (long duration = 0; duration < duration_size; ++duration)
		{
			if (!begin && frame_output[duration][pitch] >= float(frame_threshold))
			{
				begin = true;
				onset = double(duration);
				continue;
			}
			if (begin)
			{
				if ((frame_output[duration][pitch] < float(frame_threshold)) ||
					(double(duration) - onset > 600.0) ||
					(duration == duration_size - 1) ||
					(offset[duration][pitch] == 1.0f))
				{
					begin = false;
					outputs.emplace_back(onset / double(frames_per_second), double(duration) / double(frames_per_second), pitch + begin_note, long(velocity_output[long(onset)][pitch] * float(velocity_scale) + 1));
				}
			}
		}
	}
	return { std::move(outputs),{} };
}

// load Audio to sampling rate: sr, dtype: float32
std::vector<float> PianoTranScription::load_audio(const std::wstring& _path) const
{
	const auto audio = AudioPreprocess().codec(_path, sample_rate, false);
	const auto dataLen = (uint64_t)audio.getDataLen();
	std::vector<float> _data(dataLen);
	for (size_t j = 0; j < dataLen; ++j)
		_data[j] = (float)audio[j] / 32768.0f;
	return _data;
}

// detect note info with onset offset & frame (Orginal Method)
std::vector<PianoTranScription::est_note_tp> note_detection_with_onset_offset_regress(
	const std::vector<std::vector<float>>& frame_output,
	const std::vector<std::vector<float>>& onset_output,
	const std::vector<std::vector<float>>& onset_shift_output,
	const std::vector<std::vector<float>>& offset_output,
	const std::vector<std::vector<float>>& offset_shift_output,
	const std::vector<std::vector<float>>& velocity_output,
	float frame_threshold, 
	long nk)
{
	const long frames_num = long(frame_output.size());

	std::vector<PianoTranScription::est_note_tp> output_tuples;

	bool begin = false, bframe_disappear = false, boffset_occur = false;
	long bgn = 9999, fin = 0, frame_disappear = 9999, offset_occur = 9999;

	for (long i = 0; i < frames_num; ++i)
	{
		if(onset_output[i][nk]==1.0f)
		{
			if(begin)
			{
				fin = max(i - 1, 0);
				output_tuples.emplace_back(bgn, fin, double(onset_shift_output[bgn][nk]), 0.0, double(velocity_output[bgn][nk]));
				bframe_disappear = false;
				boffset_occur = false;
			}
			bgn = i;
			begin = true;
		}
		if(begin && i > bgn)
		{
			if(frame_output[i][nk] <= frame_threshold && !bframe_disappear)
			{
				frame_disappear = i;
				bframe_disappear = true;
			}
			if ((offset_output[i][nk] == 1.0f) && !boffset_occur)
			{
				offset_occur = i;
				boffset_occur = true;
			}
			if(bframe_disappear)
			{
				if (boffset_occur && offset_occur - bgn > frame_disappear - offset_occur)
					fin = offset_occur;
				else
					fin = frame_disappear;
				output_tuples.emplace_back(bgn, fin, double(onset_shift_output[bgn][nk]), double(offset_shift_output[fin][nk]), double(velocity_output[bgn][nk]));
				bframe_disappear = false;
				boffset_occur = false;
				begin = false;
			}
			if(begin && (i - bgn >= 600 || i == frames_num - 1))
			{
				fin = i;
				output_tuples.emplace_back(bgn, fin, double(onset_shift_output[bgn][nk]), double(offset_shift_output[fin][nk]), double(velocity_output[bgn][nk]));
				bframe_disappear = false;
				boffset_occur = false;
				begin = false;
			}
		}
	}
	std::sort(output_tuples.begin(), output_tuples.end());
	return output_tuples;
}

// detect note info with output dict (Orginal Method)
std::vector<PianoTranScription::est_note_events> PianoTranScription::output_dict_to_detected_notes(const NetOutPuts& output_dict) const
{
	const long class_num = long(output_dict.frame_output[0].size());
	std::vector<est_note_tp> est_tuples;
	std::vector<long> est_midi_notes;
	for (long piano_note = 0; piano_note < class_num; ++piano_note)
	{
		auto est_tuples_per_note = note_detection_with_onset_offset_regress(
			output_dict.frame_output,
			output_dict.onset_output,
			output_dict.onset_shift_output,
			output_dict.offset_output,
			output_dict.offset_shift_output,
			output_dict.velocity_output,
			float(frame_threshold),
			piano_note
		);
		if(est_tuples_per_note.empty())
			continue;
		est_tuples.insert(est_tuples.end(), est_tuples_per_note.begin(), est_tuples_per_note.end());
		for (size_t ii = 0; ii < est_tuples_per_note.size(); ++ii)
			est_midi_notes.emplace_back(piano_note + begin_note);
	}
	std::vector<est_note_events> est_on_off_note_vels;
	est_on_off_note_vels.reserve(est_tuples.size());
	for (size_t i = 0; i < est_tuples.size(); ++i)
	{
		est_on_off_note_vels.emplace_back((double(est_tuples[i].fin) + est_tuples[i].onset_shift) / double(frames_per_second),
			(double(est_tuples[i].bgn) + est_tuples[i].offset_shift) / double(frames_per_second),
			est_midi_notes[i],
			long(est_tuples[i].normalized_velocity * velocity_scale));
	}
	return est_on_off_note_vels;
}

//std::vector<PianoTranScription::est_pedal_events> PianoTranScription::output_dict_to_detected_pedals(const NetOutPuts& output_dict) const
//{
//	return {};
//}

// NetOutputs to MidiEvents (Orginal Method)
PianoTranScription::midi_events PianoTranScription::toMidiEvents(NetOutPuts& output_dict) const
{
	std::vector<est_pedal_events> _pedal;
	auto Temp = get_binarized_output_from_regression(output_dict.reg_onset_output, float(onset_threshold), onset_ali);
	output_dict.onset_output = std::move(std::get<0>(Temp));
	output_dict.onset_shift_output = std::move(std::get<1>(Temp));
	Temp = get_binarized_output_from_regression(output_dict.reg_offset_output, float(offset_threshold), offset_ali);
	output_dict.offset_output = std::move(std::get<0>(Temp));
	output_dict.offset_shift_output = std::move(std::get<1>(Temp));
	std::vector<est_note_events> _note = output_dict_to_detected_notes(output_dict);
	return { std::move(_note),std::move(_pedal) };
}

// If the class normal distribution is satisfied, return true, else false
bool is_monotonic_neighbour(const std::vector<std::vector<float>>& x, long n, long neighbour, long k)
{
	bool monotonic = true;
	const long frames = long(x.size());
	for (long i = 0; i < neighbour; ++i)
	{
		if(n - i < 0 || !(n + i < frames))
			continue;
		if (x[n - i][k] < x[n - i - 1][k] || x[n + i][k] < x[n + i + 1][k])
			monotonic = false;
	}
	return monotonic;
}

// Look for Midi events on the timeline
std::tuple<std::vector<std::vector<float>>, std::vector<std::vector<float>>> PianoTranScription::get_binarized_output_from_regression(const std::vector<std::vector<float>>& reg_output, float threshold, int neighbour)
{
	const long frames_num = long(reg_output.size());
	const long class_num = long(reg_output[0].size());
	std::vector<std::vector<float>> binary_output(frames_num, std::vector<float>(class_num, 0.0));
	std::vector<std::vector<float>> shift_output(frames_num, std::vector<float>(class_num, 0.0));
	for (long k = 0; k < class_num; ++k)
	{
		for (long n = 0; n < frames_num; ++n)
		{
			if(reg_output[n][k] > threshold && is_monotonic_neighbour(reg_output, n, neighbour, k))
			{
				binary_output[n][k] = 1.0f;
				if (reg_output[n - 1][k] > reg_output[n + 1][k])
					shift_output[n][k] = (reg_output[n + 1][k] - reg_output[n - 1][k]) / (reg_output[n][k] - reg_output[n + 1][k]) / 2.0f;
				else
					shift_output[n][k] = (reg_output[n + 1][k] - reg_output[n - 1][k]) / (reg_output[n][k] - reg_output[n - 1][k]) / 2.0f;
			}
		}
	}
	return{ binary_output ,shift_output };
}

// operators used to sort
bool operator<(const PianoTranScription::est_note_tp& a, const PianoTranScription::est_note_tp& b)
{
	return a.bgn < b.bgn;
}
bool operator<(const PianoTranScription::MidiEvent& a, const PianoTranScription::MidiEvent& b)
{
	return a.time < b.time;
}
PianoTranScription::est_note_tp operator+(const PianoTranScription::est_note_tp& a, const PianoTranScription::est_note_tp& b)
{
	return { a.bgn + b.bgn,a.fin + b.fin,a.onset_shift + b.onset_shift,a.offset_shift + b.offset_shift,a.normalized_velocity + b.normalized_velocity };
}

INFERCLASSEND