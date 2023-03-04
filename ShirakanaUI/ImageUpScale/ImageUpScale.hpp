#pragma once
#include "../Inference/inference.hpp"
#include "../Lib/rapidjson/document.h"

INFERCLASSHEADER

class RealESRGan : public BaseModelType
{
public:
    RealESRGan(const rapidjson::Document&, const callback&, Mui::MRender*);

    virtual ~RealESRGan();

    void Infer(const std::wstring&, int64_t) const override;

    std::vector<float> pre_process(std::vector<float>&);

private:
    Ort::Session* model = nullptr;
    long scale = 0;
    long tile_size = 0;
    long tile_pad = 0;
    long pre_pad = 0;
    long mod_scale = 0;
    long half = 0;
    long mod_pad_h = 0;
    long mod_pad_w = 0;
    callback _callback;
    Mui::MRender* _render = nullptr;
};

INFERCLASSEND