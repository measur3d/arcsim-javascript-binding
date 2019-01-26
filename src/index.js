const arcsim_native = require('bindings')('arcsim-binding-native')

function ArcsimTranslator() {
    this.convert_legacy_arcsim_scene = function(garment_bin_filename,
                                                garment_json_filename,
                                                obstacle_bin_filename,
                                                obstacle_json_filename) {
        return new Promise((resolve, reject) => {
            try{
                resolve(_addonInstance.convert_legacy_arcsim_scene(
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

    this.convert_legacy_garment = function(garment_bin_filename,
                                           garment_json_filename) {
        return new Promise((resolve, reject) => {
            try{
                resolve(_addonInstance.convert_legacy_garment(
                    garment_bin_filename,
                    garment_json_filename
                ));
            }
            catch( error ){
                reject(error);
            }
        });
    }

    this.convert_legacy_obstacle = function(obstacle_bin_filename,
                                            obstacle_json_filename) {
        return new Promise((resolve, reject) => {
            try{
                resolve(_addonInstance.convert_legacy_obstacle(
                    obstacle_bin_filename,
                    obstacle_json_filename
                ));
            }
            catch( error ){
                reject(error);
            }
        });
    }

    var _addonInstance = new arcsim_native.ArcsimTranslator();
}

function ArcsimBinding(name) {
    this.version = function() {
        return new Promise((resolve, reject) => {
            try{
                resolve(_addonInstance.version());
            }
            catch( error ){
                reject(error);
            }
        });
    }

    this.create_session = function(config) {
        return new Promise((resolve, reject) => {
            try{
                resolve(_addonInstance.create_session(config));
            }
            catch( error ){
                reject(error);
            }
        });
    }
    
    this.destroy_session = function(session_handle) {
        return new Promise((resolve, reject) => {
            try{
                resolve(_addonInstance.destroy_session(session_handle));
            }
            catch( error ){
                reject(error);
            }
        });
    }
    
    this.add_obstacle = function(session_handle, obstacle_data) {
        return new Promise((resolve, reject) => {
            try{
                resolve(_addonInstance.add_obstacle(session_handle, obstacle_data));
            }
            catch( error ){
                reject(error);
            }
        });
    }

    this.add_garment = function(session_handle, garment_data) {
        return new Promise((resolve, reject) => {
            try{
                resolve(_addonInstance.add_garment(session_handle,garment_data));
            }
            catch( error ){
                reject(error);
            }
        });
    }
    
    this.start_sim = function(session_handle) {
        return new Promise((resolve, reject) => {
            try{
                resolve(_addonInstance.start_sim(session_handle));
            }
            catch( error ){
                reject(error);
            }
        });
    }

    this.pause_sim = function(session_handle) {
        return new Promise((resolve, reject) => {
            try{
                resolve(_addonInstance.pause_sim(session_handle));
            }
            catch( error ){
                reject(error);
            }
        });
    }
        
    var _addonInstance = new arcsim_native.ArcsimBinding(name);
}

export { ArcsimTranslator, ArcsimBinding };
