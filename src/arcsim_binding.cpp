#include "arcsim_binding.hpp"
#include "interface.hpp"
#include <translation/arcsim_translation.hpp>

#include <iostream>
#include <string>
#include <fstream>

#include "napi-thread-safe-callback.hpp"



using namespace Napi;

namespace api_functions {
typedef uint32_t arcsim_version(unsigned int* major, unsigned int* minor, unsigned int* patch, char* build);
typedef uint32_t api_version(unsigned int* major, unsigned int* minor);
typedef uint32_t create_session(int version_major, int version_minor, SessionType session_type, int* out_handle);
typedef uint32_t destroy_session(int session_handle);
typedef uint32_t get_error_message(int session_handle, const char*& message );
typedef uint32_t validate_garment(const char *json, BinBlob *bin);
typedef uint32_t validate_garment_from_file(const char* json_file, const char* bin_file);
typedef uint32_t validate_body(const char *json, BinBlob *bin);
typedef uint32_t validate_body_from_file(const char* json_file, const char* bin_file);
typedef uint32_t add_garment(int session_handle, const char* name, const char* json, BinBlob* bin, int* out_handle);
typedef uint32_t add_garment_from_file(int session_handle, const char* json_file, const char* bin_file, int* out_handle);
typedef uint32_t add_handle(int session_handle, int garment_handle, const char* json, int* out_handle );
typedef uint32_t remove_handle(int session_handle, int handle_handle );
typedef uint32_t get_last_handle_id(int session_handle, int* last_handle_id);    
typedef uint32_t set_handle_properties( int session_handle, int handle_handle, HandleParams* params);
typedef uint32_t get_handle_properties( int session_handle, int handle_handle, HandleParams* params);
typedef uint32_t get_handle_locations( int session_handle, int handle_handle, HandleLocations** locations);
typedef uint32_t free_handle_locations( int session_handle, HandleLocations* locations);    
typedef uint32_t add_obstacle(int session_handle, const char *json, BinBlob *bin, int* out_handle);
typedef uint32_t add_obstacle_from_file(int session_handle, const char* json_file, const char* bin_file, int* out_handle);
typedef uint32_t add_obstacle_frame(int session_handle, int obstacle_handle, int frame, BinBlob *bin);
typedef uint32_t add_obstacle_frame_from_file(int session_handle, int obstacle_handle, int frame, const char* bin_file);
typedef uint32_t get_garment_mesh(int session_handle, int garment_handle, BinBlob** out_mesh, bool export_sim_metadata);
typedef uint32_t free_garment_mesh(BinBlob* mesh);
typedef uint32_t save_mesh(int session_handle, int garment_handle, const char *out_filename);
typedef uint32_t extrude_mesh(int session_handle, int garment_handle, BinBlob** out_mesh);
typedef uint32_t finalize_mesh(int session_handle);
typedef uint32_t freeze_piece(int session_handle, int garment_handle, const char* piece_name);
typedef uint32_t unfreeze_piece(int session_handle, int garment_handle, const char* piece_name);
typedef uint32_t prepare_simulation(int session_handle, SimParams* params);
typedef uint32_t get_default_simulation_parameters(SimParams *params);
typedef uint32_t damp_velocities(int session_handle, double factor);
typedef uint32_t prepare_stitching(int session_handle, StitchingParams* params);
typedef uint32_t get_default_stitching_parameters(StitchingParams *params);
typedef uint32_t prepare_meshing(int session_handle, MeshingParams* params);
typedef uint32_t get_default_meshing_parameters(MeshingParams* params);
typedef uint32_t get_session_status(int session_handle, SessionStatus* out_status);
typedef uint32_t start_session(int session_handle);
typedef uint32_t pause_session(int session_handle);
typedef uint32_t reset_session(int session_handle);
}

#define STRINGIFY(name) #name
#define GetFunction(name, ...) \
    {\
        std::cout << "Calling " << STRINGIFY(name) << " from ARCSim Library..." << std::endl;\
        api_functions:: name * fnc_ptr;\
        try {\
            fnc_ptr = ARCSim::SharedLibrary::GetFunctionPointer< api_functions:: name >(plugin_handle_, STRINGIFY(name)); \
        }\
        catch( std::exception& err ) {\
            Napi::Error::New(env, std::string("Binding Error: ") + err.what()).ThrowAsJavaScriptException();\
            return env.Null();\                                                            
        }\
        try{\
           validate(env, fnc_ptr(__VA_ARGS__ ) );\
        }\
        catch( ... ) {\
            Napi::Error::New(env, std::string("Unknown Error") ).ThrowAsJavaScriptException();\
            return env.Null();\                                                            
        }\
};

#define GetFunctionNoReturn(name, ...) \
    {\
        std::cout << "Calling " << STRINGIFY(name) << " from ARCSim Library..." << std::endl;\
        api_functions:: name * fnc_ptr;\
        try {\
            fnc_ptr = ARCSim::SharedLibrary::GetFunctionPointer< api_functions:: name >(plugin_handle_, STRINGIFY(name)); \
        }\
        catch( std::exception& err ) {\
            Napi::Error::New(env, std::string("Binding Error: ") + err.what()).ThrowAsJavaScriptException();\
        }\
        try{\
           validate(env, fnc_ptr(__VA_ARGS__ ) );\
        }\
        catch( ... ) {\
            Napi::Error::New(env, std::string("Unknown Error") ).ThrowAsJavaScriptException();\
        }\
};

struct BindingContext {
    BindingContext(Napi::Env& env) :
        env(env)
    {}

    std::vector<int> garment_handles, obstacle_handles;    
    ARCSim::SharedLibrary::HandleType plugin_handle_;
    Napi::Env env;
    ThreadSafeCallback* callback;
    std::string garment_json;    
};


std::string translateError( ErrorCode code ){
    switch( code ){
    case ARC_OK: return "Success";
    case ARC_InternalError: return "Internal Engine Error";
    case ARC_InvalidEngineVersion: return "Invalid Engine Version";
    case ARC_InvalidSessionHandle: return "Invalid Session Id";
    case ARC_InvalidGarmentHandle: return "Invalid Garment Id";
    case ARC_InvalidObstacleHandle: return "Invalid Obstacle Id";
    case ARC_InvalidConstraintHandle: return "Invalid Constraint Id";
    case ARC_InvalidFilename: return "Invalid Filename";
    case ARC_InvalidFileFormat: return "Invalid File Format";
    case ARC_NullData: return "Null Data";
    case ARC_InvalidSessionType: return "Invalid Session Type";
    case ARC_SessionRunning: return "Session Running";
    case ARC_SessionCompleted: return "Session Completed";
    case ARC_SessionInFailure: return "Session In Failure";
    case ARC_SessionUninitialized: return "Session Uninitialized";
    case ARC_SessionInitialized: return "Session Initialized";
    case ARC_SessionMissingGarment: return "Session Missing Garment";
    case ARC_SessionMissingObstacle: return "Session Missing Obstacle";
    case ARC_InvalidJSON: return "Invalid JSON";
    case ARC_InvalidRequest: return "Invalid Request";
    case ARC_InvalidArgument: return "Invalid Argument";
    case ARC_ParameterOutOfBounds: return "Parameter Out of Bound";
    case ARC_Unknown: return "Unknown Error";
    default: return "Unknown Error Code";
    }
}

void validate(Napi::Env& env, ErrorCode code ){
    if(code != ARC_OK ){
        std::string errorStr = translateError( code );
        Napi::Error::New(env, std::string("ARCSim Error: ") + errorStr)
            .ThrowAsJavaScriptException();   
    }
}

ArcsimBinding::ArcsimBinding(const Napi::CallbackInfo& info) : ObjectWrap(info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1) {
        Napi::TypeError::New(env, "Wrong number of arguments")
          .ThrowAsJavaScriptException();
        return;
    }

    if (!info[0].IsString()) {
        Napi::TypeError::New(env, "Path to ARCSim library must be provided.")
          .ThrowAsJavaScriptException();
        return;
    }

    this->plugin_path_ = info[0].As<Napi::String>().Utf8Value();

    try{
        std::cout << "Attempting to load plugin at: " << std::endl;
        std::cout << this->plugin_path_ << std::endl;
        plugin_handle_ = ARCSim::SharedLibrary::Load( this->plugin_path_ );
    }
    catch( std::runtime_error& err ){
        std::cout << err.what() << std::endl;
        Napi::Error::New(env, std::string("ARCSim Plugin could not be loaded: ") + err.what())
          .ThrowAsJavaScriptException();        
    }
}

Napi::Value ArcsimBinding::Version(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() != 0) {
        Napi::TypeError::New(env, "Wrong number of arguments")
          .ThrowAsJavaScriptException();
        return env.Null();
    }

    unsigned int major, minor, patch, api_major, api_minor;
    char build[32];
    GetFunction(arcsim_version, &major, &minor, &patch, build);
    GetFunction(api_version, &api_major, &api_minor);

    auto ret = Napi::Object::New(env);
    ret.Set( "major", Napi::Number::New(env, major) );
    ret.Set( "minor", Napi::Number::New(env, minor) );
    ret.Set( "patch", Napi::Number::New(env, patch) );
    ret.Set( "api_major", Napi::Number::New(env, api_major) );
    ret.Set( "api_minor", Napi::Number::New(env, api_minor) );
    ret.Set( "build", Napi::String::New(env, std::string(build) ) );

    return ret;
}

struct ARCSimSession
{
    SimParams params;
    MeshingParams meshing_params;    
    bool has_initialized;
};

   
Napi::Value ArcsimBinding::CreateSimulationSession(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() > 1) {
        Napi::TypeError::New(env, "Wrong number of arguments")
          .ThrowAsJavaScriptException();
        return env.Null();
    }

    ARCSimSession* session = new ARCSimSession();
    SimParams& params = session->params;
    
    GetFunction(get_default_simulation_parameters, &params);
    params.callback.data_passthrough_ptr = nullptr;
    
    if( info.Length() == 1) {
        if( !info[0].IsObject()) {
            Napi::TypeError::New(env, "Session configuration must be an object")
                .ThrowAsJavaScriptException();
            return env.Null();
        }

#define ADD_INT32_OPTION(OPTION) if( config.Has( #OPTION ) ) params.OPTION = config.Get( #OPTION ).ToNumber().Uint32Value();
#define ADD_INT8_OPTION(OPTION) if( config.Has( #OPTION ) ) params.OPTION = config.Get( #OPTION ).ToNumber().ToBoolean();
#define ADD_DOUBLE_OPTION(OPTION) if( config.Has( #OPTION ) ) params.OPTION = config.Get( #OPTION ).ToNumber().DoubleValue();
        
        Napi::Object config = info[0].As<Napi::Object>();

        ADD_INT32_OPTION(  max_frames);
        ADD_INT8_OPTION(  final_upsample);
        ADD_INT8_OPTION(  enable_collisions);
        ADD_INT32_OPTION(  collisions_physical_mode);
        ADD_DOUBLE_OPTION(  collisions_physical_sdf_stiffness);
        ADD_DOUBLE_OPTION(  collisions_physical_repulsion_stiffness);
        ADD_DOUBLE_OPTION(  collisions_physical_repulsion_thickness);
        ADD_INT32_OPTION(  collisions_geometric_mode);
        ADD_INT32_OPTION(  collisions_geometric_frequency);
        ADD_INT8_OPTION(  collisions_geometric_obstacle_aware); 
        ADD_INT32_OPTION(  collisions_geometric_local_project_max_iter);
        ADD_DOUBLE_OPTION(  collisions_geometric_local_project_offset);
        ADD_INT8_OPTION(  collisions_geometric_local_project_hard_nodes);
        ADD_DOUBLE_OPTION(  collisions_geometric_icm_max_step);
        ADD_INT32_OPTION(  collisions_geometric_icm_max_iter); 
        ADD_DOUBLE_OPTION(  collisions_geometric_harmon_projection_thickness); 
        ADD_INT32_OPTION(  collisions_geometric_harmon_max_iter);
        ADD_INT8_OPTION(  enable_physics);
        ADD_INT8_OPTION(  physics_solver_legacy);
        ADD_DOUBLE_OPTION(  friction);
        ADD_DOUBLE_OPTION(  obs_friction);
        ADD_DOUBLE_OPTION(  gravity);
        ADD_INT8_OPTION(  enable_wind); 
        ADD_DOUBLE_OPTION(  wind_density);
        ADD_DOUBLE_OPTION(  wind_drag);
        ADD_INT8_OPTION(  should_remesh);
        ADD_DOUBLE_OPTION(  remeshing_min_triangle_area);
        ADD_DOUBLE_OPTION(  remeshing_max_triangle_area);
        ADD_DOUBLE_OPTION(  remeshing_2dcurve_refinement);
        ADD_INT8_OPTION(  enable_initial_relaxation);
        ADD_INT8_OPTION(  use_local_shell_optimization);
        ADD_INT8_OPTION(  initial_relaxation_enable_static_friction);   
        ADD_DOUBLE_OPTION(  initial_relaxation_layer_size);
        ADD_DOUBLE_OPTION(  initial_relaxation_layer_dist);
        ADD_DOUBLE_OPTION(  initial_relaxation_min_sdf_distance);
        ADD_DOUBLE_OPTION(  initial_relaxation_mesh_resolution);
        ADD_INT8_OPTION(  initial_relaxation_use_isotropic_material);
        ADD_DOUBLE_OPTION(  initial_relaxation_isotropic_density);
        ADD_DOUBLE_OPTION(  initial_relaxation_isotropic_stretching);
        ADD_DOUBLE_OPTION(  initial_relaxation_isotropic_bending);
        ADD_DOUBLE_OPTION(  initial_relaxation_sdf_stiffness);
        ADD_INT32_OPTION(  initial_relaxation_max_shell_iterations);
        ADD_INT32_OPTION(  initial_relaxation_max_deflate_phase1_iterations);
        ADD_INT32_OPTION(  initial_relaxation_max_deflate_phase2_iterations);
        ADD_INT8_OPTION(  initial_relaxation_verbose_iterations);
        ADD_DOUBLE_OPTION(  finalize_gravity);
        ADD_INT32_OPTION(  finalize_optimization_max_iter);
        ADD_DOUBLE_OPTION(  finalize_optimization_max_step);
        ADD_DOUBLE_OPTION(  finalize_collisions_physical_sdf_stiffness);
        ADD_INT32_OPTION(  finalize_collisions_geometric_frequency);
        ADD_INT32_OPTION(  finalize_collisions_geometric_local_project_max_iter);
        ADD_DOUBLE_OPTION(  finalize_collisions_geometric_local_project_offset);
        ADD_INT8_OPTION(  finalize_collisions_geometric_local_project_hard_nodes);
        ADD_DOUBLE_OPTION(  finalize_collisions_geometric_harmon_projection_thickness);
        ADD_INT32_OPTION(  finalize_collisions_geometric_harmon_max_iter);
        
        if(config.Has("callback")){
            if(!config.Get("callback").IsFunction()){
                delete session;
                Napi::TypeError::New(env, "Callback must be a function")
                    .ThrowAsJavaScriptException();
                return env.Null();
            }
            BindingContext* bindingContext = new BindingContext(env);
            bindingContext->plugin_handle_ = plugin_handle_;
            bindingContext->env = env;
            bindingContext->callback = new ThreadSafeCallback(config.Get("callback").As<Function>());
            params.callback.data_passthrough_ptr = bindingContext;
        }
    }

    ErrorCode result;
    unsigned int api_major, api_minor;
    int session_handle;

    GetFunction(api_version, &api_major, &api_minor);
    GetFunction(create_session, api_major, api_minor, ST_Simulation, &session_handle);

    // Setup the callback function - its a stub that redirects calls to the JS callback, if one is set
    params.callback.func_ptr = [](CallbackData data){
        if( !data.data_passthrough )
            return;
        BindingContext& bindingContext = *reinterpret_cast<BindingContext*>(data.data_passthrough);
        ARCSim::SharedLibrary::HandleType& plugin_handle_ = bindingContext.plugin_handle_;
        Napi::Env env = bindingContext.env;
        ThreadSafeCallback& js_callback = *(bindingContext.callback);
        // Fetch the current mesh...
        Geometry::Blob blob;
        BinBlob* garment_data;
        GetFunctionNoReturn(get_garment_mesh,
                            data.session_status.handle,
                            0 /* This is bad! We need to replace with all garments! */,
                            &garment_data,
                            false
                            );
        blob.Load( *garment_data );
        GetFunctionNoReturn(free_garment_mesh, garment_data);
        ARCSim::GarmentFrameT fb_garmentFrame;        
        try{
            ARCSimTranslation::ConvertToFB(blob, fb_garmentFrame);
            fb_garmentFrame.frame = data.session_status.frame;
            fb_garmentFrame.subframe = data.session_status.steps;
            fb_garmentFrame.timestamp = data.session_status.time;
        }
        catch( std::exception& err ){
            Napi::Error::New(env, std::string("Failed to convert garment: ")+err.what() ).ThrowAsJavaScriptException();
            return;
        }

        const char* error_msg = nullptr;
        if( data.type == CT_Error ){
            GetFunctionNoReturn(get_error_message, data.session_handle, error_msg);
        }
        
        auto bytes = PackToBytestream( &fb_garmentFrame, nullptr );        
        
        js_callback.call([data, bytes, error_msg](Napi::Env env, std::vector<napi_value>& args)
        {
            const int type = data.type;
            Napi::Uint8Array js_byte_array = Napi::Uint8Array::New(env, bytes.second);
            memcpy( js_byte_array.Data(), bytes.first, bytes.second);

            Napi::Array garment_updates = Napi::Array::New(env);
            garment_updates[0u] = js_byte_array;
            
            Napi::Object status = Napi::Object::New(env);
            status.Set("handle", Napi::Number::New(env, data.session_status.handle));
            status.Set("type",  Napi::Number::New(env, data.session_status.type));
            status.Set("state",  Napi::Number::New(env, data.session_status.state));
            status.Set("frame",  Napi::Number::New(env, data.session_status.frame));
            status.Set("steps",  Napi::Number::New(env, data.session_status.steps));
            status.Set("time",  Napi::Number::New(env, data.session_status.time));
            if(error_msg)
                status.Set("error", Napi::String::New(env, error_msg));            
            status.Set("garment_data", garment_updates );
            
            args = { Napi::Number::New(env, type),
                     Napi::Number::New(env, data.session_handle),
                     status };
        });
    };    
    
    per_session_sim_params.insert( {session_handle, session} );
    
    return Napi::Number::New(env, session_handle);
}

Napi::Value ArcsimBinding::AddObstacle(const Napi::CallbackInfo& info){
    Napi::Env env = info.Env();

    if (info.Length() != 2) {
        Napi::TypeError::New(env, "Wrong number of arguments")
          .ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!info[0].IsNumber()) {
        Napi::TypeError::New(env, "Session handle must be provided")
          .ThrowAsJavaScriptException();
        return env.Null();
    }
    int session_handle = info[0].As<Napi::Number>().Int32Value();
    
    auto res = per_session_sim_params.find( session_handle );
    if(res == per_session_sim_params.end() ){
        Napi::Error::New(env, "Invalid Session Handle")
            .ThrowAsJavaScriptException();
        return env.Null();
    }
    
    ARCSimSession& session = *reinterpret_cast<ARCSimSession*>(res->second);
    
    if (!info[1].IsTypedArray()) {
        Napi::TypeError::New(env, "Obstacle data must be provided as a Uint8 TypedArray")
          .ThrowAsJavaScriptException();
        return env.Null();
    }
    Napi::TypedArray obstacle_plain_data = info[1].As<Napi::TypedArray>();
    if (obstacle_plain_data.TypedArrayType() != napi_uint8_array) {
        Napi::TypeError::New(env, "Obstacle data must be provided as a Uint8 TypedArray")
          .ThrowAsJavaScriptException();
        return env.Null();
    }
    
    Napi::Uint8Array u8array = obstacle_plain_data.As<Napi::Uint8Array>();
    uint8_t* obstacle_data = obstacle_plain_data.As<Napi::Uint8Array>().Data();           
    BinBlob obstacle_blob;
    std::string obstacle_json;
    //TODO: Convert obstacle data to blob and json here!!!   
    ARCSim::ObstacleFrameT* fb_obsframe = UnPackFromBytestream<ARCSim::ObstacleFrameT>(obstacle_data,
                                                                                       obstacle_plain_data.ElementLength(),
                                                                                       nullptr);
    if(!fb_obsframe){
        Napi::TypeError::New(env, "Obstacle data must be a packed ObstacleFrame")
            .ThrowAsJavaScriptException();
        return env.Null();
    }
    Geometry::Blob blob;
    try{
        ARCSimTranslation::ConvertFromFB(blob, obstacle_json, *fb_obsframe);
    }
    catch( std::exception& err ){
        Napi::Error::New(env, std::string("Failed to convert obstacle: ")+err.what() ).ThrowAsJavaScriptException();
        return env.Null();
    }
    auto tmp_binblob = blob.Save();
    obstacle_blob.buffer = tmp_binblob->buffer;
    obstacle_blob.len = tmp_binblob->len;    
        
    int obstacle_handle;    
    GetFunction(validate_body, obstacle_json.c_str() , &obstacle_blob );
    GetFunction(add_obstacle, session_handle, obstacle_json.c_str(), &obstacle_blob,   &obstacle_handle);
    
    return Napi::Number::New(env, obstacle_handle);
}

Napi::Value ArcsimBinding::AddGarment(const Napi::CallbackInfo& info){
    Napi::Env env = info.Env();

    if (info.Length() != 2) {
        Napi::TypeError::New(env, "Wrong number of arguments")
          .ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!info[0].IsNumber()) {
        Napi::TypeError::New(env, "Session handle must be provided")
          .ThrowAsJavaScriptException();
        return env.Null();
    }
    int session_handle = info[0].As<Napi::Number>().Int32Value();
    
    auto res = per_session_sim_params.find( session_handle );
    if(res == per_session_sim_params.end() ){
        Napi::Error::New(env, "Invalid Session Handle")
            .ThrowAsJavaScriptException();
        return env.Null();
    }
    
    ARCSimSession& session = *reinterpret_cast<ARCSimSession*>(res->second);
    
    if (!info[1].IsTypedArray()) {
        Napi::TypeError::New(env, "Garment data must be provided as a Uint8 TypedArray")
          .ThrowAsJavaScriptException();
        return env.Null();
    }
    Napi::TypedArray garment_plain_data = info[1].As<Napi::TypedArray>();
    if (garment_plain_data.TypedArrayType() != napi_uint8_array) {
        Napi::TypeError::New(env, "Garment data must be provided as a Uint8 TypedArray")
          .ThrowAsJavaScriptException();
        return env.Null();
    }
    
    Napi::Uint8Array u8array = garment_plain_data.As<Napi::Uint8Array>();
    uint8_t* garment_data = garment_plain_data.As<Napi::Uint8Array>().Data();           
    BinBlob garment_blob;
    std::string garment_json;
    //TODO: Convert garment data to blob and json here!!!   
    ARCSim::GarmentT* fb_garment = UnPackFromBytestream<ARCSim::GarmentT>(garment_data,
                                                                                garment_plain_data.ElementLength(),
                                                                                nullptr);
    if(!fb_garment){
        Napi::TypeError::New(env, "Garment data must be a packed Garment")
            .ThrowAsJavaScriptException();
        return env.Null();
    }
    Geometry::Blob blob;
    
    try{
        ARCSimTranslation::ConvertFromFB(blob, garment_json, *fb_garment);
    }
    catch( std::exception& err ){
        Napi::Error::New(env, std::string("Failed to convert garment: ")+err.what() ).ThrowAsJavaScriptException();
        return env.Null();
    }
    
    auto tmp_binblob = blob.Save();
    garment_blob.buffer = tmp_binblob->buffer;
    garment_blob.len = tmp_binblob->len;    

    std::ofstream blob_out("./blob.bin");
    blob_out.write( garment_blob.buffer, garment_blob.len );
    blob_out.close();
    
    int garment_handle;    
    GetFunction(validate_garment, garment_json.c_str() , &garment_blob );
    GetFunction(add_garment, session_handle, "data_garment", garment_json.c_str(), &garment_blob,   &garment_handle);
    
    return Napi::Number::New(env, garment_handle);
}


Napi::Value ArcsimBinding::StartSimulation(const Napi::CallbackInfo& info){
    Napi::Env env = info.Env();

    if (info.Length() != 1) {
        Napi::TypeError::New(env, "Wrong number of arguments")
          .ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!info[0].IsNumber()) {
        Napi::TypeError::New(env, "Session handle must be provided")
          .ThrowAsJavaScriptException();
        return env.Null();
    }
    int session_handle = info[0].As<Napi::Number>().Int32Value();
    
    auto res = per_session_sim_params.find( session_handle );
    if(res == per_session_sim_params.end() ){
        Napi::Error::New(env, "Invalid Session Handle")
            .ThrowAsJavaScriptException();
        return env.Null();
    }
    
    
    ARCSimSession& session = *reinterpret_cast<ARCSimSession*>(res->second);

    if(! session.has_initialized ){
        GetFunction(prepare_simulation, session_handle, &session.params);
        session.has_initialized = true;
    }

    GetFunction(start_session, session_handle);
    
    return env.Null();    
}

Napi::Value ArcsimBinding::PauseSimulation(const Napi::CallbackInfo& info){
    Napi::Env env = info.Env();

    if (info.Length() != 1) {
        Napi::TypeError::New(env, "Wrong number of arguments")
          .ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!info[0].IsNumber()) {
        Napi::TypeError::New(env, "Session handle must be provided")
          .ThrowAsJavaScriptException();
        return env.Null();
    }
    int session_handle = info[0].As<Napi::Number>().Int32Value();
    
    auto res = per_session_sim_params.find( session_handle );
    if(res == per_session_sim_params.end() ){
        Napi::Error::New(env, "Invalid Session Handle")
            .ThrowAsJavaScriptException();
        return env.Null();
    }
    
    
    ARCSimSession& session = *reinterpret_cast<ARCSimSession*>(res->second);

    GetFunction(pause_session, session_handle);
    
    return env.Null();    
}

Napi::Value ArcsimBinding::DestroySimulationSession(const Napi::CallbackInfo& info){
    Napi::Env env = info.Env();

    if (info.Length() != 1) {
        Napi::TypeError::New(env, "Wrong number of arguments")
          .ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!info[0].IsNumber()) {
        Napi::TypeError::New(env, "Session handle must be provided")
          .ThrowAsJavaScriptException();
        return env.Null();
    }
    int session_handle = info[0].As<Napi::Number>().Int32Value();

    auto res = per_session_sim_params.find( session_handle );
    if(res == per_session_sim_params.end() ){
        Napi::Error::New(env, "Invalid Session Handle")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    GetFunction(destroy_session, session_handle);

    return env.Null();    
}

Napi::Value ArcsimBinding::GenerateMesh(const Napi::CallbackInfo& info){
    Napi::Env env = info.Env();

    if (info.Length() != 2) {
        Napi::TypeError::New(env, "Wrong number of arguments")
          .ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if( !info[0].IsString()) {
        Napi::TypeError::New(env, "Argument 1 must be a JSON String")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    if( !info[1].IsFunction()) {
        Napi::TypeError::New(env, "Argument 2 must be a callback function")
            .ThrowAsJavaScriptException();
        return env.Null();
    }


    ARCSimSession* session = new ARCSimSession();
    MeshingParams& params = session->meshing_params;
    
    GetFunction(get_default_meshing_parameters, &params);
    params.callback.data_passthrough_ptr = nullptr;

    unsigned int api_major, api_minor;
    int session_handle;

    GetFunction(api_version, &api_major, &api_minor);
    GetFunction(create_session, api_major, api_minor, ST_Meshing, &session_handle);

    int garment_handle;
    std::string garment_json = info[0].As<Napi::String>().Utf8Value();
    GetFunction(add_garment, session_handle, "data_garment", garment_json.c_str(), nullptr, &garment_handle);
    
    BindingContext* bindingContext = new BindingContext(env);
    bindingContext->plugin_handle_ = plugin_handle_;
    bindingContext->env = env;
    bindingContext->callback = new ThreadSafeCallback(info[1].As<Function>());
    bindingContext->garment_handles.push_back(garment_handle);
    bindingContext->garment_json = garment_json;    
    params.callback.data_passthrough_ptr = bindingContext;

    params.callback.func_ptr = [](CallbackData data){
        if( !data.data_passthrough )
            return;
        BindingContext& bindingContext = *reinterpret_cast<BindingContext*>(data.data_passthrough);
        ARCSim::SharedLibrary::HandleType& plugin_handle_ = bindingContext.plugin_handle_;
        Napi::Env env = bindingContext.env;
        ThreadSafeCallback& js_callback = *(bindingContext.callback);
        // Check status
        if( !( data.type == CT_Finished || data.type == CT_Error ) )
            return;

        const char* error_msg = nullptr;
        if( data.type == CT_Error )
            GetFunctionNoReturn(get_error_message, data.session_handle, error_msg);

        ARCSim::SceneT fb_scene;
        if( data.type == CT_Finished ){            
            // Fetch the current mesh...
            Geometry::Blob blob;
            BinBlob* garment_data;
            GetFunctionNoReturn(get_garment_mesh,
                                data.session_status.handle,
                                bindingContext.garment_handles[0],
                                &garment_data,
                                false
                                );
            blob.Load( *garment_data );
            GetFunctionNoReturn(free_garment_mesh, garment_data);

            std::vector< std::array< float, 2 > > vertices_2d = blob.Get2DVertices();
            std::vector< std::array< float, 3 > > vertices_3d;
            vertices_3d.resize( vertices_2d.size() );
            for( int v = 0; v < vertices_2d.size(); ++v)
                vertices_3d.at(v) = { vertices_2d.at(v)[0], vertices_2d.at(v)[1], 0.0 };
            blob.Set3DVertices(vertices_3d);                                
            
            fb_scene.garments.emplace_back( std::make_unique<ARCSim::GarmentT>() );
            try{
                ARCSimTranslation::ConvertToFB( blob, bindingContext.garment_json, *(fb_scene.garments.at(0)) );
                ARCSimTranslation::ConvertToFB( blob, bindingContext.garment_json, fb_scene.constraints );
            }
            catch( std::exception& err ){
                Napi::Error::New(env, std::string("Failed to convert garment: ")+err.what() ).ThrowAsJavaScriptException();
                return;
            }
        }
        
        auto bytes = PackToBytestream( &fb_scene, ARCSim::SceneIdentifier() );                
        js_callback.call([data, bytes, error_msg](Napi::Env env, std::vector<napi_value>& args)
        {
            Napi::Uint8Array js_byte_array = Napi::Uint8Array::New(env, bytes.second);
            memcpy( js_byte_array.Data(), bytes.first, bytes.second);


            if(error_msg)
                args = { js_byte_array, Napi::String::New(env, error_msg) };
            else
                args = { js_byte_array };
        });
    };

    GetFunction(prepare_meshing, session_handle, &params);
    GetFunction(start_session, session_handle);
    
}



Napi::Function ArcsimBinding::GetClass(Napi::Env env) {
    return DefineClass(env, "ArcsimBinding", {
            ArcsimBinding::InstanceMethod("version", &ArcsimBinding::Version),
                ArcsimBinding::InstanceMethod("create_session", &ArcsimBinding::CreateSimulationSession),
                ArcsimBinding::InstanceMethod("destroy_session", &ArcsimBinding::DestroySimulationSession),
                ArcsimBinding::InstanceMethod("add_obstacle", &ArcsimBinding::AddObstacle),
                ArcsimBinding::InstanceMethod("add_garment", &ArcsimBinding::AddGarment),
                ArcsimBinding::InstanceMethod("start_sim", &ArcsimBinding::StartSimulation),
                ArcsimBinding::InstanceMethod("pause_sim", &ArcsimBinding::PauseSimulation),
                ArcsimBinding::InstanceMethod("generate_mesh", &ArcsimBinding::GenerateMesh)
    });
}
