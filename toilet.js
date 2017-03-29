var _waterflow = 0; // Preffered % of full capacity to flush toilet with
var _aroma = 0;     // Preferred aroma to dispense after flushing
var _capacity = 0;  // Capacity of water the toilet can store at a time

function Toilet(waterflow, capacity)
{
    /* Constructor */
    _waterflow = waterflow;
    _aroma = 0;
    _capacity = capacity;
}

Toilet.prototype._validateNumber = function(number, name, min, max)
{
    if (typeof number !== 'number') {
        throw new Error(name + ' must be a number');
    }

    if (number < min || number > max) {
        throw new Error(name + ' must be between ' + min + ' and ' + max);
    }
};

Toilet.prototype.setWaterFlow = function(waterflow)
{
    this._validateNumber(waterflow,'waterflow', 0, 100);
    _waterflow = waterflow;
};

Toilet.prototype.waterFlow = function()
{
    return _waterflow;
};

Toilet.prototype.setAroma = function(aroma)
{
    this._validateNumber(aroma, 'aroma', 0, 1);
    _aroma = aroma;
}

Toilet.prototype.aroma = function()
{
    return _aroma;
}

Toilet.prototype.refreshPercipitation = function()
{
    /* TODO: Request JSON from API and write data into weather_data buffer */
}

Toilet.prototype.averagePercipitation = function()
{
    /* TODO: Calculate average percipitation of the last few days */
}

Toilet.prototype.percipitation = function()
{
    /* Return latest percipitation data */
}

module.exports = Toilet;
