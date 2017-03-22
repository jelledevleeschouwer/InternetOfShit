#!/usr/bin/env node
/**
 *
 **/

var exec = require('child_process').exec;
const servo_flush = 0;  // BCM17/pin11
const servo_locker = 1; // BCM18/pin12
const servo_aroma = 2;  // BCM27/pin13

init_servod();
toilet_flush(95);
aroma_diffuser(1);
aroma_diffuser(2);
toilet_close();
toilet_lock();
toilet_unlock();
kill_servod();

/**
 * initializes the servoblaster, needs to be executed at startup.
 **/
function init_servod() {
	cmd = 'sudo servod --p1pins=11,12,13';
	console.log(cmd);
	exec(cmd,
	    (error, stdout, stderr) => {
	        if (error !== null) {
	            console.log(`exec error: ${error}`);
	        }
	});
	//while (Date.now() < end) ;
	servo_controller(servo_flush, 5);
	servo_controller(servo_locker, 5);
	servo_controller(servo_aroma, 50);
}

/**
 * kills the servoblaster process, needs to be executed at the end.
 **/
function kill_servod() {
	cmd = 'sudo killall servod';
	console.log(cmd);
	exec(cmd,
	    (error, stdout, stderr) => {
	        if (error !== null) {
	            console.log(`exec error: ${error}`);
	        }
	});
}

function servo_controller(id, position) {
	cmd = parse('echo %s=%s% > /dev/servoblaster', id, position);
	console.log(cmd);
	exec(cmd,
	    (error, stdout, stderr) => {
	        if (error !== null) {
	            console.log(`exec error: ${error}`);
	        }
	});
	var end = Date.now() + 700
	while (Date.now() < end) ;
}

function parse(str) {
    var args = [].slice.call(arguments, 1),
        i = 0;
    return str.replace(/%s/g, function() {
        return args[i++];
    });
}

function toilet_flush( water_flow ) {
	servo_controller(servo_flush, water_flow);
	servo_controller(servo_flush, 5);
}

function aroma_diffuser( aroma_id ) {
	if( aroma_id == 1 ) {
		servo_controller(servo_aroma, 5);
	} else if( aroma_id ==2 ) {
		servo_controller(servo_aroma, 95);
	} else {
		console.log('ERROR: aroma_id should be 1 or 2 !!!')
	}
	servo_controller(servo_aroma, 50);
}

function toilet_close() {
	servo_controller(servo_locker, 95);
	servo_controller(servo_locker, 5);
}

function toilet_lock() {
	servo_controller(servo_locker, 95);
}

function toilet_unlock() {
	servo_controller(servo_locker, 5);
}
