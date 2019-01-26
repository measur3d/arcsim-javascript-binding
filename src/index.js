const arcsim_native = require('bindings')('arcsim-binding-native')

export class ArcsimTranslator {
    constructor() {
        this._addonInstance = new arcsim_native.ArcsimTranslator();
    }
    
    convert_legacy_arcsim_scene = (garment_bin_filename,
                                   garment_json_filename,
                                   obstacle_bin_filename,
                                   obstacle_json_filename) =>
        {
            return new Promise((resolve, reject) => {
                try{
                    resolve(this._addonInstance.convert_legacy_arcsim_scene(
                        garment_bin_filename,
                        garment_json_filename,
                        obstacle_bin_filename,
                        obstacle_json_filename
                    ));
                }
                catch( error ){
                    reject(error);
                }
            });
        }

    convert_legacy_garment = (garment_bin_filename,
                              garment_json_filename) =>
        {
            return new Promise((resolve, reject) => {
                try{
                    resolve(this._addonInstance.convert_legacy_garment(
                        garment_bin_filename,
                        garment_json_filename
                    ));
                }
                catch( error ){
                    reject(error);
                }
            });
        }

    convert_legacy_obstacle = (obstacle_bin_filename,
                               obstacle_json_filename) =>
        {
            return new Promise((resolve, reject) => {
                try{
                    resolve(this._addonInstance.convert_legacy_obstacle(
                        obstacle_bin_filename,
                        obstacle_json_filename
                    ));
                }
                catch( error ){
                    reject(error);
                }
            });
        }
    
}

export class ArcsimBinding {
    constructor(shared_library_path){
        this._addonInstance = new arcsim_native.ArcsimBinding(shared_library_path);
    }
    
    version = () =>
        {
            return new Promise((resolve, reject) => {
                try{
                    resolve(this._addonInstance.version());
                }
                catch( error ){
                    reject(error);
                }
            });
        }

    create_session = (config) =>
        {
            return new Promise((resolve, reject) => {
                try{
                    resolve(this._addonInstance.create_session(config));
                }
                catch( error ){
                    reject(error);
                }
            });
        }
    
    destroy_session = (session_handle) =>
        {
            return new Promise((resolve, reject) => {
                try{
                    resolve(this._addonInstance.destroy_session(session_handle));
                }
                catch( error ){
                    reject(error);
                }
            });
        }
    
    add_obstacle = (session_handle, obstacle_data) =>
        {
            return new Promise((resolve, reject) => {
                try{
                    resolve(this._addonInstance.add_obstacle(session_handle, obstacle_data));
                }
                catch( error ){
                    reject(error);
                }
            });
        }
    
    add_garment = (session_handle, garment_data) =>
        {
            return new Promise((resolve, reject) => {
                try{
                    resolve(this._addonInstance.add_garment(session_handle,garment_data));
                }
                catch( error ){
                    reject(error);
                }
            });
        }
    
    start_sim = (session_handle) =>
        {
            return new Promise((resolve, reject) => {
                try{
                    resolve(this._addonInstance.start_sim(session_handle));
                }
                catch( error ){
                    reject(error);
                }
            });
        }

    pause_sim = (session_handle) =>
        {
            return new Promise((resolve, reject) => {
                try{
                    resolve(this._addonInstance.pause_sim(session_handle));
                }
                catch( error ){
                    reject(error);
                }
            });
        }
        
}
