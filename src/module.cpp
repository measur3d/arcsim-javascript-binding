
#include <napi.h>
#include "arcsim_binding.hpp"
#include "arcsim_translator.hpp"

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    
    Napi::String binding_name = Napi::String::New(env, "ArcsimBinding");
    exports.Set(binding_name, ArcsimBinding::GetClass(env));
    
    Napi::String translator_name = Napi::String::New(env, "ArcsimTranslator");
    exports.Set(translator_name, ArcsimTranslator::GetClass(env));
    
    return exports;
}

NODE_API_MODULE(addon, Init)
