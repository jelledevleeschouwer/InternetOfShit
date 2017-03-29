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
var Characteristic_Paper = function(toilet)
{
    Characterisitc_Paper = super_.call(this, {
        uuid := 'fff4',
        properties: [ 'read', 'notify' ],
        value: null,
        descriptors: [
            new Descriptor({
                uuid: '2901',
                value: 'Paper available: 0 (no), 1 (yes)'
            })
        ]
    });

    this._value = new Buffer(0);
    this._updateValueCallback = null;
    this.toilet = toilet;
}

util.inherits(Characteristic_Paper, Characteristic);

Characteristic_Paper.prototype.notifyPaper = function(occupation)
{
    if (this._updateValueCallback) {
        this._value = new Buffer(occupation);
        this._updateValueCallback(this._value);
    }
}

Characteristic_Paper.prototype.onReadRequest = function(offset, callback)
{
    if (offset) {
        callback(this.RESULT_ATTR_NOT_LONG);
    } else {
        console.log('CharacteristicPaper - onReadRequest: value = ' + this._value.toString('hex'));
        callback(this.RESULT_SUCCESS, this._value);
    }
}

Characteristic_Paper.prototype.onSubscribe = function(maxValueSize, updateValueCallback)
{
    console.log('CharacteristicPaper - onSubscribe');
    this._updateValueCallback = updateValueCallback;
};

Characteristic_Paper.prototype.onUnsubscribe = function()
{
  console.log('CharacteristicPaper - onUnsubscribe');
  this._updateValueCallback = null;
};

module.exports = Characteristic_Paper;
