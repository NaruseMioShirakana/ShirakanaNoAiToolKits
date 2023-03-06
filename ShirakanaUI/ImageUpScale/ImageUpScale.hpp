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