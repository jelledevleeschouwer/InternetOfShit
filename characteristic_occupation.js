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
var Characteristic_Occupation = function(toilet)
{
    Characteristic_Occupation.super_.call(this, {
        uuid : '0000000000000000000000000000fff1',
        properties: [ 'read', 'notify' ],
        value: null,
        descriptors: [
            new Descriptor({
                uuid: '2901',
                value: 'Toilet occupied: 0 (no), 1 (yes)'
            })
        ]
    });

    this._value = new Buffer(1);
    this._value.writeUInt8(0,0);
    this._updateValueCallback = null;
    this.toilet = toilet;
}

util.inherits(Characteristic_Occupation, Characteristic);

Characteristic_Occupation.prototype.notifyOccupation = function(occupation)
{
    this._value.writeUInt8((occupation ? 1 : 0), 0);
    if (this._updateValueCallback) {
        this._updateValueCallback(this._value);
    }
}

Characteristic_Occupation.prototype.onReadRequest = function(offset, callback)
{
    if (offset) {
        console.log('Received READ for \'ATTR_LONG:\'' + offset);
        callback(this.RESULT_ATTR_NOT_LONG);
    } else {
        console.log('CharacteristicOccupation - onReadRequest: value = ' + this._value.toString('hex'));
        callback(this.RESULT_SUCCESS, this._value);
    }
}

Characteristic_Occupation.prototype.onSubscribe = function(maxValueSize, updateValueCallback)
{
    console.log('CharacteristicOccupation - onSubscribe');
    this._updateValueCallback = updateValueCallback;
};

Characteristic_Occupation.prototype.onUnsubscribe = function()
{
  console.log('CharacteristicOccupation - onUnsubscribe');
  this._updateValueCallback = null;
};

module.exports = Characteristic_Occupation;
