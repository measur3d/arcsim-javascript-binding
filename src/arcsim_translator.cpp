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


void ArcsimTranslator::LoadBinJson( const std::string& json_path, const std::string& bin_path,
                  std::string& json_data, std::shared_ptr<char>& bin_data, uint64_t& bin_size)
{
    char* json_buffer;

    if(std::ifstream blob_bin_in{bin_path.c_str(), std::ios::binary | std::ios::ate}) {
        bin_size= blob_bin_in.tellg();
        bin_data = std::shared_ptr<char>(new char[bin_size], std::default_delete<char[]>());
        blob_bin_in.seekg(0);
        blob_bin_in.read(bin_data.get(), bin_size);
    }

    if(std::ifstream blob_json_in{json_path.c_str(), std::ios::ate}) {
        uint64_t size = blob_json_in.tellg();
        json_buffer = new char[(size+1)];
        blob_json_in.seekg(0);
        blob_json_in.read(json_buffer, size);
        json_buffer[size] = '\0';
    }
    
    json_data = std::string(json_buffer);
    delete [] json_buffer;    
}


Napi::Value ArcsimTranslator::ConvertLegacyGarment(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();

    if (info.Length() != 2) {
        Napi::TypeError::New(env, "Wrong number of arguments")
          .ThrowAsJavaScriptException();
        return env.Null();
    }

    for( int arg = 0; arg < 2; ++arg )
        if( !info[arg].IsString()) {
            Napi::TypeError::New(env, "Argument must be a filepath(string)")
                .ThrowAsJavaScriptException();
            return env.Null();
        }
        
    std::string garment_blob_filename = info[0].As<Napi::String>();
    std::string garment_json_filename = info[1].As<Napi::String>();
    std::string json_str;    
    uint64_t bin_buffer_size;
    std::shared_ptr<char> bin_buffer;

    LoadBinJson(garment_json_filename, garment_blob_filename, json_str, bin_buffer, bin_buffer_size);

    Geometry::Blob::BinBuffer buffer = {bin_buffer_size, bin_buffer.get()};
    Geometry::Blob blob;
    blob.Load( buffer );
  
    ARCSim::SceneT fb_scene;
    
    // Load Garment
    fb_scene.garments.emplace_back( std::make_unique<ARCSim::GarmentT>() );
    ARCSimTranslation::ConvertToFB( blob, json_str, *(fb_scene.garments.at(0)) );
    
    // Load Constraints
    ARCSimTranslation::ConvertToFB( blob, json_str, fb_scene.constraints );
    
    auto bytes = PackToBytestream( &fb_scene, ARCSim::SceneIdentifier() );

    Napi::Uint8Array js_byte_array = Napi::Uint8Array::New(env, bytes.second);
    memcpy( js_byte_array.Data(), bytes.first, bytes.second);

    return js_byte_array;
}


Napi::Value ArcsimTranslator::ConvertLegacyObstacle(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();

    if (info.Length() != 2) {
        Napi::TypeError::New(env, "Wrong number of arguments")
          .ThrowAsJavaScriptException();
        return env.Null();
    }

    for( int arg = 0; arg < 2; ++arg )
        if( !info[arg].IsString()) {
            Napi::TypeError::New(env, "Argument must be a filepath(string)")
                .ThrowAsJavaScriptException();
            return env.Null();
        }
        
    std::string obstacle_blob_filename = info[0].As<Napi::String>();
    std::string obstacle_json_filename = info[1].As<Napi::String>();
    std::string json_str;    
    uint64_t bin_buffer_size;
    std::shared_ptr<char> bin_buffer;

    LoadBinJson(obstacle_json_filename, obstacle_blob_filename, json_str, bin_buffer, bin_buffer_size);

    Geometry::Blob::BinBuffer buffer = {bin_buffer_size, bin_buffer.get()};
    Geometry::Blob blob;
    blob.Load( buffer );
  
    ARCSim::SceneT fb_scene;
    
    // Load Obstacle
    fb_scene.obstacles.emplace_back( std::make_unique<ARCSim::ObstacleT>() );
    ARCSimTranslation::ConvertToFB( blob, json_str, *(fb_scene.obstacles.at(0)) );

    auto bytes = PackToBytestream( &fb_scene, ARCSim::SceneIdentifier() );

    Napi::Uint8Array js_byte_array = Napi::Uint8Array::New(env, bytes.second);
    memcpy( js_byte_array.Data(), bytes.first, bytes.second);

    return js_byte_array;
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

    std::string garment_str, body_str;    
    uint64_t garment_buffer_size, body_buffer_size;
    std::shared_ptr<char> garment_buffer, body_buffer;

    LoadBinJson(garment_json_filename, garment_blob_filename, garment_str, garment_buffer, garment_buffer_size);
    LoadBinJson(obstacle_json_filename, obstacle_blob_filename, body_str, body_buffer, body_buffer_size);
    
    Geometry::Blob::BinBuffer garment_buf = {garment_buffer_size, garment_buffer.get()};
    Geometry::Blob garment_blob;
    garment_blob.Load( garment_buf );
    Geometry::Blob::BinBuffer body_buf = { body_buffer_size, body_buffer.get()};
    Geometry::Blob body_blob;
    body_blob.Load( body_buf );
  
    ARCSim::SceneT fb_scene;
    
    // Load Garment
    fb_scene.garments.emplace_back( std::make_unique<ARCSim::GarmentT>() );
    ARCSimTranslation::ConvertToFB( garment_blob, garment_str, *(fb_scene.garments.at(0)) );
    
    // Load Constraints
    ARCSimTranslation::ConvertToFB( garment_blob, garment_str, fb_scene.constraints );
    
    // Load Body Obstacle
    fb_scene.obstacles.emplace_back( std::make_unique<ARCSim::ObstacleT>() );
    ARCSimTranslation::ConvertToFB( body_blob, body_str, *(fb_scene.obstacles.at(0)) );
    
    auto bytes = PackToBytestream( &fb_scene, ARCSim::SceneIdentifier() );

    Napi::Uint8Array js_byte_array = Napi::Uint8Array::New(env, bytes.second);
    memcpy( js_byte_array.Data(), bytes.first, bytes.second);

    return js_byte_array;
}

Napi::Function ArcsimTranslator::GetClass(Napi::Env env) {
    return DefineClass(env, "ArcsimTranslator", {
            ArcsimTranslator::InstanceMethod("convert_legacy_arcsim_scene", &ArcsimTranslator::ConvertLegacyArcsimScene),
            ArcsimTranslator::InstanceMethod("convert_legacy_garment", &ArcsimTranslator::ConvertLegacyGarment),
            ArcsimTranslator::InstanceMethod("convert_legacy_obstacle", &ArcsimTranslator::ConvertLegacyObstacle)
    });
}
