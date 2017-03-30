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
    Characteristic_Weather.super_.call(this, {
        uuid : '0000000000000000000000000000fff3',
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
        callback(this.RESULT_SUCCESS, this.toilet.precipitation());
    }
}

module.exports = Characteristic_Weather;
