const arcsim = require("../lib/index.js")
let binding = new arcsim.ArcsimBinding("/Users/nathan/Development/ArcSim2/src/build/src/arcsim/libarcsim.dylib")
var parsedJSON = require('./garment.bad.json');
let garment_json = JSON.stringify( parsedJSON );
let result = binding.generate_mesh( garment_json );
result.then( (data, error) => {
    console.log( data );
    console.log(error);
});
