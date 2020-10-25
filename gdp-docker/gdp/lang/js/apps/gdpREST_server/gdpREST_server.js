#!/usr/bin/env node
/* vim: set ai sw=4 sts=4 ts=4 : */

// Node.js-Based JavaScript server to provide a REST interface to the GDP
//
// Alec Dara-Abrams
// 2014-11-05
//
// TBD: Copyright, clean up code; regularize indenting with JS-aware tool;
//      bring internal doc up to date.
//      Check for possible error returns from libgdp calls.


// Documentation:
//   RESTful Interface for the Global Data Plane
//   Eric Allman
//   U.C. Berkeley Swarm Lab 2014-09-20
//   Draft 5
//   http://swarmlab.eecs.berkeley.edu/publications/5097/
//             restful-interface-global-data-plane
//   References to sub-sections of this document are prefixed with "//D"
//   in the code below.

// Assumptions:
//   Installation has done "npm install" in the directory of this file.
//   This install is driven by the "dependencies:" listed in package.json,
//   a npm package description file.  Note, the dependencies are version-
//   dependent. The Makefile for this program should perform this install.
//
//   The GDP directory containing the files that implement the GCL's is
//   located at GDP_GCL_ABSPATH = '/var/tmp/gcl' .  See the declaration of
//   GDP_GCL_ABSPATH below for comments.
//
// Usage:
//   Window A:
//     Start a gdp daemon:
//      ../../../../gdpd/gdpd &
//   Window B:
//   node gdpREST_server.js
//     Starts an HTTP server on default port 8080 (env var PORT overrides)
//     which listens and acts on GDP RESTful interface HTTP requests.  See
//     documentation above for details.
//   Window C:
//   Examples of client use of API.
//     Start a web browser on http://localhost:8080/gdp/v1/gcl
//     GET http://localhost:8080/gdp/v1/gcl
//     Response: list of GDP GCL's on localhost as lines of plain text
//               Note, names shown are Base64url notation.


// Load Node.js modules for calling foreign functions -- C functions here.
//
// See libgdp_h.js for details and assumptions about the directory
// where Node.js modules must be placed.
//
// Parameters needed for libgdp_h.js and gdpjs_supt.js
// They MUST be adapted to the directory where this program will be run.
// See libgdp_h.js for details.
var GDP_DIR           = "../../../../";
var GDPJS_DIR         = GDP_DIR + "./lang/js/gdpjs/";
var NODE_MODULES_DIR  = "";
//
var LIBGDP_H_DIR = GDPJS_DIR;
// Here we include and evaluate our shared GDP Javascript "header file",
// 'libgdp_h.js', within the global scope/environment.
var fs = require('fs');   // Node.js's built-in File System module
eval( fs.readFileSync( LIBGDP_H_DIR + 'libgdp_h.js').toString() );
// We similarly include & evaluate some JS support functions.
eval( fs.readFileSync( LIBGDP_H_DIR + 'gdpjs_supt.js').toString() );
eval( fs.readFileSync( LIBGDP_H_DIR + 'rw_supt.js').toString() );


// =============================================================================
// Load gdpREST JS Support routines
var GDP_REST_DIR           = "./";
eval( fs.readFileSync( GDP_REST_DIR + 'gdpREST_server_supt.js').toString() );


// =============================================================================
// Initialization


// DEBUG = true to include console.log() debugging output; else = false.
var DEBUG = false;


// TBD: add as startup option
var GDP_GCL_ROOT_DIR = '/gdp/v1';

// Load the Express packages?/modules? we need
var express    = require('express');
// Expres "middleware" modules
var bodyParser = require('body-parser');

// Create our server app as a JS object (courtesy of Express).
//   We will start it far below.  See app.listen(port) .
var app        = express();

// Configure app - general parsing for HTTP request body
// TBD: not clear yet just what triggers this (copying?) and parsing.
//      Only for requests with particular Content-Type header values?
app.use(bodyParser.urlencoded({ extended: true }));
app.use(bodyParser.json());
app.use(bodyParser.raw({ type: "application/octet-stream", limit: "10mb" }));

// app will listen on this port - default can be overriden from the environment
var port     = process.env.PORT || 8080;


// GDP initialization - null String implies default GDP daemon at localhost:2468
// TBD: add daemon host:port as a program parameter
// TBD: Needed - we should be stateless wrt GDP, right? No here need for
//      gdp_rest_init( '' );  ?


// =============================================================================
// Set up routing (HTTP request dispatching)
// {

// TBD: Do we really want to use an express.Router to set up routing or just
//      set up directly using app.VERB() & app.use() ?
//
// Create a router for app
var router = express.Router();

// Some non-method-specific testing and DEBUG routes
//
// express server-side "middleware" to use for all requests
router.use(
function(req, res, next)
{
	if( DEBUG )
	{
    // TBD: control this (sometimes voluminous) logging via a startup option
	console.log('{\n Incoming request: req.method = ' + req.method );
	console.log( get_request_components( req ) );
	console.log('\n}');
	}
	next();
});

// Some method-specific testing and DEBUG routes

// A little usage information and a nascent admin dashboard
// Accessed via GET http://localhost:8080/gdp/v1/
router.get('/',
function(req, res)
{
	// Construct this page dynamically as an Express view (with layout?)
	var help_html = 
	"<!DOCTYPE html> \
	 <html> \
     <body> \
 \
     <h3>Helpful URLs</h3> \
	 <a href='http://127.0.0.1:8080/gdp/v1'>This help page.</a> \
     <h3>Test GET URLs which can be sent directly by a browser link</h3> \
	 <ul> \
	 <li> \
	 <a href='http://127.0.0.1:8080/gdp/v1/DEBUG/OFF'> \
	    Turn server side debug output off.</a> \
	 </li> \
	 <li> \
	 <a href='http://127.0.0.1:8080/gdp/v1/DEBUG/ON'> \
	    Turn server side debug output on.</a> \
	 </li> \
	 <li> \
	 <a href='http://127.0.0.1:8080/gdp/v1/DEBUG/requestAsString'> \
	    Return the request sent by this link as a string.</a> \
	 </li> \
	 <li> \
	 <a href='http://127.0.0.1:8080/gdp/v1/DEBUG/headers'> \
	    Return the request headers sent by this link as JSON.</a> \
	 </li> \
	 <li> \
	 <a href='http://127.0.0.1:8080/gdp/v1/gcl'> \
	    GET /gdp/v1/gcl — list all known GCLs.</a> \
	 </li> \
	 <li> \
	 <a href='http://127.0.0.1:8080/gdp/v1/gcl/1UGpEMT_Wsyed3DUulxHFtgWMsxjGdo73CNcr_JLP20'> \
	    GET /gdp/v1/gcl/<gcl_name> — list information known about specified GCL.</a> \
	 </li> \
	 <li> \
	 <a hrefxx='http://127.0.0.1:8080/gdp/v1/gcl/4iITrdLdbuunxpZbYGHbAXKIXnvdllWc-wkYPh69ueY/1'> \
	    Use POSTMAN: GET /gdp/v1/gcl/<gcl_name>/<recno#> — return a specified record.</a> \
	 </li> \
	 <li> \
	 <a hrefxx='http://127.0.0.1:8080/gdp/v1/gcl/4iITrdLdbuunxpZbYGHbAXKIXnvdllWc-wkYPh69ueY/last'> \
	    Use POSTMAN: GET /gdp/v1/gcl/<gcl_name>/last — return a specified record - the most recently written record.</a> \
	 </li> \
	 <li> \
	 <a href='http://127.0.0.1:8080/gdp/v1/gcl/4iITrdLdbuunxpZbYGHbAXKIXnvdllWc-wkYPh69ueY/2/3'> \
	    GET /gdp/v1/gcl/<gcl_name>?recno=<#>&nrecs=<#> — get a series of records.</a> \
	 </li> \
	 </ul> \
     <h3>Test POST URLs which can be sent by POSTMAN</h3> \
	 <ul> \
	 <li> \
	    TBD here from local browser-side JavaScript. \
	 </li> \
	 </ul> \
     </body> \
     </html>";

	if( DEBUG )
	{ console.log( help_html );	}
	res.send( help_html );	
});

// Test and DEBUG routes are below the path GDP_GCL_ROOT_DIR/DEBUG
// DEBUG output is always on for requests on these routes

// Accessed via GET http://localhost:8080/gdp/v1/DEBUG/:on_off
// Turn server DEBUG switch on or off  ( :on_off = "ON" | "OFF" )
router.get('/DEBUG/:on_off',
function(req, res, next)
{
	var dbg_state  = req.params.on_off;
	var reqstr = req_asString(req);	
	console.log( { message: reqstr } );	
	if ( dbg_state.toUpperCase() == "ON"  )
	{  DEBUG = true;  }
	else
	if ( dbg_state.toUpperCase() == "OFF" )
	{  DEBUG = false; }
	else
	{  // else leave as is and go on to the next potential route
	   next();
	}
	// TBD: prefix with program name
	console.log( 'DEBUG state = \"' + dbg_state + '\"' );
	res.json( { message: reqstr } );	
});

// Accessed via GET http://localhost:8080/gdp/v1/DEBUG/requestAsString
// Send the GET request back as a brief JSON-ish string.
// GET http://localhost:8080/gdp/v1/DEBUG/requestAsString

// TBD: we get the below error on this URL only from browser (Chrome) NOT from
//      Postman (with no headers).
// DEBUG state = "requestAsString"
// Error: Can't set headers after they are sent.
// Problem with next() in 'DEBUG/:on_off' above?  If so, what about the
// ordering of other route matches when there is a possibility of more than
// one match?

router.get('/DEBUG/requestAsString',
function(req, res)
{
	var reqstr = req_asString(req);	
	console.log( { message: reqstr } );	
	res.json(    { message: reqstr } );	
});

// Accessed via GET http://localhost:8080/gdp/v1/DEBUG/headers
router.get('/DEBUG/headers',
function(req,res)
{
	var reqstr = req_asString(req);	
	console.log( { message: reqstr } );	
	res.set('Content-Type','text/plain');
	var s = '{\n';
	for( var name in req.headers )
	{ s += '   ' + name + ': ' + req.headers[name] + '\n'; }
	s +='\n}';
	res.send(s);
});

// Accessed via POST http://localhost:8080/gdp/v1/DEBUG/withBody
// Send the POST request with data in the body back as a brief JSON-ish string.
// Set up by, say, POSTMAN.
// TBD: note we're using app.VERB() here not router.VERB() - makes a difference?
app.post('/DEBUG/withBody',
function(req, res)
{
	// var base64EncodedBody = new Buffer(req.body.toString(), 'base64');
	var reqstr = req_asString(req);	
	// var req_body_asString = new Buffer( req.body.toString() );
	var req_body_asString = req.body.toString();
	console.log({
		message:  reqstr,
		body:     req_body_asString.toString('utf8'),
	});
	res.json({
		message:  reqstr,
		body:     req_body_asString.toString('utf8'),
	});
});


// REST API Proper Routes at GDP_GCL_ROOT_DIR/

//D GET /gdp/v1/gcl — list all known GCLs
//D Request:
//D   Method:       GET
//D   Request URI:  /gdp/v1/gdl
//D   HTTP Vesion:  HTTP/1.1
//D   Headers:
//D     Host:       <requesting host:port>
//D   Body:         none
//D Response: 
//D   HTTP Vesion:  HTTP/1.1
//D   Status Code:   200
//D   Reason Phrase: OK
//D   Headers:
//D     Content-Type: application/json; type=gdp/gcllist
//D   Body:
//D     [
//D      	<gcl_id>,
//D      	...
//D     ]
//D We supress detail in references to the GDP RESTful Interface document below
//D? TBD: add note about 500 eror below
//
// Accessed via GET http://localhost:8080/gdp/v1/gcl
router.get('/gcl',
function(req, res)
{
	if( DEBUG )
	{
	var reqstr = req_asString(req);	
	console.log( 'GET /gdp/v1/gcl — list all known GCLs' );
	console.log( { message: reqstr } );	
	}
	var array_of_gcl_names;

	array_of_gcl_names = get_gcl_base64_namesSync( /* use default GCL path */ );
	if ( array_of_gcl_names == null )
	{	// some sort of error on call above
		// TBD is this the right way to return an HTTP error of this type?
		res.type('text/plain');
		res.status(500);
		res.send('500 - Internal Server Error, GCL directory not availble.');
	}
	else
	{	res.json( array_of_gcl_names );
	}
});


//D GET /gdp/v1/gcl/<gcl_name> — list information known about specified GCL
//D Defining documentation detail suppressed for now.
//
// Accessed via GET http://localhost:8080/gdp/v1/gcl/<gcl_name>
router.get('/gcl/:gcl_name',
function(req, res)
{
	if( DEBUG )
	{
	var reqstr = req_asString(req);	
	console.log( 'GET metadata for gcl: \"' + req.params.gcl_name + '\"' );	
	console.log( { message: reqstr } );	
	}
	var res_body =
	{	// Currently, no metadata other than the name
		'gcl_id': req.params.gcl_name
	};
	// TBD: check for existence/non-existence - 404 Not Found.
	//      Can this be done with the current libgdp?
	// TBD: awaiting libgdp additions for GCL metadata (# of records included)
	res.set( 'Content-Type', 'application/json; type=gdp/gcldesc' );
	res.json( res_body );
});


//D POST /gdp/v1/gcl — Create a new (automatically named) GCL
//D Defining documentation detail suppressed for now.
//
// Accessed via POST http://localhost:8080/gdp/v1/gcl
router.post('/gcl',
function(req, res)
{
	if( DEBUG )
	{
	var reqstr = req_asString(req);	
	console.log( 'POST /gdp/v1/gcl — Create a new GCL' );
	console.log( { message: reqstr } );	
	}
	var res_body =
	{	// Currently, no metadata other than the name
		gcl_id: null,
		nrecs:  0,
		// for low level server-side error info
		err:    { ok:   /* Boolean */            null,
		          code: /* Strig: hex integer */ null,
				  msg:  /* String */             null
				}
	};

	// set up for call to write_gcl_records()
	var gdpd_addr  = null;  // default "127.0.0.1:2468"
	var gcl_name   = null;  // default; create a new gcl
	var gcl_append = false; // ignored on create a new gcl
	var recsrc     = 0;     // read the gcl records from the Array recarray
	var recarray   = [ ];   // no elements to write just create a new, empty GCL
	var conout     = false; // no output to console for a recarray source
	var wrv;                // temp return value holder

	/*  Returns:
		{ error_isok: false|true, error_code: EP_STAT, error_msg: String,
		  gcl_name: String
		}
	*/
	wrv = write_gcl_records( gdpd_addr, gcl_name, gcl_append,
	                         recsrc, recarray, conout );
	// TBD: error checking after call
	// set up the JSON we return in the response body
	res.set( 'Content-Type', 'application/json; type=gdp/gcldesc' );
	res_body.gcl_id = wrv.gcl_name;  // Base64 String name of newly created gcl
	// Lower level error information - .ok == true ==> not a problemo
	res_body.err = { ok: wrv.error_isok, code: wrv.error_code,
	                 msg: wrv.error_msg };
	res.status(201);  // Created
	res.json( res_body );
});


//D PUT /gdp/v1/gcl/<symbolic_name> — Create a new (explicitly) named GCL
//D Defining documentation detail suppressed for now.
//
// Accessed via PUT http://localhost:8080/gdp/v1/gcl/<symbolic_name>
/*
   TBD: We have a problem with this request.  We get an assertion failed
        from gdp_api.c:366 - a problem with closing a gclh
        This seems to happen iff the named gcl already exists - check this.

   GDPD output:
   PUT create new explicitly named gcl: "a_symbolic_gcl_name"
   { message: 'HTTP request: PUT sent to localhost:8080/gdp/v1/gcl/a_symbolic_gcl_name' }
   Assertion failed at gdp_api.c:366: assert: (gclh) != NULL && (gclh) != EP_GEN_DEADBEEF
   Abort trap: 6
 */
router.put('/gcl/:symb_gcl_name',
function(req, res)
{
	if( DEBUG )
	{
	var reqstr = req_asString(req);	
	console.log( 'PUT create new explicitly named gcl: \"' + req.params.symb_gcl_name + '\"' );	
	console.log( { message: reqstr } );	
	}
	var symname = req.params.symb_gcl_name;
	var res_body =
	{	// Currently, no metadata other than the name
		gcl_id: null,
		nrecs:  0,
		// for low level server-side error info
		err:    { ok:   /* Boolean */            null,
		          code: /* Strig: hex integer */ null,
				  msg:  /* String */             null
				}
	};
	
	// set up for call to write_gcl_records()
	var gdpd_addr  = null;  // default "127.0.0.1:2468"
	var gcl_name   = symname;  // name of new gcl is in symname
	var gcl_append = false; // no data to append on this create
	var recsrc     = 0;     // read the gcl records from the Array recarray
	var recarray   = [ ];   // no elements to write just create a new, empty GCL
	var conout     = false; // no output to console for recarray source
	var wrv;

	/*  Returns:
		{ error_isok: false|true, error_code: EP_STAT, error_msg: String,
		  gcl_name: String
		}
	*/
	wrv = write_gcl_records( gdpd_addr, gcl_name, gcl_append,
	                         recsrc, recarray, conout );
	// TBD: error checking after call
	// set up the JSON we return in the response body
	res.set( 'Content-Type', 'application/json; type=gdp/gcldesc' );
	// We trust that wrb.gcl_name and (symname==gcl_name) are == .
	// TBD check if not?
	// TBD are we returning the correct per spec gcl form of name & is this
	//     the best form of gcl name to return?
	res_body.gcl_id = wrv.gcl_name;  // Base64 String name of newly created gcl
	res_body.err = { ok: wrv.error_isok, code: wrv.error_code,
	                 msg: wrv.error_msg };
	res.status(201);  // Created
	res.json( res_body );
});


//D DELETE /gdp/v1/gcl/<gcl_name> — delete a GCL
//D Defining documentation detail suppressed for now.
//
// Accessed via DELETE http://localhost:8080/gdp/v1/gcl/<gcl_name>
router.delete('/gcl/:gcl_name',
function(req, res)
{
	if( DEBUG )
	{
	var reqstr = req_asString(req);	
	console.log( 'DELETE gcl: \"' + req.params.gcl_name + '\"' );	
	console.log( { message: reqstr } );	
	}

	// TBD: awaiting libgdp additions for GCL deletion
	res.type('text/plain');
	res.status(501);
	res.send('501 - Not Implemented');
});


//D POST /gdp/v1/gcl/<gcl_name> — add a record to specified GCL
//D Request Data
//D (Opaque data to be appended, but recommended to use JSON)
//D Response Data
//D 201 Created
//D     Content-Type: application/json; type=gdp/response
//D 
//D     {
//D      	"gcl_id": <gcl_id>,
//D      	"recno": <integer>,
//D 	    "timestamp": <commit timestamp>
//D 	    ...
//D      }
//D 401 Unauthorized
//D 404 Not Found
//D Description
//D Adds a record to the named GCL.  The information returned shows the GDP-assigned metadata associated with the new record.
//
// Accessed via POST http://localhost:8080/gdp/v1/gcl/<gcl_name>
router.post('/gcl/:gcl_name',
function(req, res)
{
	if( DEBUG )
	{
	var reqstr = req_asString(req);	
	console.log( 'POST add a record to specified gcl: \"' + req.params.gcl_name + '\"' );	
	console.log( { message: reqstr } );	
	console.log( 'req.body = \"' +  req.body + '\"' );
	}
	var gcl_name = req.params.gcl_name;

	var res_body =
	{	// Currently, no metadata other than the name
		gcl_id:    null,
		recno:     -1,
		timestamp: "<commit timestamp>",
		// for low level server-side error info
		err:    { ok:   /* Boolean */            null,
		          code: /* Strig: hex integer */ null,
				  msg:  /* String */             null
				}
	};
	
	// set up for call to write_gcl_records()
	var gdpd_addr  = null;  // default "127.0.0.1:2468"
	                        // name of new gcl is already in gcl_name
	var gcl_append = true;  // add new record to end of gcl
	var recsrc     = 0;     // read the gcl records from the Array recarray
	// just a single element to write to the existing GCL
	var recarray   = [ req.body ];
	var conout     = false; // no output to console for recarray source
	var wrv;

	/*  Returns:
		{ error_isok: false|true, error_code: EP_STAT, error_msg: String,
		  gcl_name: String
		}
		recarray_out[] is an array parallel to recarray[] with, in our
		one-record-write case here, a single element:
		{ recno: <record number of new record>, time_stamp: <ts of new record> }
	*/
	var recarray_out = [];
	wrv = write_gcl_records( gdpd_addr, gcl_name, gcl_append,
	                         recsrc, recarray, conout, recarray_out );
	// TBD: error checking after call
	// TBD: especially, check for gcl with name gcl_name not existing
	// set up the JSON we return in the response body
	res.set( 'Content-Type', 'application/json; type=gdp/response' );
	// We trust that wrb.gcl_name and gcl_name are == .  TBD check if not?
	// TBD are we returning the correct per spec gcl form of name & is this
	//     the best form of gcl name to return?
	res_body.gcl_id = wrv.gcl_name;  // Base64 String name of specified gcl
	// TBD: make sure recarray_out[0] and its recno and time_stamp fields exist
	res_body.recno = recarray_out[0].recno;
	res_body.timestamp = recarray_out[0].time_stamp;
	res_body.err = { ok: wrv.error_isok, code: wrv.error_code,
	                 msg: wrv.error_msg };
	res.status(201);  // Created
	res.json( res_body );
});


//D GET /gdp/v1/gcl/<gcl_name>?recno=<#> — return a specified record
//  Modified to:
//D GET /gdp/v1/gcl/<gcl_name>/<recno#> — return a specified record
//  TBD: check for lexical incompatibilities with URL-encoded constraints
//       Use query strings to pass in arguments as per spec.
//D Request Data
//D none
//D Response Data
//D 200 OK
//D     Content-Type: <as specified as metadata during GCL creation>
//D      GDP-Record-Number: <recno>
//D      GDP-Commit-Timestamp: <timestamp>
//D 
//D     <opaque data as written by POST>
//D 404 Not Found
//D Description
//D Returns the contents of the record indicated by recno.  As a special case, if recno is the text "last" it returns the last (most recently written) record.
//D Note that the metadata is included in the response header, not in the data itself, in order to maintain the opacity of that data. Question: should we move the metadata into the header for other commands as well to maintain symmetry?
//D This call is not orthogonal to the others because it does not assume that the data is application/json.
//
// Accessed via GET /gdp/v1/gcl/<gcl_name>/<recno#>
router.get('/gcl/:gcl_name/:recno',
function(req, res)
{
	var gcl_name = req.params.gcl_name;
	var recno    = req.params.recno;

	if( DEBUG )
	{
	var reqstr = req_asString(req);	
	console.log( 'GET specified record gcl: \"' + gcl_name +
	             '\"  recno =\"' + recno + '\"' );	
	console.log( { message: reqstr } );	
	}

	// translate "last" into internal indicator value
	if ( recno == "last" ) { recno = -1; }

	// set up for call to read_gcl_records()
	var gdpd_addr      = null;  // default "127.0.0.1:2468"
	                            // name of new gcl is already in gcl_name
    var gcl_firstrec   = recno; // just one record to read here
	var gcl_numrecs    = 1;
    var gcl_subscribe  = false;
	var gcl_multiread  = false;
	var recdest        = 0;     // read the gcl records into the rrv Array
	// DEBUG
	// var recdest        = -1;    // write the gcl records to stdout
	var conout         = false; // no output to console for Array return
	var rrv;

    /* Returns:
	   { error_code: EP_STAT,
	     records: Array of Strings, each element with opaque record content
	   }
     */
    rrv = read_gcl_records( gdpd_addr, gcl_name,
                            gcl_firstrec, gcl_numrecs,
                            gcl_subscribe, gcl_multiread, recdest,
							conout,
							// below not used for subscribe==false
							null,  // gdp_event_cbfunc,
							false  // wait_for_events
                          );
	// TBD: error checking after call: bad gdpd_addr, gcl_name, record numbers
	
	// TBD: Content-Type of data written to record not available from a gcl.
	//      This seems to be per record metadata or is the type
	//      the same for all the records in a given gcl?
	// TBD: any parameters on 'application/octet-stream' - like a limit?
	res.set( 'Content-Type',         'application/octet-stream' );
	if ( rrv.records[0] )
	{ res.set( 'GDP-Record-Number',    rrv.records[0].recno     );
	  res.set( 'GDP-Commit-Timestamp', rrv.records[0].timestamp );
	  // only one record to return for this request
	  if( DEBUG )
	  { console.log( 'rrv.records[0].value = \"' + rrv.records[0].value + '\"' );
	  }
	  res.send( rrv.records[0].value );
	}
	else
	{ // TBD: is this the only way we can have a bad (e.g., non-existent)
	  //      recno coming back from read_gcl_records() above?
	  // res.set( 'GDP-Record-Number',    "undefined"              );
	  // res.set( 'GDP-Commit-Timestamp', "undefined"              );
	  // // only one record to return for this request
	  if( DEBUG )
	  { console.log( 'rrv.records[0].value = \"' + "undefined" + '\"' );
	    console.log('  Returning 501 - Not Implemented');
	  }
	  // TBD: is this the correct error return?
	  // res.send( "" );  // TBD: is this the best error value to return here?
	  res.type('text/plain');
	  res.status(404);
	  res.send('404 - Not Found');
	}
});


//D GET /gdp/v1/gcl/<gcl_name>?recno=<#>&nrecs=<#> — get a series of records
//  Modified to:
//D GET /gdp/v1/gcl/<gcl_name>/<recno#>/<nrecs#> —  get a series of records
//  TBD: check for lexical incompatibilities with URL-encoded constraints
//       Use query strings to pass in arguments as per spec.
//       Note: The recno parameter is optional and defaults to 1.
//D Request Data
//D none
//D Response
//D 200 OK
//D     [
//D 	{
//D 	    "recno": <integer>,
//D 	    "timestamp": <timestamp>,
//D 	    "value": <record value>
//D 	},
//D 	...
//D     ]
//D 404 Not Found
//D Description
//D Returns a sequence of up to nrecs records starting from recno encoded as an array of JSON objects.  If nrecs is zero, all data from recno to the end is returned.  The recno parameter is optional and defaults to 1.
//
// Accessed via GET /gdp/v1/gcl/<gcl_name>/<recno#>/<nrecs#>
router.get('/gcl/:gcl_name/:recno/:nrecs',
function(req, res)
{
	var gcl_name = req.params.gcl_name;
	var recno    = req.params.recno;
	var nrecs    = req.params.nrecs;

	// TBD: would a device like this also be useful for the multi-record
	//      read case here?  Maybe use nrecs as the number of records
	//      going backwards from the most recent ("last") record.
	// // translate "last" into internal indicator value
	// if ( recno == "last" ) { recno = -1; }

	if( DEBUG )
	{
	var reqstr = req_asString(req);	
	console.log( 'GET specified record gcl: \"' + gcl_name +
	             '\"  recno =\"' + recno + '\"' +
	             '\"  nrecs =\"' + nrecs + '\"' );	
	console.log( { message: reqstr } );	
	}
	
	// set up for call to read_gcl_records()
	var gdpd_addr      = null;  // default "127.0.0.1:2468"
	                            // name of new gcl is already in gcl_name
    var gcl_firstrec   = recno; // multiple records to read here
	var gcl_numrecs    = nrecs;
    var gcl_subscribe  = false;
	var gcl_multiread  = false;
	var recdest        = 0;     // read the gcl records into the rrv Array
	var conout         = false; // no output to console for Array return
	var rrv;

    /* Returns:
	   { error_code: EP_STAT,
	     records: Array of Strings, each element with opaque record content
	   }
     */
    rrv = read_gcl_records( gdpd_addr, gcl_name,
                            gcl_firstrec, gcl_numrecs,
                            gcl_subscribe, gcl_multiread, recdest,
							conout,
							// below not used for subscribe==false
							null,  // gdp_event_cbfunc,
							false  // wait_for_events
                          );
	// TBD: error checking after call: bad gdpd_addr, gcl_name, record numbers
	
	// TBD: Content-Type of data written to record not available from a gcl.
	//      This seems to be per record metadata or is the type
	//      the same for all the records in a given gcl?
	// TBD: any parameters on 'application/octet-stream' - like a limit?
	// res.set( 'Content-Type',         'application/octet-stream' );
	// TBD: We're being creative here -- docs need some work on Content_Type
	res.set( 'Content-Type', 'application/json; type=gdp/response' );
	// one or more records to return for this request
	if( DEBUG )
	{
	console.log( 'rrv.records = \"', rrv.records, '\"' );
	}
	res.json( rrv.records );
});




//D GET /gdp/v1/post/gcl/<gcl_name>?<arguments> — add data to a GCL (not REST compliant)
//D Description
//D To be determined.  Probably will create a JSON object including the specified arguments and append that to the GCL, unencrypted of course.



// Accessed via GET http://localhost:8080/gdp/v1
router.get('/',
function(req, res)
{
	var reqstr = req_asString(req);	
	console.log( { message: reqstr } );	
	res.json(    { message: reqstr } );	
});

// Accessed via POST http://localhost:8080/gdp/v1
router.post('/',
function(req, res)
{
	var reqstr = req_asString(req);	
	console.log( { message: reqstr } );	
	res.json(    { message: reqstr } );	
});

// } Done setting up routing -- static for this server
// =============================================================================


// Register our routes with our server
app.use( GDP_GCL_ROOT_DIR, router );


// Start our server
// =============================================================================
app.listen(port);
console.log( 'gdpREST_server.js Listening on port ' + port);
console.log( 'Start a browser at: http://127.0.0.1:' + port + '/gdp/v1' );
console.log( 'Turn server side debug output off' );
console.log( '   http://127.0.0.1:8080/gdp/v1/DEBUG/OFF' );
console.log( 'Turn server side debug output back on' );
console.log( '   http://127.0.0.1:8080/gdp/v1/DEBUG/ON' );


// If arg is true, output some simple testing/debug information
// to console.log() and return
misc_lower_level_inline_tests( /* Boolean */ false );



// TBD1 - clean up this code
// Accessed via GET /gdp/v1/gcl/<gcl_name>/<recno#>/<nrecs#>/<timeout-seconds> 
router.get('/gcl/:gcl_name/:recno/:nrecs/:timeout',
function(req, res)
{
	var gcl_name = req.params.gcl_name;
	var recno    = req.params.recno;
	var nrecs    = req.params.nrecs;
	var timeout  = req.params.timeout;

	// we append rrv.records after each call of read_gcl_records()
	var recs_read = [];

	if( DEBUG )
	{
	var reqstr = req_asString(req);	
	console.log( 'GET monitor a GCL gcl: \"' + gcl_name +
	             '\"  recno   =\"' + recno   + '\"' +
	             '\"  nrecs   =\"' + nrecs   + '\"' +
	             '\"  timeout =\"' + timeout + '\"' );	
	console.log( { message: reqstr } );	
	}


	// will be set true after timeout seconds by cbf_Tout() as a result of
	// setTimeout( cbf_Tout, ...).  We stop polling below when it goes true.
	// LEFT OFF HERE - don't need this Boolean anymore?
	var monitoring_done = false;
	if( DEBUG )
	{
	console.log( 'monitoring_done = \"' + monitoring_done + '\"' );
	}

	// var delay_Tout = 5000;  // milliseconds
	var delay_Tout = timeout * 1000;  // seconds to milliseconds
	var aTimeoutObject;
	var nCalls_cbf_Tout = 0;

	function cbf_Tout( a1, a2 ) 
	{
		if( DEBUG )
		{
		nCalls_cbf_Tout++;
		console.log( 'In cbf_Tout nCalls_cbf_Tout = %d', nCalls_cbf_Tout );
		// console.log( 'a1 = \"' + a1 + '\"' );
		// console.log( 'a2 = \"' + a2 + '\"' );
		// fflush_all_js();  // sometimes Node.js may not empty buffers
		}
		clearInterval( anIntervalObject );
		if( DEBUG )
		{
		monitoring_done = true;
		console.log( 'monitoring_done = \"' + monitoring_done + '\"' );
		}
	    cb_finalize();
	}

	var cbf_Tout_arg_1 = 1, cbf_Tout_arg_2 = 2;

	// setTimeout(callback, delay, [arg], [...])
	aTimeoutObject = setTimeout( cbf_Tout, delay_Tout, 
								 cbf_Tout_arg_1, cbf_Tout_arg_2 );

	// var delay_Int = 1000;  // milliseconds
	// TBD better ratio: 10 polls per HTTP call
	var delay_Int = delay_Tout / 10;  // milliseconds
	var anIntervalObject;
	var nCalls_cbf_Int = 0;

	function cbf_Int( a1, a2 ) 
	{
		if( DEBUG )
		{
		nCalls_cbf_Int++;
		console.log( 'In cbf_Int nCalls_cbf_Int = %d', nCalls_cbf_Int );
		// console.log( 'a1 = \"' + a1 + '\"' );
		// console.log( 'a2 = \"' + a2 + '\"' );
		// console.log( 'Do a loop' );
		}
		cb_poll_for_records_available();
	}

	var cbf_Int_arg_1 = 1, cbf_Int_arg_2 = 2;

	// setInterval(callback, delay, [arg], [...])
	anIntervalObject = setInterval( cbf_Int, delay_Int,
									cbf_Int_arg_1, cbf_Int_arg_2 );


	// set up for call to read_gcl_records()
	var gdpd_addr      = null;  // default "127.0.0.1:2468"
	                            // name of new gcl is already in gcl_name
    var gcl_firstrec   = recno; // multiple records to read here
	var gcl_numrecs    = nrecs;
    var gcl_subscribe  = true;  // for this request we are subscribing
								// to writes on this gcl
	var gcl_multiread  = false;
	var recdest        = 0;     // read the gcl records into the rrv Array
	// DEBUG
	// var recdest        = -1;    // write the gcl records to stdout
	var conout         = false; // no output to console for Array return
	var wait_for_events= false; // do not wait indefinitely for gdp events
	var rrv;

	// TBD is there really any use for this (synchronous?) JS callback?
	function gdp_event_cbfunc( gdp_event_type, datum, recarray_out )
	{
		if( DEBUG )
		{
		console.log( 'In gdp_event_cbfunc: gdp_event_type = ', gdp_event_type,
		             ' recarray_out = \"', recarray_out, '\"' );
		// DEBUG LEFT OFF
//C  	fprintf(stdout, " >>> ");A
		// See process.stdout.write comments above in do_simpleread() .
		process.stdout.write( " >>> " );
		fflush_all_js();  // sometimes Node.js may not empty buffers
		gdp_datum_print_stdout_js( datum );
		}
	}


	function cb_poll_for_records_available()
	{
    /* Returns:
	   { error_code: EP_STAT,
	     records: Array of Strings, each element with opaque record content
	   }
     */
    rrv = read_gcl_records( gdpd_addr, gcl_name,
                            gcl_firstrec, gcl_numrecs,
                            gcl_subscribe, gcl_multiread, recdest,
							conout,
							gdp_event_cbfunc,
							wait_for_events
                          );
	// recs_read = recs_read "concatenated with" rrv.records;
	Array.prototype.push.apply( recs_read, rrv.records );
	if( DEBUG )
	{
	console.log( 'monitoring_done = ', monitoring_done );
	console.log( 'rrv.records = \"', rrv.records, '\"' );
	console.log( 'recs_read = \"', recs_read, '\"' );
	}
	// LEFT OFF HERE - if we get any records here should we stop polling
	// and send back a response: shutdown both timers and call cb_finalize() ?
	if ( rrv.records.length != 0 )
	{
		// TBD any particular ordering on these clear's; e.g., race conditions
		// with the timers going off ?
		clearTimeout( aTimeoutObject );
		// LEFT OFF or just call: cbf_Tout() ?
		clearInterval( anIntervalObject );
		monitoring_done = true;
		if( DEBUG )
		{
		console.log( 'monitoring_done = \"' + monitoring_done + '\"' );
		}
	    cb_finalize();
		
	}
	}
	
	function cb_finalize()
	{
	// TBD: error checking after call: bad gdpd_addr, gcl_name, record numbers
	// TBD: Content-Type of data written to record not available from a gcl.
	//      This seems to be per record metadata or is the type
	//      the same for all the records in a given gcl?
	// TBD: any parameters on 'application/octet-stream' - like a limit?
	// res.set( 'Content-Type',         'application/octet-stream' );
	// TBD: We're being creative here -- docs need some work on Content_Type
	// Can only set header once:
	res.set( 'Content-Type', 'application/json; type=gdp/response' );
	if ( true )  // LEFT OFF HERE - moved from callback above
	{
	// one or more records to return for this request
	if( DEBUG )
	{
	console.log( 'rrv.records = \"', rrv.records, '\"' );
	}
	// Note, res.json actually sends back a response; can only do this once.
	res.json( recs_read );
	}
	}


	// OK, just return for now -- timer callbacks associated with
	//   aTimeoutObject = setTimeout()  and  anIntervalObject = setInterval()
	// will gather records, assemble them into recs_read[], and ship off
	// the HTTP response, res.json() .
	// TBD is there any way the timers could fail to do their callbacks?

});


