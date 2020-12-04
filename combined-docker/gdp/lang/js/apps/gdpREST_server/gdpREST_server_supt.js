/* vim: set ai sw=4 sts=4 ts=4 : */

// Support JS functions and variables for the
// Node.js-Based JavaScript server to provide a REST interface to the GDP
//
// Alec Dara-Abrams
// 2014-11-06
//
// TBD: Copyright, clean up code; regularize indenting with JS-aware tool
//      Check for possible error returns from libgdp calls.


// Documentation:
//   TBD - but see gdpREST_server.js

// Assumptions:
//   TBD - if even necessary given the descriptions of individual functions
//   & variables below.
//
//   Avoid any dependencies on:
//      'libgdp_h.js' 'gdpjs_supt.js' 'rw_supt.js'


// =============================================================================
// GDP-related Initialization
//
function gdp_rest_init( gdp_deamon_host_port )
{
	// There may be more initialization for the REST interface so we
	// use this function as a placeholder.

	// Big TBD: handle errors here, especially, no GDP daemon running
	//          add 
	gdp_init_js( gdp_deamon_host_port );
}


// =============================================================================
// Parse a request into its components (not all components may be present)
// TBD: this is a very rough version.  Not all HTTP components are provided
//      and some Node.js & Express-derived components are also provided.
//      See comments below for details.
//
//   For spec-level details see RFC 723x ToC at http://www.w3.org/Protocols/
//
//   Grammar fragments from: http://tools.ietf.org/html/rfc7230
//
//     HTTP-message   = start-line
//                      *( header-field CRLF )
//                      CRLF
//                      [ message-body ]
//     start-line     = request-line / status-line
//
//     request-line   = method SP request-target SP HTTP-version CRLF
//     request-target = origin-form / absolute-form
//                      / authority-form / asterisk-form
//     origin-form    = absolute-path [ "?" query ]        ; most common form
//	   header-field   = field-name ":" OWS field-value OWS
//     message-body   = *OCTET
//
//     status-line    = HTTP-version SP status-code SP reason-phrase CRLF
//     status-code    = 3DIGIT
//     reason-phrase  = *( HTAB / SP / VCHAR / obs-text )
//  ; The most common form of request-target is the origin-form.
//     origin-form    = absolute-path [ "?" query ]
//
// Note, from http://tools.ietf.org/html/rfc3986#section-3 
//     URI         = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
//     hier-part   = "//" authority path-abempty / path-absolute
//                   / path-rootless / path-empty
//
//
/* Returns: { *( req_component_name: req_component_value, ) } */
function get_request_components( r )
{
	// DEBUG
	// console.log( 'In get_request_components() r = \n{======', r, '\n======}' );
	var rv =
	{
		// HTTP request proper components
		"method":		r.method,        /*    method            */
		"originalUrl":	r.originalUrl,   /* ~~ request-target    */
		"httpVersion":	r.httpVersion,   /* ~~ HTTP-version      */
		"headers":		r.headers,       /* ~~ *( header-field ) */
		//
		// Note: r.body is {} even for POST's for which Postman
		// has placed "raw" octet data in the POST's body.
		// With Express, evidently, we have to explicitly bind middleware
		// to app to pull out and process the HTTP request's body (even
		// plain octets) into the Express request's .body -- the .body we
		// see here.  This middleware will be parameterized by some
		// combination of the request's Content_Type header and a type
		// indicator passed into the middleware at app binding time.
        // E.g., app.use(bodyParser.raw({ type: "application/octet-stream",
		// limit: "10mb" })).
		//
		"body":			r.body,          /*    [ message-body ]  */

		// Node.js and Express components derived from HTTP request proper
		"url":			r.url,        // TBD: doc these req fields
		"params":		r.params,
		"query":		r.query,
		"route":		r.route,
		"path":			r.path,
		"hostname":		r.hostname,
		"trailers":		r.trailers,
		"statusCode":	r.statusCode,
		"_parsedURL":	r._parsedURL,
	};
	return rv;


// Node.js Express's documentation is sketchy so we take an empirical approach.///
// Some other fields in rv to look at:
//    socket:
//	   {
//	     parser:
//          { _headers: [],
//            _url: '',
//            onHeaders: [Function: parserOnHeaders],
//            onHeadersComplete: [Function: parserOnHeadersComplete],
//            onBody: [Function: parserOnBody],
//            onMessageComplete: [Function: parserOnMessageComplete],
//            socket: [Circular],
//            incoming: [Circular],
//            maxHeaderPairs: 2000,
//            onIncoming: [Function] },
//	   }
//	  connection:
//	   {
//         parser:
//          { _headers: [],
//            _url: '',
//            onHeaders: [Function: parserOnHeaders],
//            onHeadersComplete: [Function: parserOnHeadersComplete],
//            onBody: [Function: parserOnBody],
//            onMessageComplete: [Function: parserOnMessageComplete],
//            socket: [Circular],
//            incoming: [Circular],
//            maxHeaderPairs: 2000,
//            onIncoming: [Function] },
//	   }
//

} /* end function get_request_components() */


// =============================================================================
// Return a brief human-readable version of the request req as a String
// More verbose version, but currently incomplete, in get_request_components()
//
/* String */
function req_asString( req )
{
	var rv = 'HTTP request: ' + req.method +
	         ' sent to ' + req.headers.host + req.originalUrl;	
	// Note: req.headers.host is only HTTP-level convention - not TCP/IP truth

	// DEBUG - a Node.js/JS experiment on hold here
	// We can't seem to test for a req.body that prints as "{}".
	if ( false )
	{
	var empty_body_1 = (req.body == {});
	var empty_body_2 = (req.body == null);
	var empty_body_3 = (req.body);

	console.log( 'empty_body_1 = ', empty_body_1 );
	if ( empty_body_1 ) { console.log("empty_body_1 true"); }
	else { console.log("empty_body_1 false"); }

	console.log( 'empty_body_2 = ', empty_body_2 );
	if ( empty_body_2 ) { console.log("empty_body_2 true"); }
	else { console.log("empty_body_2 false"); }

	console.log( 'empty_body_3 = ', empty_body_3 );
	if ( empty_body_3 ) { console.log("empty_body_3 true"); }
	else { console.log("empty_body_3 false"); }

	console.log( 'empty_body_3 = ', empty_body_3 );
	if ( empty_body_3 ) { console.log("empty_body_3 true"); }
	else { console.log("empty_body_3 false"); }

	console.log('A: req.body = \"' + req.body + '\"' );	
	console.log('B: req.body = \"', req.body, '\"' );	
	}

	return rv;
}


// =============================================================================
// Return an Array of Stings containing the Base64 encoded names of all
// the GCL's available on this system
//
// If gdp_gcl_path is not present or undefined, then use the default path
// for the local GDP's GCL directory; otherwise, use gdp_gcl_path as the
// path for that directory.
// Returns null if there are any errors; e.g., error on file/directory read.
//
// Note, this only returns reliable results if the, currently, one-and-only-one
// GDP daemon is running locally.  Additionally, this function is dependent
// on internal GDP GCL directory and filename storage details.
// TBD: Note comments on hard-wired path name GDP_GCL_ABSPATH.
// TBD: Better method of error return and better error info.

/* Array of Strings | null */
function get_gcl_base64_namesSync( gdp_gcl_path )
{
	// This GET REST API awaits support by libgdp. We rely on internal
	// implementation detail here.
	// E.g., location of GCL directory and one .data file per GCL.
	//
	// TBD: un-hard-wire this.  Can, at least, the default value be gotten
	//      from libgdp or, more generally, from the currently running local
	//      gdp daemon(s)?
	var GDP_GCL_ABSPATH = '/var/tmp/gcl';
	var gcl_files = "";  // TBD: make sure this falls through on errors

    if( (typeof gdp_gcl_path) === 'undefined' )
	{ gdp_gcl_path = GDP_GCL_ABSPATH; }

	// TBD: consider asynch??
	try
	{	// TBD: check for error on fs.readdirSync;
		//      e.g., Error: ENOENT, no such file or directory
		gcl_files = fs.readdirSync( gdp_gcl_path );
	} catch(e)
	{	// gcl_files will be ""
		// DEBUG
		// console.log("name:" + e.name + "\nmessage:" + e.message)
		// console.log( 'gcl_files = ' + gcl_files );
		return null;
	}
	// will hold gcl file names changed to gcl Base64 encoded String names
	var gcl_names = [];
	var j = 0;

    for ( i = 0; i < gcl_files.length; i++ )
	{
		if ( /.*\.data$/.test(gcl_files[i]) )
		{   // only consider the gcl .data files
			gcl_names[j] = gcl_files[i].replace( /\.data$/, '' ); // strip .data
			j++;
		}
	}
	return gcl_names;
}


// =============================================================================
