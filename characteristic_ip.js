/**
 *  Dependencies
 **/
const node_modules = '/usr/local/lib/node_modules/'
var bleno = require(node_modules + 'bleno');
var ip = require(node_modules + 'ip');
var util = require('util');

/**
 *  Types
 **/
var Characteristic = bleno.Characteristic;
var Descriptor = bleno.Descriptor;

/**
 *  Constructor
 **/
var Characteristic_Ip = function(toilet)
{
    Characteristic_Ip.super_.call(this, {
        uuid : '0000000000000000000000000000fff5',
        properties: [ 'read' ],
        value: null,
        descriptors: [
            new Descriptor({
                uuid: '2901',
                value: 'IP-address'
            })
        ]
    });

    this.toilet = toilet;
}

util.inherits(Characteristic_Ip, Characteristic);

Characteristic_Ip.prototype.onReadRequest = function(offset, callback)
{
    if (offset) {
        console.log('Received READ for \'ATTR_LONG:\'' + offset);
        callback(this.RESULT_ATTR_NOT_LONG);
    } else {
        const buf = new Buffer(4);
        var arr = ip.address('eth0').split(".");
        buf.writeUInt8(parseInt(arr[0], 10), 0);
        buf.writeUInt8(parseInt(arr[1], 10), 1);
        buf.writeUInt8(parseInt(arr[2], 10), 2);
        buf.writeUInt8(parseInt(arr[3], 10), 3);
        callback(this.RESULT_SUCCESS, buf);
    }
}

module.exports = Characteristic_Ip;
