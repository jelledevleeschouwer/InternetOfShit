#!/usr/bin/env node

/**
 *  Dependencies
 **/
const node_modules = '/usr/local/lib/node_modules/'
var bleno = require(node_modules + 'bleno');
var program = require(node_modules + 'commander.js');
var ip = require(node_modules + 'ip');
var exec = require('child_process').exec;
var net = require('net');

/**
 *  Types
 **/
var Descriptor = bleno.Descriptor;
var ShittyService = require('./shittyservice');
var Toilet = require('./toilet')

/**
 *  Constants
 **/
const device_name = "InternetOfShit"

const servo_flush = 0;  // BCM17 / pin11
const servo_locker = 1; // BCM18 / pin12
const servo_aroma = 2;  // BCM27 / pin13

/**
 *  Global variables
 **/
var server_path = "/tmp/server.sock"; // Default servkit socket-path
var client;

/* Internet-of-shit related */
var toilet_occupied = false;
var toilet_locked = false;
var paper_available = false;
var paper_occupied = false;
var computed_flow = 0;

var toilet = new Toilet(100, 2);
var shittyService = new ShittyService(toilet);

/* Configuration parameters */
var preferred_aroma = 0;
var preferred_flow = 0;

/* First thing, parse command-line options */
program
    .version('0.1')
    .option('-f, --filepath [path]', 'Path to server socket')
    .parse(process.argv);

if (program.filepath) {
    server_path = program.filepath;
    console.log("Set server socket path to: " + server_path);
}

/*********************************************************************************
 * Setup and configuration
 *********************************************************************************/

/* Spawn servo-daemon process */
init_servod();
init_adc();
init_api();

test();

/*********************************************************************************
 * Internet-of-shit logic
 *********************************************************************************/

/* Notify change in toilet seat pressure */
function toilet_seat_notify(value)
{
    var occupied = value == 1 ? true : false;

    if (toilet_locked) { // Can't do anything with toilet if locked, supply toilet paper to unlock
        return;
    }

    if (toilet_occupied && !occupied) { // Toilet was occupied, but not anymore
        console.log('InternetOfShit: Toilet: AVAILABLE');
        toilet_occupied = occupied;
        /* Diffuse prefered aroma */
        aroma_diffuser(toilet.aroma());
        /* Flush the toilet */
        toilet_flush(toilet.waterFlow() * toilet.compensation())
        /* Close the toilet seat */
        toilet_close();
        /* Check temporary paper_available-flag */
        if (paper_occupied)
            toilet_paper_notify(paper_occupied);
    } else if (!toilet_occupied && occupied) { // Toilet was not occupied, but now is
        console.log('InternetOfShit: Toilet: OCCUPIED');
        toilet_occupied = occupied;
    } else {
        console.log("ERROR: Server should only notify on changes of pressuress\n");
    }

    shittyService.notifyOccupation(toilet_occupied);
}

/* Notify change in toilet paper pressure */
function toilet_paper_notify(value)
{
    var available = value == 1 ? true : false;

    console.log('Toilet occupied: ' + toilet_occupied);
    console.log('Paper available: ' + available);

    if (toilet_occupied) { // Wait with updating paper_available until person is gone
        paper_occupied = available;
        return;
    } else {
        if (available) {
            paper_occupied = false;
            toilet_unlock();
            toilet_locked = false;
        } else {
            toilet_lock();
            toilet_locked = true;
        }
    }

    shittyService.notifyPaper(paper_available);
}

/*********************************************************************************
 *  Interface with BLE and Android-app
 *********************************************************************************/

// Services
var services = [ shittyService ];

// For setting Bleno Services
bleno.on('servicesSet', function(error)
{
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

// Accepting new clients
bleno.on('accept', function(clientAddress)
{
    console.log('on -> accept: Client connected: ' + clientAddress);
});

// Disconnecting clients
bleno.on('disconnect', function(clientAddress)
{
    console.log('on -> disconnect: Client disconnected: ' + clientAddress);
});

bleno.on('stateChange', function(state)
{
    console.log('on -> stateChange: ' + state);
    if (state == 'poweredOn') {
        bleno.startAdvertising(device_name, [ shittyService.uuid ]);
    } else {
        console.log("ERROR: Could not turn on BLE-interface, not able to recover");
        process.exit();
    }
});

/* // XXX: TEST
var toggle = false;
setInterval(function() {
    toggle = !toggle;
    shittyService.notifyOccupation(toggle);
    shittyService.notifyPaper(toggle);
}, 6000);
*/

/*********************************************************************************
 *  Interface with Weather API and
 *********************************************************************************/

function init_api()
{
    toilet.refreshPrecipitation();
}

function test()
{
    console.log('My ip: ' + ip.address('eth0'));
    toilet.compensation();
    toilet.precipitation();
}

/*********************************************************************************
 *  Interface with ADC and toilet-seat and toilet paper
 *********************************************************************************/

/* Connect to ADC-server */
function init_adc()
{
    client = net.createConnection(server_path);
}

/* On connect callback */
client.on("connect", function() {
    console.log("Client connected to server at path: " + server_path);
});

/* On error callback */
client.on("error", function(error) {
    console.log("Server closed connection on his side: " + error);
    process.exit();
})

/* On data available callback */
client.on("data", function(data) {
    JSON.parse(data.toString('utf8'), (key, value) => {
        if (key.length != 0) {
            if (key == 'SEAT') {
                toilet_seat_notify(value);
            } else if (key == 'PAPER') {
                toilet_paper_notify(value);
            } else {
                console.log("ERROR: Unkown key received from server" + key);
            }
        }
    });
});

/*********************************************************************************
 *  Interface with Servos and aroma-diffuser, toilet lock and flush
 *********************************************************************************/

/**
 * initializes the servoblaster, needs to be executed at startup.
 **/
function init_servod()
{
	cmd = 'sudo servod --p1pins=11,12,13';
	exec(cmd,
	    (error, stdout, stderr) => {
	        if (error !== null) {
	            console.log(`exec error: ${error}`);
	        }
	});
}

/**
 * kills the servoblaster process, needs to be executed at the end.
 **/
function kill_servod()
{
	cmd = 'sudo killall servod';
	console.log(cmd);
	exec(cmd,
	    (error, stdout, stderr) => {
	        if (error !== null) {
	            console.log(`exec error: ${error}`);
	        }
	});
    servo_controller(servo_flush, 5);
    servo_controller(servo_locker, 5);
    servo_controller(servo_aroma, 50);
}

function servo_controller(id, position)
{
	cmd = parse('echo %s=%s% > /dev/servoblaster', id, position);
	exec(cmd,
	    (error, stdout, stderr) => {
	        if (error !== null) {
	            console.log(`exec error: ${error}`);
	        }
	});
	var end = Date.now() + 700
	while (Date.now() < end) ;
}

function parse(str)
{
    var args = [].slice.call(arguments, 1),
        i = 0;
    return str.replace(/%s/g, function() {
        return args[i++];
    });
}

function toilet_flush(water_flow)
{
    const max_flow = 50;
    console.log("Flushing toilet with flow of: " + water_flow);
    water_flow = (water_flow / 100) * max_flow;
	servo_controller(servo_flush, water_flow);
	servo_controller(servo_flush, 5);
}

function aroma_diffuser(aroma_id)
{
	if (aroma_id == 1) {
		servo_controller(servo_aroma, 5);
	} else if (aroma_id == 2) {
		servo_controller(servo_aroma, 95);
	} else {
		console.log('ERROR: aroma_id should be 1 or 2 !!!')
	}

	servo_controller(servo_aroma, 50);
}

function toilet_close()
{
	servo_controller(servo_locker, 5);
	servo_controller(servo_locker, 95);
}

function toilet_lock()
{
    console.log("Internet of shit lock: ON");
	servo_controller(servo_locker, 5);
}

function toilet_unlock()
{
    console.log("Internet of shit lock: OFF");
	servo_controller(servo_locker, 95);
}

