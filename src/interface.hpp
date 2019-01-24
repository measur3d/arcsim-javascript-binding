#ifndef ARCSIM_INTERFACE
#define ARCSIM_INTERFACE

#include <stdint.h>

// V0.3

#if defined(_WIN32) && !defined(ARCSIM_STATIC_LINKING)
    #ifdef EXPORT_DLL
        #define EXPOSED_API __declspec(dllexport)
    #else
        #ifndef STATIC_LINK
            #define EXPOSED_API __declspec(dllimport)
        #else
            #define EXPOSED_API
        #endif
    #endif
#else
    #define EXPOSED_API
#endif

// API Constants

#define INTERFACE_API_BINARY_VERSION_MAJOR          1
#define INTERFACE_API_BINARY_VERSION_MINOR          1
#define INTERFACE_API_BINARY_VERSION_PATCH          0

#define INTERFACE_API_MAX_BUILD_INFO_LENGTH         32

// Typedefs

typedef uint32_t ErrorCode;

// Enumerations

enum ErrorCodes
{
    //! Successful result
    ARC_OK = 0,
    //! Failure inside engine, use error_message to review
    ARC_InternalError = 1,
    //! Invalid engine version requested for new session
    ARC_InvalidEngineVersion = 10000,
    //! Invalid session requested from api
    ARC_InvalidSessionHandle,
    //! Invalid garment requested from api
    ARC_InvalidGarmentHandle,
    //! Invalid obstacle (body) requested from the api
    ARC_InvalidObstacleHandle,
    //! Invalid constraint requested from the api
    ARC_InvalidConstraintHandle,
    //! Invalid filename (empty, invalid path, etc..)
    ARC_InvalidFilename,
    //! Filetype unsupported by the operation
    ARC_InvalidFileFormat,
    //! Data passed into the api was null
    ARC_NullData,
    //! Operation requires a different session type than was requested
    ARC_InvalidSessionType,
    //! Can't perform the operation because session is currently running
    ARC_SessionRunning,
    //! Can't perform the operation because session has been completed and closed
    ARC_SessionCompleted,
    //! Can't perform the operation because session has entered a failed state
    ARC_SessionInFailure,
    //! Can't perform the operation because session has not yet been initialized
    ARC_SessionUninitialized,
    //! Can't perform the operation because session has already been initialized
    ARC_SessionInitialized,
    //! Can't perform the operation because no garment has been set for the session
    ARC_SessionMissingGarment,
    //! Can't perform the operation because no obstacle (body) has been set for the session
    ARC_SessionMissingObstacle,
    //! JSON data failed to validate schema
    ARC_InvalidJSON,
    //! Bin file failed to validate schema
    ARC_InvalidBin,
    //! Operation failed
    ARC_InvalidRequest,
    //! Operation failed due to badly formed argument
    ARC_InvalidArgument,
    //! Operation failed because parameter was outside acceptable value bounds
    ARC_ParameterOutOfBounds,
    //! Unknown error
    ARC_Unknown
};


enum ErrorCodesInternal
{
    //! Concave split during remeshing
    ARC_ECI_ConcaveSplit = 5010,
    ARC_ECI_NumericalOpFailure = 3010,
    ARC_ECI_CurveOpFailure = 3012,
    //! Intersection error
    ARC_ECC_CollisionError = 8023
};


// session type enumerations
enum SessionType
{
    ST_Simulation = 0,
    ST_Stitching,
    ST_Meshing,
};

// session state enumerations
enum SessionState
{
    SS_Uninitialized = 0,
    SS_NotStarted,
    SS_Running,
    SS_Paused,
    SS_Completed,
    SS_Error
};

// callback stage enumerations
enum CallbackType
{
    CT_Initialize = 0,
    CT_CollisionStep,
    CT_OptimizationStep,
    CT_RemeshingStep,
    CT_SimulationFrame,
    CT_Paused,
    CT_Finished,
    CT_Error,
};

// handle (constraint) type enumerations
enum HandleType
{
    HT_Invalid,
    HT_Node,
    HT_Elastic,
    HT_Pin,
    HT_Loop,
    HT_Button,
    HT_BaryButton,
    HT_Collar,
    HT_OrientedButton,
    HT_Barrier,
    HT_Force
};


// logging verbosity levels
enum LogVerbosity
{
    // Never log at this level
    LOG_Verbosity_OFF     = -9, 
    
    // Decreasing in severity
    LOG_Verbosity_FATAL   = -3,
    LOG_Verbosity_ERROR   = -2,
    LOG_Verbosity_WARNING = -1,
    
    // Normal messages. 
    LOG_Verbosity_INFO    =  0,
    
    // Same as Verbosity_INFO in every way.
    LOG_Verbosity_0       =  0,
    
    // Verbosity levels - higher levels indicate more detailed and pedantic logging
    LOG_Verbosity_1       = +1,
    LOG_Verbosity_2       = +2,
    LOG_Verbosity_3       = +3,
    LOG_Verbosity_4       = +4,
    LOG_Verbosity_5       = +5,
    LOG_Verbosity_6       = +6,
    LOG_Verbosity_7       = +7,
    LOG_Verbosity_8       = +8,
    LOG_Verbosity_9       = +9,
    
    // Log everything
    LOG_Verbosity_MAX     = +9,
};

    
// Configuration Parameters


// Return status structure

struct SessionStatus
{
    int handle;
    SessionType type;
    SessionState state;

    int frame;
    int steps;
    double time;
};

struct CallbackData
{
    CallbackType type;
    int session_handle;
    SessionStatus session_status;
    void* data_passthrough;
};


// Asset data structures

struct BinBlob
{
    uint64_t len;
    const char* buffer;
};

struct InterchangeData
{
    uint64_t len;
    const uint8_t* buffer;
};


struct ErrorData
{
    int handle;
    ErrorCode code;
    const char* code_str;
    const char* description;
    InterchangeData data;
};


// Exchange types

typedef void callback_func(const volatile CallbackData data);

struct CallbackHook
{
    callback_func *func_ptr;
    void *data_passthrough_ptr;
};

// generic math containers
struct Vector2
{
    double x;
    double y;
};

struct Vector3
{
    double x;
    double y;
    double z;
};

// generic handle parameters
struct HandleParams
{
    // These parameters are output only
    HandleType type;
    bool active;
    
    // These parameters are input/output
    bool enabled;
    double stiffness;
    uint32_t start_frame;
    uint32_t end_frame;
};

// handle location data (for visualization purposes)
struct HandleLocations
{
    // Number of discrete points this handle "occupies"
    Vector3* center;
    Vector3* orientation;    
    
    // Worldspace locations (XYZ triplet in memory * number of points)
    uint32_t worldspace_num_points;
    Vector3* worldspace_points;
    
    // Garment material space locations (UV pair in memory * number of points)
    uint32_t materialspace_num_points;
    Vector2* materialspace_points;
};

// response from api logging events
struct LogMessage
{
    LogVerbosity   verbosity;   // Verbosity of this log message
    const char* filename;    // Internal filename that originated logging
    unsigned    line;        // Internal linenumber that originated logging
    
    // You would generally print a LogMessage by just concating the buffers without spacing.
    // Optionally, ignore preamble and indentation.    
    const char* preamble;    // Date, time, uptime, thread, file:line, verbosity.
    const char* indentation; // Spacing for aligning log messages
    const char* prefix;      // Assertion failure info goes here (or "").
    const char* message;     // Log message goes here.
};

// Callback type to receive log message events
typedef void (*log_handler_t)(void* user_data, const LogMessage* message);

// Callback type to receive log close events
typedef void (*close_handler_t)(void* user_data);

// Callback type to receive log flush events
typedef void (*flush_handler_t)(void* user_data);

enum SimParamPresets
{
    SPP_Draft = 0,
    SPP_Normal = 1,
    SPP_High = 2
};

// _sim specific parameters
struct SimParams
{

    //
    // General Simulation Parameters
    // 
    
    // Simulate for no more than max_frames
    uint32_t max_frames;

    // Perform a upsampling of cloth mesh topology after simulation completes
    uint8_t final_upsample;


    //
    // Collision Parameters
    //

    /* Global toggle for all collision handling / intersection removal during simulation. */
    uint8_t enable_collisions;

    /* Physical collision response mode. Bitwise flags are as follows:
     * 1 = SDF - Cloth-obstacle collision constraints, generated using signed-distance fields
     * 2 = Repulsion - Proximity-based repulsion constraints  
     * (Enumeration of mode values: 0 = No collision constraints, 1 = SDF only, 2 = Repulsion only, 3 = Both */
    uint32_t collisions_physical_mode;

    /* SDF collision constraints stiffness.
     * Controls the relative strength of SDF constraints */
    double collisions_physical_sdf_stiffness;

    /* Proximity-based repulsion collision constraint stiffness. 
     * Controls the relative strength of proximity-based repulsion constraints.
     * Can be set stiffer if a sim results in a highly tangled configuration. */
    double collisions_physical_repulsion_stiffness;

    /* Proximity-based repulsion collision thickness.
     * Controls the range of influence of proximity-based repulsion constraints */
    double collisions_physical_repulsion_thickness;

    /* Geometric collisions failsafe mode.
     * Controls how intersections are resolved when they persist beyond the constrained physics solve.
     * Bitwise flags are as follows:
     * 1 = LocalProjectOut - Locally project meshes into non-colliding state
     * 2 = ICM - Intersection Contour Minimization, http://www.miralab.ch/bibliography/167.pdf
     * 4 = Harmon - Harmon et al. 2008. Continuous collision detection and inelastic projection. */
    uint32_t collisions_geometric_mode;

    /* How often should geometric collision handling be performed?
     * This is the number of frames between applications of the intersection removal algorithm(s).
     * e.g., a value of 2 means every-other frame intersections are removed after the physics step. */
    uint32_t collisions_geometric_frequency;

    /* Obstacle awareness during inters ection removal. If this is disabled then only cloth-cloth intersections are removed */
    uint8_t collisions_geometric_obstacle_aware; 

    /* Maximum iterations allowed for a single local projection operation during intersection removal */
    uint32_t collisions_geometric_local_project_max_iter;

    /* Additional offset distance above the body's surface to which elements should be projected during intersection removal */
    double collisions_geometric_local_project_offset;

    /* Toggle for doing a hard SDF projection on nodes when using the local projection scheme for
     * removing intersections after the physics step. */
    uint8_t collisions_geometric_local_project_hard_nodes;
    
    /* Size of an optimization step when using the ICM intersection removal algorithm. */
    double collisions_geometric_icm_max_step;

    /* Max number of iterations allowed when using the ICM intersection removal algorithm */
    uint32_t collisions_geometric_icm_max_iter; 

    /* The projection thickness when using the Harmon (2008) collision resolution algorithm */
    double collisions_geometric_harmon_projection_thickness; 

    /* The maximum number of iterations allowed for the Harmon algorithm */
    uint32_t collisions_geometric_harmon_max_iter;



    //
    // Physics - General Parameters
    // 

    // Toggle which controls whether or not physics steps are carried out
    uint8_t enable_physics;

    // If true, the legacy solver is used for the physics update. Otherwise Newton.
    uint8_t physics_solver_legacy;

    // Controls the amount of friction between the cloth and itself
    double friction;

    // Controls the amount of friction between the cloth and solid obstacles
    double obs_friction;

    // Controls the direction and strength of gravity in the Z-axis
    double gravity;



    //
    // Physics - Wind Parameters
    //

    // Toggle for computing wind forces during the physics step
    uint8_t enable_wind; 

    // Density of wind, controls relative strength
    double wind_density;

    // Velocity of wind - direction and speed
    Vector3 wind_velocity;

    // Air resistance drag parameter
    double wind_drag;



    //
    // Remeshing Parameters
    //

    // Disable/Enable both static and dynamic remeshing
    uint8_t should_remesh;

    // Minimum triangle size remeshing allows
    double remeshing_min_triangle_area;

    // Maximum triangle size remeshing allows
    double remeshing_max_triangle_area;

    // Max size of boundary edge relative to boundary curvature
    double remeshing_2dcurve_refinement;
    

    
    //
    // Initial Relaxation Parameters
    //

    // Enables/Disable the initial relaxation optimization
    uint8_t enable_initial_relaxation;

    // Use original shell optimization or local shell optimization
    uint8_t use_local_shell_optimization;

    // Whether use the obstacle information in collision handling or not
    //////uint8_t obstacle_aware_collision_handling;
    // Enables static friction effects during the initial relaxation process for improved cloth/body friction behavior 
    uint8_t initial_relaxation_enable_static_friction;   

    // Controls the shell distance between garments for multi-garment relaxation
    double initial_relaxation_layer_size;

    // Controls the inner-shell target distance for a garment in multi-garment relaxation
    double initial_relaxation_layer_dist;

    // Controls the minimum separation distance away from the obstacle
    double initial_relaxation_min_sdf_distance;

    // Controls the static remeshing resolution used during initial relaxation
    double initial_relaxation_mesh_resolution;

    // Disables/Enables the usage of the simple isotropic material during initital relaxation,
    // If disabled, the normal material is used throughout initial relaxation
    uint8_t initial_relaxation_use_isotropic_material;

    // (Isotropic Material Only) Sets the density of the initial_isotropic material
    double initial_relaxation_isotropic_density;

    // (Isotropic Material Only) Sets the stretching coefficient of the initial_isotropic material
    // Larger Values = More Resistance to Stretching
    // Smaller Values = Less Resistance to Stretching
    double initial_relaxation_isotropic_stretching;

    // (Isotropic Material Only) Sets the bending coefficient of the initial_isotropic material
    // Larger Values = More Resistance to bending (flatter results)
    // Smaller Values = Less Resistance to bending (curvier results)
    double initial_relaxation_isotropic_bending;

    // Strength of the body-cloth repulsion during initial relaxation
    double initial_relaxation_sdf_stiffness;

    // Maximum number of iterations allowed per shell phase
    uint32_t initial_relaxation_max_shell_iterations;

    // Maximum number of iterations allowed in the first deflate phase
    uint32_t initial_relaxation_max_deflate_phase1_iterations;

    // Maximum number of iterations allowed in the second deflate phase
    uint32_t initial_relaxation_max_deflate_phase2_iterations;

    // Emit updated geometry for all optimization steps ( otherwise, just between phases )
    uint8_t initial_relaxation_verbose_iterations;



    //
    // Finalize Stage Parameters
    //

    /* Strength of gravity during the finalize stage of the simulation */
    double finalize_gravity;

    /* Maximum allowed number of iterations during optimization in the finalize stage */
    uint32_t finalize_optimization_max_iter;

    /* Maximum optimization step size during the finalize stage */
    double finalize_optimization_max_step;

    /* Relative strength of SDF collision constraints during optimization in the finalize stage */
    double finalize_collisions_physical_sdf_stiffness;

    /* The number of steps between intersection removal passes during optimization in the finalize stage.
     * e.g., a value of 2 means only do intersection removal every-other step */
    uint32_t finalize_collisions_geometric_frequency;

    /* Maximum allowed iterations for a local project operation during intersection removal in the finalize stage */
    uint32_t finalize_collisions_geometric_local_project_max_iter;

    /* Amount by which to project intersecting elements above the surface of a colliding body 
     * when performing local projection during the finalize stage */
    double finalize_collisions_geometric_local_project_offset;

    /* Toggle for doing a hard SDF projection on garment nodes when using the local projection scheme for
     * removing intersections during the finalize stage */
    uint8_t finalize_collisions_geometric_local_project_hard_nodes;

    /* The projection thickness when using the Harmon (2008) collision resolution algorithm
     * to remove intersections during the finalize stage. */
    double finalize_collisions_geometric_harmon_projection_thickness;

    /* The maximum number of iterations allowed for the Harmon algorithm during the finalize stage */
    uint32_t finalize_collisions_geometric_harmon_max_iter;

    //
    // Callback Setup
    //
    
    // callback hook;
    CallbackHook callback;

};

// stitching specific parameters
struct StitchingParams
{
    // callback hook;
    CallbackHook callback;
};

// meshing specific parameters
struct MeshingParams
{
    //
    // Remeshing and Triangulation Parameters
    //

    // enforcing edge length
    float edge_length;
    // subdivision curvature deviation threshold
    float max_deviation;
    // minimum edge subdivision
    float min_subdiv;


    // callback hook;
    CallbackHook callback;
};

// Mesh Post-Processing Flags
enum MeshPostProcessing {
    MPP_Extrude = 1,
    MPP_Decimate = 2
};

// Mesh Post-Processing Parameters
struct PostProcessingParams 
{
    uint32_t action; // Multiple MeshPostProcessing flags may be combined here

    // Extrude Parameters
    float extrude_projection_thickness;
    uint8_t extrude_only_outwards;    
    
    // Decimate Parameters
    uint32_t decimate_target_triangle_count;    
};

// Physics metadata flags
enum PhysicsMetadataFlags {
    PMD_None = 0,
    PMD_Strain = 0x0001,
    PMD_Curvature = 0x0002,
    PMD_StretchEnergy = 0x0004,
    PMD_BendEnergy = 0x0008,
    PMD_Flag = 0x0010,
    PMD_CollisionDistance = 0x0020,
    // PMD_Curvature2D = 0x0040,
    // PMD_Marked = 0x0080,
    PMD_All = 0x003F,  // Curvature2D and Marked are unused
    PMD_Default = PMD_Strain | PMD_Curvature | PMD_StretchEnergy | PMD_BendEnergy  // Gerber defaults
};


// Shared library import/export declarations

extern "C"
{
    // utility functions

    //! Queries the library to retrieve the arcsim version.
    //! Any parameter can be passed NULL to skip it. 'build' is assumed to be
    //! a pre-allocated buffer of at least 32 bytes long. It will be filled with
    //! a NULL-terminated string indicating the build is debug or release.
    /*!
      \sa arcsim_version()
      \param major - returns the major arcsim version.
      \param minor - returns the minor arcsim version.
      \param patch - returns the patch arcsim version.
      \param build - returns arcsim build number.
      \return library status error code
    */
    EXPOSED_API ErrorCode arcsim_version(unsigned int* major, unsigned int* minor, unsigned int* patch, char* build);

    //! Queries the library to retrieve the API version.
    /*!
      \sa api_version()
      \param major - returns the major arcsim api version.
      \param minor - returns the minor arcsim api version.
      \return library status error code
    */
    EXPOSED_API ErrorCode api_version(unsigned int* major, unsigned int* minor);


    //! Provides hooks into the internal logging system to recover information for debugging. This is global, not per session.
    /*!
     * \sa api_log_callback()
     * \param log_callback - function pointer for handling log messages. Set to NULL to cancel current callbacks and disable logging. 
     * \param user_data - arbitrary user data passed to the log handlers
     * \param verbosity - filters log messages above the provided verbosity limit
     * \param on_close - called when the log system should handle close events (for files or network logging sinks)
     * \param on_flush - called when the log system should flush to the sink (for files or network logging sinks)
     * \return library status error code
    */
    EXPOSED_API ErrorCode api_log_callback(log_handler_t log_callback, void* user_data,
                                           LogVerbosity verbosity, close_handler_t on_close = 0,
                                           flush_handler_t on_flush = 0);
    
    
    // session management

    //! Requests a new processing session. Sessions can be of type Simulation or Stitching.
    /*!
      \sa create_session(), SessionType
      \param version_major - api major version.
      \param version_minor - api minor version.
      \param session_type - input sim session type.
      \param out_id - returns an assigned sim session ID.
      \return library status error code
    */
    EXPOSED_API ErrorCode create_session(int version_major, int version_minor, SessionType session_type, int* out_id);

    //! Destroys a processing session from a sim handle ID.
    /*!
      \sa destroy_session()
      \param session_handle - sim handle ID.
      \return library status error code
    */
    EXPOSED_API ErrorCode destroy_session(int session_id);


    // error handling using ErrorData

    //! Retrieves the optional data attached to an ErrorCode
    /*!
     \sa get_last_error()
     \param data - returns the data associated to the last error raised
     \return library status error code
    */
    EXPOSED_API ErrorCode get_last_error(ErrorData** data);


    //! Returns the ID for a friendly error message for an Error Code
    /*!
    \sa get_error_message_id()
    \param error_message_id - ptr to error message
    \return library status error code
    */
    EXPOSED_API ErrorCode get_error_message(ErrorCode code, const char** error_message);


    //! Releases error data allocated memory.
    /*!
      \sa free_garment_mesh(), BinBlob
      \param data - error data pointer.
      \return library status error code
    */
    EXPOSED_API ErrorCode free_error(ErrorData* data);


    // assets management using BinBlob

    //! Validates a garment json string and an associated binary blob.
    /*!
      \sa validate_garment(), BinBlob
      \param json - json formatted string (\0 terminated) containing the garment definition.
      \param bin - garment binary blob mesh to be validated.
      \return library status error code
    */
    EXPOSED_API ErrorCode validate_garment(const char *json, BinBlob *bin);

    //! Validates a garment json file and, optionally, an associated binary file.
    /*!
      \sa validate_garment_from_file()
      \param json_file - file path to a garment json file definition (string \0 terminated).
      \param bin_file - file path to a garment binary file (string \0 terminated).
      \return library status error code
    */
    EXPOSED_API ErrorCode validate_garment_from_file(const char* json_file, const char* bin_file);

    //! Validates a body json string and an associated binary blob.
    /*!
      \sa validate_body(), BinBlob
      \param json - json formatted string (\0 terminated) containing the body definition.
      \param bin - body binary blob mesh to be validated.
      \return library status error code
    */
    EXPOSED_API ErrorCode validate_body(const char *json, BinBlob *bin);

    //! Validates a body json file and, optionally, an associated binary file.
    /*!
      \sa validate_body_from_file()
      \param json_file - file path to a body json file definition (string \0 terminated).
      \param bin_file - file path to a body binary file (string \0 terminated).
      \return library status error code
    */
    EXPOSED_API ErrorCode validate_body_from_file(const char* json_file, const char* bin_file);



    //! Adds a garment json string and an associated binary blob to an active sim session.
    /*!
      
      Adds a garment json string and an associated binary blob to an
      active sim session. Upon return, a handle to the added garment
      will be created. Additionally, handles for handles in the
      garment json will be created implicitly, starting from the last
      added handle. This value can be retrieved by calling \ref
      get_last_handle_id().
      
      \sa add_garment(), BinBlob
      \param session_id - sim valid handle ID
      \param name - custom name string for the garment (must be \0 terminated)
      \param json - json formatted string (\0 terminated) containing the garment definition.
      \param bin - garment binary blob mesh.
      \param out_id - returns an assigned garment handle ID.
      \return library status error code
    */
    EXPOSED_API ErrorCode add_garment(int session_id, const char* name, const char* json, BinBlob* bin, int* out_id);

    //! Adds a garment json file and an associated binary file to an active sim session.
    /*!

      Adds a garment from a json filename and an associated binary
      blob filename to an active sim session. Upon return, a handle to
      the added garment will be created. Additionally, handles for
      handles in the garment json will be created implicitly, starting
      from the last added handle. This value can be retrieved by
      calling \ref get_last_handle_id().

      \sa add_garment_from_file()
      \param session_id - sim valid handle ID
      \param json_file - file path to a garment json file definition (string \0 terminated).
      \param bin_file - file path to a garment binary file (string \0 terminated).
      \param out_id - returns an assigned garment handle ID.
      \return library status error code
    */
    EXPOSED_API ErrorCode add_garment_from_file(int session_id, const char* json_file, const char* bin_file, int* out_id);


    //! Adds a new handle into an existing, active sim session. Sim session must have been started, and paused before calling
    /*!
      \sa add_handle()
      \param session_id - sim valid handle ID
      \param garment_handle - garment valid handle ID
      \param json - json text representing the new handle to be added
      \param out_id - handle pointing to newly created handle
      \return library status error code
    */
    EXPOSED_API ErrorCode add_handle(int session_id, int garment_id, const char* json, int* out_id);

    //! Removes an existing handle from an active sim session. Sim session must have been started and paused before calling
    //! Note: only used to pass in simulation session
    /*!
      \sa remove_handle()
      \param session_id - sim valid handle ID
      \param handle_id - handle valid handle ID
      \return library status error code
    */
    EXPOSED_API ErrorCode remove_handle(int session_id, int handle_id );

    //! Gets the id to the last added handle, or -1 if there are no active handles
    //! Note: only used to pass in simulation session
    /*!
      \sa get_last_handle_id()
      \param session_id - sim valid handle ID
      \param last_handle_id - integer to be filled with the last added handle id or -1 if no handles exist
      \return library status error code
    */
    EXPOSED_API ErrorCode get_last_handle_id(int session_id, int* last_handle_id);


    //! Sets generic properties on an existing handle
    /*!
      \sa set_handle_properties()
      \param session_id - sim valid handle ID
      \param handle_id - handle valid handle ID
      \param params - pointer to HandleParams struct containing new handle parameters
      \return library status error code
    */
    EXPOSED_API ErrorCode set_handle_properties( int session_id, int handle_id, HandleParams* params);

    //! Retrieves generic properties from an existing handle
    /*!
      \sa get_handle_properties()
      \param session_id - sim valid handle ID
      \param handle_id - handle valid handle ID
      \param params - pointer to HandleParams struct which will be filled with the referenced handle's current parameters
      \return library status error code
    */    
    EXPOSED_API ErrorCode get_handle_properties( int session_id, int handle_id, HandleParams* params);

    //! Retrieves the world and material space locations for the handle
    //! Note: only used to pass in simulation session
    /*!
      \sa get_handle_locations()
      \param session_id - sim valid handle ID
      \param handle_id - handle valid handle ID
      \param locations - pointer to hold a new HandleLocations struct which will be allocated and returned with the
                         location data of the handle
      \return library status error code
    */
    EXPOSED_API ErrorCode get_handle_locations( int session_id, int handle_id, HandleLocations** locations);


    //! Frees previously allocated HandleLocations struct
    /*!
      \sa free_handle_locations()
      \param locations - pointer to HandleLocations to be freed
      \return library_status_error_code
    */
    EXPOSED_API ErrorCode free_handle_locations( HandleLocations* locations);

    //! Adds an obstacle json string and an associated binary blob to an active sim session.
    /*!
      \sa add_obstacle(), BinBlob
      \param session_id - sim valid handle ID
      \param json - json formatted string (\0 terminated) containing the obstacle definition.
      \param bin - obstacle binary blob mesh.
      \param out_id - returns an assigned obstacle handle ID.
      \return library status error code
    */
    EXPOSED_API ErrorCode add_obstacle(int session_id, const char *json, BinBlob *bin, int* out_id);

    //! Adds an obstacle json string and an associated binary blob to an active sim session.
    /*!
      \sa add_obstacle_from_file(), BinBlob
      \param session_id - sim valid handle ID
      \param json_file - file path to a garment json file definition (string \0 terminated).
      \param bin_file - file path to a garment binary file (string \0 terminated).
      \param out_id - returns an assigned obstacle handle ID.
      \return library status error code
    */
    EXPOSED_API ErrorCode add_obstacle_from_file(int session_id, const char* json_file, const char* bin_file, int* out_id);


    //! Sets an obstacle binary blob to an active sim session and a frame number.
    /*!
      \sa add_obstacle_frame(), BinBlob
      \param session_id - sim valid handle ID
      \param obstacle_id - obstacle handle ID.
      \param frame - frame number.
      \param bin - obstacle binary blob mesh.
      \return library status error code
    */
    EXPOSED_API ErrorCode add_obstacle_frame(int session_id, int obstacle_id, int frame, BinBlob *bin);

    //! Adds an obstacle json string and an associated binary blob to an active sim session.
    /*!
      \sa add_obstacle_frame_from_file(), BinBlob
      \param session_id - sim valid handle ID
      \param obstacle_id - obstacle handle ID.
      \param frame - frame number.
      \param bin_file - file path to a garment binary file (string \0 terminated).
      \return library status error code
    */
    EXPOSED_API ErrorCode add_obstacle_frame_from_file(int session_id, int obstacle_id, int frame, const char* bin_file);


    //! Retrieves the last generated garment cache stored in the binary blob mesh structure.
    /*!
      \sa get_garment_mesh(), BinBlob
      \param session_id - valid sim handle ID
      \param garment_id - valid garment handle ID
      \param out_mesh - output mesh in binary blob format (data structure will be allocated and returned).
      \param export_sim_metadata - include geometric simulation data (strain, stretch_energy, etc..) in mesh blob
      \return library status error code
    */
    EXPOSED_API ErrorCode get_obstacle_mesh(int session_id, int obstacle_id, BinBlob** out_mesh);
    
    //! Retrieves the last generated garment cache stored in the binary blob mesh structure.
    /*!
      \sa get_garment_mesh(), BinBlob
      \param session_id - valid sim handle ID
      \param garment_id - valid garment handle ID
      \param out_mesh - output mesh in binary blob format (data structure will be allocated and returned).
      \param physics_metadata_flags - include geometric simulation data (strain, stretch_energy, etc..) in mesh blob
      \return library status error code
    */
    EXPOSED_API ErrorCode get_garment_mesh(int session_id, int garment_id, BinBlob** out_mesh, uint32_t physics_data_flags);

    //! Releases binary blob allocated memory.
    /*!
      \sa free_garment_mesh(), BinBlob
      \param mesh - binary blob pointer.
      \return library status error code
    */
    EXPOSED_API ErrorCode free_garment_mesh(BinBlob* mesh);


    // assets modification

    //! Saves the last generated mesh (garment cache) on file. Supported file types: [OBJ, BIN].
    /*!
      \sa save_mesh()
      \param session_id - valid sim handle ID
      \param garment_id - valid garment handle ID
      \param out_filename - output file path.
      \return library status error code
    */
    EXPOSED_API ErrorCode save_mesh(int session_id, int garment_id, const char *out_filename);

    //! Saves the bin blob on file. Supported file types: [OBJ, BIN].
    /*!
      \sa save_mesh_from_bin_blob()
      \param bin - binary blob pointer.
      \param out_filename - output file path.
      \return library status error code
    */
    EXPOSED_API ErrorCode save_mesh_from_bin_blob(BinBlob* bin, const char *out_filename);


    //! Applies additional geometric processing to garment meshes and returns a modified copy
    /*!
      \sa post_process_mesh()
      \param session_id - valid sim handle ID
      \param garment_id - valid garment handle ID
      \param out_mesh - output mesh in binary blob format (data structure will be allocated and returned).
      \param params - pointer to PostProcessingParams struct, which controls what operations are performed
      \return library status error code
    */
    EXPOSED_API ErrorCode post_process_mesh(int session_id, int garment_id, BinBlob **out_mesh, PostProcessingParams* params);
    
    
    //! Run finalize and updated the cloth data.  As a result, a big collision optimization step is performed and remeshing in case of upsampling
    /*!
     \sa finalize_mesh()
     \param sim_handle - valid sim handle ID
     \return library status error code
     */
    EXPOSED_API ErrorCode finalize_mesh(int session_id);


    // actions on the simulation

    //! Freezes a piece by name. It may or may not already be frozen.
    //! The piece will remain frozen until unfreeze_piece is called with that piece.
    //! The piece name that is not found will be ignored.
    //! Sim MUST be paused or unstarted or an error will be returned.
    /*!
     \sa freeze_piece()
     \param session_id - valid sim handle ID
     \param piece_names - list of piece names to freeze
     \return library status error code
    */
    EXPOSED_API ErrorCode freeze_piece(int session_id, int garment_id, const char* piece_name);

    //! UNFreezes a piece by name. It may or may not already be frozen.
    //! The piece will remain frozen until freeze_piece is called with that piece.
    //! The piece name that is not found will be ignored.
    //! Sim MUST be paused or unstarted or an error will be returned.
    /*!
     \sa unfreeze_piece()
     \param session_id - valid sim handle ID
     \param piece_names - list of piece names to freeze
     \return library status error code
    */
    EXPOSED_API ErrorCode unfreeze_piece(int session_id, int garment_id, const char* piece_name);

    // simulation-specific

    //! Prepares a simulation session passing simulation paramenters to the engine. This function can be called only on simulation sessions.
    /*!
      \sa prepare_simulation(), SimParams
      \param session_id - valid sim handle ID
      \param params - simulation parameters
      \return library status error code
    */
    EXPOSED_API ErrorCode prepare_simulation(int session_id, SimParams* params);

     //! Sets the simulation parameters to sane defaults
     /*!
      \sa get_default_simulation_parameters(), SimParams
      \param params - simulation parameters to set
      \return library status error code
     */
    EXPOSED_API ErrorCode get_default_simulation_parameters(SimParams *params);

     //! Sets the simulation parameters to a particular quality preset
     /*!
      \sa get_preset_simulation_parameters(), SimParams
      \param params - simulation parameters to set
      \param preset - which quality preset to load
      \return library status error code
     */
    EXPOSED_API ErrorCode get_preset_simulation_parameters(SimParams *params, SimParamPresets preset);
    

    //! Damp the velocities by scaling the velocities by a given scalar.
    /*!
     \sa damp_velocities()
     \param session_id - valid sim handle ID
     \param factor - scaling factor to be applied to the node velocities
     \return library status error code
     */
    EXPOSED_API ErrorCode damp_velocities(int session_id, double factor);
    
    // stitching-specific

    //! Prepares a simulation session passing simulation paramenters to the engine. This function can be called only on stitching sessions.
    /*!
      \sa prepare_stitching(), StitchingParams
      \param session_id - valid sim handle ID
      \param params - stitching parameters
      \return library status error code
    */
    EXPOSED_API ErrorCode prepare_stitching(int session_id, StitchingParams* params);

    //! Sets the stitching parameters to sane defaults
    /*!
     \sa get_default_stitching_parameters(), StitchingParams
     \param params - simulation parameters to set
     \return library status error code
    */
    EXPOSED_API ErrorCode get_default_stitching_parameters(StitchingParams *params);


    // meshing-specific

    //! Generates a triangulated mesh from a garment json definition returning a binary blob.
    /*!
      \sa prepare_meshing(), MeshingParams, BinBlob
      \param session_id - sim handle id ().
      \param params - input meshing parameters.
      \return library status error code
    */
    EXPOSED_API ErrorCode prepare_meshing(int session_id, MeshingParams* params);

    //! Sets the meshing parameters to sane defaults
    /*!
     \sa get_default_meshing_parameters(), MeshingParams
     \param params - simulation parameters to set
     \return library status error code
    */
    EXPOSED_API ErrorCode get_default_meshing_parameters(MeshingParams* params);


    // engine session management

    //! Returns the engine status.
    /*!
      \sa get_session_status(), SimStatus
      \param session_id - valid sim handle ID
      \param out_status - output sim status structure.
      \return library status error code
    */
    EXPOSED_API ErrorCode get_session_status(int session_id, SessionStatus* out_status);

    //! Starts the engine session.
    /*!
      \sa start_session()
      \param session_id - valid sim handle ID
      \return library status error code
    */
    EXPOSED_API ErrorCode start_session(int session_id);


    //! Pauses the engine session.
    //! Only valid during simulation (not meshing, stitching, initial relaxation, or finalize).
    /*!
      \sa pause_session()
      \param session_id - valid sim handle ID
      \return library status error code
    */
    EXPOSED_API ErrorCode pause_session(int session_id);


    //! Resets the engine session.
    /*!
      \sa reset_session()
      \param session_id - valid sim handle ID
      \return library status error code
    */
    EXPOSED_API ErrorCode reset_session(int session_id);

}

#endif // ARCSIM_INTERFACE
