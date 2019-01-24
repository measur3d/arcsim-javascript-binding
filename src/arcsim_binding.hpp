#ifndef ARCSIM_BINDING_HPP_
#define ARCSIM_BINDING_HPP_

#pragma once

#include <napi.h>
#include <map>
#include "shared_library.hpp"

class ArcsimBinding : public Napi::ObjectWrap<ArcsimBinding>
{
public:
    ArcsimBinding(const Napi::CallbackInfo&);
    Napi::Value Version(const Napi::CallbackInfo&);
    Napi::Value CreateSimulationSession(const Napi::CallbackInfo&);
    Napi::Value DestroySimulationSession(const Napi::CallbackInfo&);
    Napi::Value AddObstacle(const Napi::CallbackInfo&);
    Napi::Value AddGarment(const Napi::CallbackInfo&);
    Napi::Value StartSimulation(const Napi::CallbackInfo&);
    Napi::Value PauseSimulation(const Napi::CallbackInfo&);

    static Napi::Function GetClass(Napi::Env);

private:
    std::string plugin_path_;
    ARCSim::SharedLibrary::HandleType plugin_handle_;
    
    // Session info    
    std::map<int, void*> per_session_sim_params; 
};


#endif
