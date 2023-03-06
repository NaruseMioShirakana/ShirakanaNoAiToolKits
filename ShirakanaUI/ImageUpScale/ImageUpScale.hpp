#pragma once
#include "../Inference/inference.hpp"
#ifdef max
#undef max
#include "rapidjson.h"
#include "document.h"
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif
INFERCLASSHEADER

class RealESRGan : public BaseModelType
{
public:
    RealESRGan(const rapidjson::Document&, const callback&, Mui::MRender*);

    virtual ~RealESRGan();

    void Infer(const std::wstring&, int64_t) const override;

private:
    Ort::Session* model = nullptr;
    Ort::Session* model_alpha = nullptr;
    long scale = 4;
    long tile_size = 10;
    long tile_pad = 10;
    long pre_pad = 10;
    long mod_scale = 4;
    callback _callback;
    Mui::MRender* _render = nullptr;

    std::vector<const char*> inputNames = { "audio" };
    std::vector<const char*> outputNames = { "frame_output", "reg_onset_output", "reg_offset_output", "velocity_output", "reg_pedal_onset_output", "reg_pedal_offset_output", "pedal_frame_output" };
};

INFERCLASSEND