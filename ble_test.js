var bleno = require('/usr/local/lib/node_modules/bleno/index.js');

var Descriptor = bleno.Descriptor;
var Characteristic = bleno.Characteristic;
var PrimaryService = bleno.PrimaryService;

var descriptor = new Descriptor({
    uuid: '2901',
    value: 'Hello' // Static value must be of type Buffer or String
});

// BLE_SHIELD_TX
var tx_characteristic = new Characteristic({
    uuid: '713d0003503e4c75ba943148f18d941e',
    properties: [ 'write', 'writeWithoutResponse' ],
    onWriteRequest: function(newData, offset, withoutResponse, callback) { // Coming from Android App
        console.log('Got data from Android App: ' + newData.toString('utf8'));
        callback(bleno.Characteristic.RESULT_SUCCESS);
    },
    descriptors: [ descriptor ]
});

// BLE_SHIELD_RX
var rx_characteristic = new Characteristic({
    uuid: '713d0002503e4c75ba943148f18d941e',
    properties: [ 'read' ],
    onReadRequest: function(offset, callback) {
        console.log('Got read-request from Android App');
        callback(Characteristic.RESULT_SUCCESS, null)
    },
    descriptors: [ descriptor ]
})

// BLE_SHIELD_SERVICE
var primaryService = new PrimaryService({
    uuid: '713d0000503e4c75ba943148f18d941e',
    characteristics: [ tx_characteristic, rx_characteristic ]
});

// Services
var services = [ primaryService ];

// For setting Bleno Services
bleno.on('servicesSet', function(error) {
    console.log('on -> servicesSet: ' + (error ? 'error ' + error : 'success'));
});

// When advertising succeeded, set advertising services
bleno.on('advertisingStart', function(error) {
    console.log('on -> advertisingStart: ' + (error ? 'error ' + error : 'success'));

    if (!error) {
        bleno.setServices( services, function(error) {
            console.log('setServices: '  + (error ? 'error ' + error : 'success'));
        });
    }
});

bleno.on('accept', function(clientAddress) {
    console.log('on -> accept: Client connected: ' + clientAddress);
});

bleno.on('disconnect', function(clientAddress) {
    console.log('on -> disconnect: Client disconnected: ' + clientAddress);
})


