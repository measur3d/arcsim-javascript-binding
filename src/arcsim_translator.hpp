#ifndef ARCSIM_TRANSLATOR_HPP_
#define ARCSIM_TRANSLATOR_HPP_

#pragma once

#include <napi.h>

class ArcsimTranslator : public Napi::ObjectWrap<ArcsimTranslator>
{
public:
    ArcsimTranslator(const Napi::CallbackInfo&);
    Napi::Value ConvertLegacyArcsimScene(const Napi::CallbackInfo&);

    static Napi::Function GetClass(Napi::Env);
};

#endif
