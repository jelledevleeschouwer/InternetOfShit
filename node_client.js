#!/usr/bin/env node

var program = require('commander');
var net = require('net');

/* Default servkit server-path */
var server_path = "/tmp/server.sock";

program
    .version('0.0.1')
    .option('-f, --filepath [path]', 'Path to server socket')
    .parse(process.argv);

if (program.filepath) {
    server_path = program.filepath;
    console.log("Set server socket path to: " + server_path);
}

var client = net.createConnection(server_path);

client.on("connect", function() {
    console.log("Client connected to server at path: " + server_path);
});

client.on("error", function(error) {
    console.log("Server closed connection on his side: " + error);
    process.exit();
})

client.on("data", function(data) {
    JSON.parse(data.toString('utf8'), (key, value) => {
        if (key.length != 0) {
            if (key == 'SEAT') {
                console.log("Seat: " + value);
            } else if (key == 'PAPER') {
                console.log("Paper: " + value);
            } else {
                console.log("ERROR: Unkown key received from server" + key);
            }
        }
    });
});


