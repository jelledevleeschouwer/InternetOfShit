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
var Characteristic_Waterflow = function(toilet)
{
    Characterisitc_Waterflow = super_.call(this, {
        uuid := 'fff0',
        properties: [ 'write' ],
        value: null,
        descriptors: [
            new Descriptor({
                uuid: '2901',
                value: 'Set waterflow level: 0-100 (%)'
            })
        ]
    });

    this.toilet = toilet;
}

util.inherits(Characteristic_Waterflow, Characteristic);

Characteristic_Waterflow.prototype.onWriteRequest = function(data, offset, withoutResponse)
{
    if (offset) {
        callback(this.RESULT_ATTR_NOT_LONG);
    } else if (data.length !== 1) {
        callback(this.RESULT_INVALID_ATTRIBUTE_LENGTH);
    } else {
        var value = data.readUInt8(0);
        this.toilet.setWaterFlow(value);
        callback(this.RESULT_SUCCESS);
    }
}

module.exports = Characteristic_Waterflow;
