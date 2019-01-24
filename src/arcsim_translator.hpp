#ifndef ARCSIM_TRANSLATOR_HPP_
#define ARCSIM_TRANSLATOR_HPP_

#pragma once

#include <napi.h>

class ArcsimTranslator : public Napi::ObjectWrap<ArcsimTranslator>
{
public:
    ArcsimTranslator(const Napi::CallbackInfo&);
    Napi::Value ConvertLegacyArcsimScene(const Napi::CallbackInfo&);
    Napi::Value ConvertLegacyGarment(const Napi::CallbackInfo&);
    Napi::Value ConvertLegacyObstacle(const Napi::CallbackInfo&);

    static Napi::Function GetClass(Napi::Env);

private:

    void LoadBinJson( const std::string& json_path, const std::string& bin_path,
                      std::string& json_data, std::shared_ptr<char>& bin_data, uint64_t& bin_size);
    

    
};

#endif
