/* vim: set ai sw=4 sts=4 ts=4 : */

// Simple HTTP server which runs the test program rw_supt_test.js
// when its page localhost:8888/ is hit with a GET.
// 2014-11-04
// Alec Dara-Abrams

// Start this server in a Terminal window with:
//   node server_running_rw_supt_test.js
// Open a browser in another window on:
//   localhost:8888
// Terminal window outputs: "Calling run_rw_supt_test()"
// Manually enter gcl records in the Terminal window ending with a ^D
// The Terminal window will list the progress of the test.
// The browser window will give an indication that the test has been run.
// ^C in the Terminal window to terminate the server.

// TBD: Seems to be a problem on the second refresh of the browser.
//      After the first invocation, stdin always gives an EOF indication.


var http = require("http");

http.createServer(function (request, response) {
    response.writeHead(200, {
        "Content-Type": "text/plain"
    });
    response.write(
        "Ran the test program rw_supt_test.js;"
    );
    console.log("Calling run_rw_supt_test()");
    run_rw_supt_test();
    response.end();
}).listen(8888);



/* vim: set ai sw=4 sts=4 ts=4 : */

// This is a brief test of the read/write support functions in the
// file gdpjs/rw-supt.js .  It should be run in lang/js/apps/ .


// Usage (via the Node.js runtime): 
//    node rw_supt_test.js

// Load Node.js modules for calling foreign functions -- C functions here.
// See libgdp_h.js for details and assumptions about the directory
// where Node.js modules must be placed.
//
// Parameters needed for libgdp_h.js and gdpjs_supt.js
// They MUST be adapted to the directory where this program will be run.
// See libgdp_h.js for details.
var GDP_DIR = "../../../";
var GDPJS_DIR = GDP_DIR + "./lang/js/gdpjs/";
var NODE_MODULES_DIR = "";
//
var LIBGDP_H_DIR = GDPJS_DIR;
// Here we include and evaluate our shared GDP Javascript "header file",
// 'libgdp_h.js', within the global scope/environment.
var fs = require('fs'); // Node.js's built-in File System module
eval(fs.readFileSync(LIBGDP_H_DIR + 'libgdp_h.js').toString());
// We similarly include & evaluate some JS support functions.
eval(fs.readFileSync(LIBGDP_H_DIR + 'gdpjs_supt.js').toString());
eval(fs.readFileSync(LIBGDP_H_DIR + 'rw_supt.js').toString());


function run_rw_supt_test() {

    var brk = "==============================================================";
    brk = "\n" + brk + "\n";

    // =======================================================================
    // Set up calls to write_gcl_records()

    // Common hard-wired arguments
    gdpd_addr = null; // default "127.0.0.1:2468"
    gcl_name = null; // default; create a new gcl
    gcl_append = false;
    // gcl_append = true;   // TBD: use an existing gcl

    // Test calls to write_gcl_records()
    console.log(brk);

    // A: write to gcl from stdin
    // recsrc = -1;  // read the gcl records to be written from stdin with
    //               // prompts to and echoing for the user on stdout
    // recarray = [ ];
    recsrc = -1; // read the gcl records to be written from stdin...
    recarray = []; // not used for recsrc = -1
    write_gcl_records(gdpd_addr, gcl_name, gcl_append, recsrc, recarray);
    console.log(brk);

    // B: write to gcl from JS Array
    // recsrc =  0;  // read the gcl records from the Array recarray
    // recarray = [ "Item 01 - from recarray", "Item 02", "Item 03" ];
    recsrc = 0; // read the gcl records from the Array recarray
    recarray = ["Item 01 - from recarray", "Item 02", "Item 03"];
    write_gcl_records(gdpd_addr, gcl_name, gcl_append, recsrc, recarray);
    console.log(brk);


    // C: write to gcl N records with integers as contents
    // recsrc >  0;  // write recsrc records with automatically generated
    //               // content: the integers starting at 1 and going up to
    //               // recsrc, inclusive.
    // recsrc   =  7;
    // recarray = [ ];
    recsrc = 7;
    recarray = [];
    write_gcl_records(gdpd_addr, gcl_name, gcl_append, recsrc, recarray);
    console.log(brk);


    // =======================================================================
    // Set up calls to read_gcl_records()

    // TBD

} /* end function run_rw_supt_test() */


// Invoke the test
// run_rw_supt_test();
