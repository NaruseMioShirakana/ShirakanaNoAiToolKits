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
    long s_width = 64;
    long s_height = 64;
    callback _callback;
    Mui::MRender* _render = nullptr;

    std::vector<const char*> inputNames = { "src" };
    std::vector<const char*> outputNames = { "img" };
};

INFERCLASSEND