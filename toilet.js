const node_modules = '/usr/local/lib/node_modules/'
var bleno = require(node_modules + 'bleno');
var request = require(node_modules + 'request');
var dateformat = require(node_modules + 'dateformat');

/* Constants */
const api_key = '8c6d0d97e3024096bb0134601172003';
const uri_pre = 'https://api.weathersource.com/v1/07750dddc345fa1044c7/history_by_postal_code.json?&timestamp_between=';
const uri_post = '&postal_code_eq=22222&country_eq=US&fields=precip';
const max_compensation = 30 // Max percentage we can compensate for with weather data

/* Variables */
var _waterflow = 0;         // Preffered % of full capacity to flush toilet with
var _aroma = 0;             // Preferred aroma to dispense after flushing
var _capacity = 0;          // Capacity of water the toilet can store at a time
var precipitation = null;
var timestamp = null;
var refreshing = false;

var compensation = 0;

function Toilet(waterflow, capacity)
{
    /* Constructor */
    precipitation = new Buffer(10);
    precipitation.fill(0);
    _waterflow = waterflow;
    _aroma = 1;
    _capacity = capacity;
    compensation = 100;
}

Toilet.prototype._validateNumber = function(number, name, min, max)
{
    if (typeof number !== 'number') {
        console.log(name + ' must be a number');
        throw new Error(name + ' must be a number');
    }

    if (number < min || number > max) {
        console.log(name + ' must be between ' + min + ' and ' + max);
        throw new Error(name + ' must be between ' + min + ' and ' + max);
    }
};

Toilet.prototype.setWaterFlow = function(waterflow)
{
    this._validateNumber(waterflow,'waterflow', 0, 100);
    _waterflow = waterflow;
    console.log('Set waterflow to: ' + waterflow + '%');
};

Toilet.prototype.waterFlow = function()
{
    return _waterflow;
};

Toilet.prototype.setAroma = function(aroma)
{
    this._validateNumber(aroma, 'aroma', 0, 1);
    _aroma = aroma;
    console.log('Set aroma to: ' + aroma + '%');
}

Toilet.prototype.aroma = function()
{
    return _aroma;
}

/*********************************************************************************
 *  Interface with Weather API
 *********************************************************************************/

var api_callback = function(error, response, body)
{
    if (!error && response.statusCode == 200) {
        var result = JSON.parse(body);
        var sum = 0;
        var i = 0;
        var mm = 0;

        result.forEach(function(entry) {
            if (i > 4) // Only support up to 5 last days
                return;
            mm = entry.precip * 25.400;
            precipitation.writeUInt16BE(Math.round(mm * 1000), i * 2);
            sum += mm;
            i += 1;
        });

        sum = sum / 5;
        timestamp = new Date();
        refreshing = false;
        console.log('toilet.js: Retrieved latest weather data succesfully!');
        console.log("toilet.js: averagePrecipitation: " + sum + ' liter/m3');
        console.log("toilet.js: precipitation: " + precipitation.toString('hex'));

        compensation = sum / _capacity;
        if (compensation >= 1) { // Clip at 100 % compensation
            compensation = 1;
        } else {
            compensation = 1 - ((1 - compensation) * (max_compensation / 100));
        }
        console.log('toilet.js: compensation factor: ' + compensation * 100 + '%');
    } else {
//        console.log('toilet.js: ERROR: Failed retrieving API data, returned status \'' + response.statusCode + '\': ' + error);
    }
}

Toilet.prototype.refreshPrecipitation = function()
{
    var yesterday = new Date();

    // Only refresh precipitation data when 24 hours have passed since last
    // retrieval
    yesterday.setHours(yesterday.getHours() - 24);
    //console.log(yesterday + '\n' + timestamp);
    if ((timestamp < yesterday || timestamp == null) && !refreshing) {
        // Build URI for REST API request
        const now = new Date();
        const end = dateformat(now, "yyyy-mm-dd");
        const unix = Math.round(+new Date()- 1000*60*60*24*5);
        const past = new Date(unix);
        const start = dateformat(past, "yyyy-mm-dd");
        const uri = uri_pre + start + '%2C' + end + uri_post;

        // Send a GET request
        request(uri, api_callback);
        refreshing = true;
    }
}

Toilet.prototype.compensation = function()
{
    this.refreshPrecipitation();
    return compensation;
}

Toilet.prototype.precipitation = function()
{
    this.refreshPrecipitation();
    return precipitation;
}

module.exports = Toilet;
