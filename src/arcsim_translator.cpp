#include "arcsim_translator.hpp"
#include <translation/arcsim_translation.hpp>

#include <iostream>
#include <fstream>
#include <string>


using namespace Napi;

ArcsimTranslator::ArcsimTranslator(const Napi::CallbackInfo& info) : ObjectWrap(info) {
    Napi::Env env = info.Env();

    if (info.Length() != 0) {
        Napi::TypeError::New(env, "Wrong number of arguments")
          .ThrowAsJavaScriptException();
        return;
    }
}

Napi::Value ArcsimTranslator::ConvertLegacyArcsimScene(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() != 4) {
        Napi::TypeError::New(env, "Wrong number of arguments")
          .ThrowAsJavaScriptException();
        return env.Null();
    }

    for( int arg = 0; arg < 4; ++arg )
        if( !info[arg].IsString()) {
            Napi::TypeError::New(env, "Argument must be a filepath(string)")
                .ThrowAsJavaScriptException();
            return env.Null();
        }
        
    std::string garment_blob_filename = info[0].As<Napi::String>();
    std::string garment_json_filename = info[1].As<Napi::String>();
    std::string obstacle_blob_filename = info[2].As<Napi::String>();
    std::string obstacle_json_filename = info[3].As<Napi::String>();
    uint64_t bin_buffer_size, body_bin_buffer_size;
    char* bin_buffer, *body_bin_buffer;
    char* json_buffer, *body_json_buffer;
    
    if(std::ifstream blob_bin_in{garment_blob_filename.c_str(), std::ios::binary | std::ios::ate}) {
        bin_buffer_size = blob_bin_in.tellg();
        bin_buffer = new char[bin_buffer_size];
        blob_bin_in.seekg(0);
        blob_bin_in.read(bin_buffer, bin_buffer_size);
    }

    if(std::ifstream blob_json_in{garment_json_filename.c_str(), std::ios::ate}) {
        uint64_t size = blob_json_in.tellg();
        json_buffer = new char[(size+1)];
        blob_json_in.seekg(0);
        blob_json_in.read(json_buffer, size);
        json_buffer[size] = '\0';
    }

    if(std::ifstream body_blob_bin_in{obstacle_blob_filename.c_str(), std::ios::binary | std::ios::ate}) {
        body_bin_buffer_size = body_blob_bin_in.tellg();
        body_bin_buffer = new char[body_bin_buffer_size];
        body_blob_bin_in.seekg(0);
        body_blob_bin_in.read(body_bin_buffer, body_bin_buffer_size);
    }

    if(std::ifstream body_blob_json_in{obstacle_json_filename.c_str(), std::ios::ate}) {
        uint64_t size = body_blob_json_in.tellg();
        body_json_buffer = new char[(size+1)];
        body_blob_json_in.seekg(0);
        body_blob_json_in.read(body_json_buffer, size);
        body_json_buffer[size] = '\0';
    }
    
    std::string json_str( json_buffer );
    std::string body_json_str( body_json_buffer );
    Geometry::Blob::BinBuffer buffer = {bin_buffer_size, bin_buffer};
    Geometry::Blob blob;
    blob.Load( buffer );
    Geometry::Blob::BinBuffer body_buffer = { body_bin_buffer_size, body_bin_buffer};
    Geometry::Blob body_blob;
    body_blob.Load( body_buffer );
  
    ARCSim::SceneT fb_scene;
    
    // Load Garment
    fb_scene.garments.emplace_back( std::make_unique<ARCSim::GarmentT>() );
    ARCSimTranslation::ConvertToFB( blob, json_str, *(fb_scene.garments.at(0)) );
    
    // Load Constraints
    ARCSimTranslation::ConvertToFB( blob, json_str, fb_scene.constraints );
    
    // Load Body Obstacle
    fb_scene.obstacles.emplace_back( std::make_unique<ARCSim::ObstacleT>() );
    ARCSimTranslation::ConvertToFB( body_blob, body_json_str, *(fb_scene.obstacles.at(0)) );
    
    auto bytes = PackToBytestream( &fb_scene, ARCSim::SceneIdentifier() );

    Napi::Uint8Array js_byte_array = Napi::Uint8Array::New(env, bytes.second);
    memcpy( js_byte_array.Data(), bytes.first, bytes.second);

    return js_byte_array;
}

Napi::Function ArcsimTranslator::GetClass(Napi::Env env) {
    return DefineClass(env, "ArcsimTranslator", {
    ArcsimTranslator::InstanceMethod("convert_legacy_arcsim_scene", &ArcsimTranslator::ConvertLegacyArcsimScene)
    });
}
