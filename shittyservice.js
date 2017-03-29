/**
 *  Dependencies
 **/
var bleno = require('/usr/local/lib/node_modules/bleno');
var util = require('util');

/**
 *  Types
 **/
var PrimaryService = bleno.PrimaryService;

/**
 *  Types
 **/
var CharacteristicWaterFlow = require('./characteristic_waterflow');
var CharacteristicAroma = require('./characteristic_aroma');
var CharacteristicOccupation = require('./characteristic_occupation');
var CharacteristicPaper = require('./characteristic_paper');
var CharacteristicWeather = require('./characteristic_weather');

var occupationChar;
var paperChar;

function ShittyService(toilet) {
    this.occupationChar = new CharacteristicOccupation(toilet);
    this.paperChar = new CharacteristicPaper(toilet);
    ShittyService.super_.call(this, {
        uuid: 'ffffffffffffffffffffffffffd017ed',
        characteristics: [
            new CharacteristicWaterFlow(toilet),
            new CharacteristicAroma(toilet),
            occupationChar,
            paperChar,
            new CharacteristicWeather(toilet)
        ]
    });
}

util.inherits(ShittyService, PrimaryService);

ShittyService.prototype.notifyOccupation = function(occupation)
{
    this.occupationChar.notifyOccupation(occupation)
}

ShittyService.prototype.notifyPaper = function(available)
{
    this.paperChar.notifyPaper(occupation)
}

module.exports = ShittyService;
