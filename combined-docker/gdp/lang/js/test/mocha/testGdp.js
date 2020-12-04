
// To run this test, do:
//   sudo npm install -g mocha
//   mocha testGdp.js

var gdpjs = require('../../index.js');
var assert = require('assert');

var logdname = "edu.berkeley.eecs.gdp-03.gdplogd";

describe('gdp/lang/js/test/mocha/testGdp.js: create, append and read', function () {
    it('gdp_gcl_open()', function () {
	    var logname = "gdp.lang.js.test.mocha.testGdp." + Math.random();

	    // Opening mode (0: for internal use only, 1: read-only, 2: read-append, 3: append-only)
	    var iomode = 2;
	    var log = new gdpjs.gdp_gcl_open(logname, iomode, logdname);

	    // FIXME: These functions are a bit too general and should be refactored.

	    // Write:
	    var inputData = "My Record.";
	    var gdpdAddress = logdname;
	    var gclName = logname;
		
	    var os = require('os');

	    var logdxname = os.hostname();
	    var gclAppend = true;
	    var recordSource = 0;
	    var recordArray = new Array(inputData);
	    var consoleOut = false;
	    var recordArrayOut = new Array(1);
	    var returnValue = gdpjs.write_gcl_records(gdpdAddress, gclName, logdxname, gclAppend,
                                        recordSource, recordArray, consoleOut,
                                        recordArrayOut);

	    // Read
	    var gdpd_addr = logdname;
	    var gcl_name = logname;
	    var gcl_firstrec = 1;
	    var gcl_numrecs = 1;
	    var gcl_subscribe = false;
	    var gcl_multiread = false;
	    var conout = true; // recdest = -1 so output to console.log()
	    var gdp_event_cbfunc = null; // no callback needed when just logging
	    var wait_for_events = true; // wait indefinitely in read_gcl_records() for
	    var gcl_get_next_event = false;
	    var outputData = gdpjs.read_gcl_records(gdpd_addr, gcl_name,
					      gcl_firstrec, gcl_numrecs,
					      gcl_subscribe, gcl_multiread, /*recdest,*/
					      conout, gdp_event_cbfunc,
					      wait_for_events,
					      gcl_get_next_event
					      );

	    // Remove the trailing newline
	    assert.equal(inputData, outputData.records[0].value.replace(/\0+$/, ''));
	});
});
