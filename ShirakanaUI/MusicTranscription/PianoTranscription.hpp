#pragma once
#include "../Inference/inference.hpp"
#include "../AVCodec/AvCodec.hpp"
#include "../Lib/rapidjson/document.h"

#include "../Audio/Midi.hpp"

INFERCLASSHEADER

class PianoTranScription : public BaseModelType
{
public:
	struct MidiEvent
	{
		double time = 0;
		long midi_note = 0;
		long velocity = 0;
		MidiEvent(double t = 0.0, long m = 0, long v = 0) :time(t), midi_note(m), velocity(v) {}
	};

	struct NetOutPuts
	{
		std::vector<std::vector<float>> frame_output, reg_onset_output, reg_offset_output, velocity_output, reg_pedal_onset_output, reg_pedal_offset_output, pedal_frame_output, onset_output, onset_shift_output, offset_output, offset_shift_output, pedal_onset_output, pedal_offset_output;
		NetOutPuts(std::vector<std::vector<float>>* input) :
			frame_output(std::move(input[0])),
			reg_onset_output(std::move(input[1])),
			reg_offset_output(std::move(input[2])),
			velocity_output(std::move(input[3])),
			reg_pedal_onset_output(std::move(input[4])),
			reg_pedal_offset_output(std::move(input[5])),
			pedal_frame_output(std::move(input[6])) {}
	};

	struct est_note_tp
	{
		long bgn = 0;
		long fin = 0;
		double onset_shift = 0.0;
		double offset_shift = 0.0;
		double normalized_velocity = 0.0;
		est_note_tp(long a, long b, double c, double d, double e) :bgn(a), fin(b), onset_shift(c), offset_shift(d), normalized_velocity(e) {}
	};

	struct est_note_events
	{
		double onset_time = 0.0;
		double offset_time = 0.0;
		long midi_note = 0;
		long velocity = 0;
		est_note_events(double a, double b, long c, long d) :onset_time(a), offset_time(b), midi_note(c), velocity(d) {}
	};

	struct est_pedal_events
	{
		double onset_time = 0.0;
		double offset_time = 0.0;
	};

	struct midi_events
	{
		std::vector<est_note_events> note;
		std::vector<est_pedal_events> pedal;
		midi_events() = default;
		midi_events(std::vector<est_note_events>&& ene, std::vector<est_pedal_events>&& epe) : note(ene), pedal(epe) {}
	};

	PianoTranScription(const rapidjson::Document& , const callback&);

	virtual ~PianoTranScription();

	midi_events frame_to_note_info(const std::vector<std::vector<float>>& frame_output, const std::vector<std::vector<float>>& offset_output, const std::vector<std::vector<float>>& velocity_output) const;

	std::vector<float> load_audio(const std::wstring& _path) const;

	cxxmidi::File Infer(const std::wstring& _path, int64_t batch_size = 1) const;

	midi_events toMidiEvents(NetOutPuts& output_dict) const;

	static std::tuple<std::vector<std::vector<float>>, std::vector<std::vector<float>>> get_binarized_output_from_regression(const std::vector<std::vector<float>>&, float, int);

	std::vector<est_note_events> output_dict_to_detected_notes(const NetOutPuts& output_dict) const;

	//std::vector<est_pedal_events> output_dict_to_detected_pedals(const NetOutPuts& output_dict) const;

	static std::vector<float> load_debug_npy();
private:
	Ort::Session* PianoTranScriptionModel = nullptr;
	size_t segment_samples = 160000;
	callback _callback;
	long sample_rate = 16000;
	long classes_num = 88;
	long begin_note = 21;
	float segment_seconds = 10.0f;
	float hop_seconds = 1.0f;
	float frames_per_second = 100.0;
	long velocity_scale = 128;
	double onset_threshold = 0.3;
	double offset_threshold = 0.3;
	double frame_threshold = 0.1;
	bool use_org_method = false;
	// double pedal_offset_threshold = 0.2;
	long onset_ali = 2;
	long offset_ali = 4;

	std::vector<const char*> inputNames = { "audio" };
	std::vector<const char*> outputNames = { "frame_output", "reg_onset_output", "reg_offset_output", "velocity_output", "reg_pedal_onset_output", "reg_pedal_offset_output", "pedal_frame_output" };
};

bool operator<(const PianoTranScription::est_note_tp& a, const PianoTranScription::est_note_tp& b);

bool operator<(const PianoTranScription::MidiEvent& a, const PianoTranScription::MidiEvent& b);

PianoTranScription::est_note_tp operator+(const PianoTranScription::est_note_tp& a, const PianoTranScription::est_note_tp& b);

INFERCLASSEND