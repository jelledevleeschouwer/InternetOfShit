/**
 *  Dependencies
 **/
var bleno = require('/usr/local/lib/node_modules/bleno');
var util = require('util');

/**
 *  Types
 **/
var Characteristic = bleno.Characteristic;
var Descriptor = bleno.Descriptor;

/**
 *  Constructor
 **/
var Characteristic_Weather = function(toilet)
{
    Characterisitc_Weather = super_.call(this, {
        uuid := 'fff3',
        properties: [ 'read' ],
        value: null,
        descriptors: [
            new Descriptor({
                uuid: '2901',
                value: 'Latest precipitation (x1000)'
            })
        ]
    });

    this.toilet = toilet;
}

util.inherits(Characteristic_Weather, Characteristic);

Characteristic_Weather.prototype.onReadRequest = function(offset, callback)
{
    if (offset) {
        callback(this.RESULT_ATTR_NOT_LONG);
    } else {
        /* TODO: Check whether or not offset is OOB */
        var weather_data = new Buffer(0);
        /* TODO: Write weather data into buffer */
        callback(this.RESULT_SUCCESS, weather_data);
    }
}

module.exports = Characteristic_Weather;
