/* vim: set ai sw=4 sts=4 ts=4 : */

// Node.js Javascript version of gdp/apps/writer-test.c


// Usage (via the Node.js runtime): 
//    node writer-test.js [-a] [-D dbgspec] [-G gdpd_addr] <gcl_name>

// Provenance:
//
// This file, writer-test.js, is translated by hand from writer-test.c
// In the below let <This file> = "writer-test".
//
// Some lines of C code (including all their comments & blank likes) from
// <This file>.c are prefixed with "//C  ".
// Regular JS comments in write-test.js are prefixed with "//".
//
// If you find the included C code annoying, consider removing these lines
// with something like:
//    sed  -e '/^\/\/C/d' <This file>.js

var mod_getopt = require('posix-getopt');
var os = require('os');
var path = require('path');

try {
    var gdpjs = require('gdpjs');
} catch (exception) {
    var gdpjs = require('../gdpjs/gdpjs');
}

// Load Node.js modules for calling foreign functions -- C functions here.
// See libgdp_h.js for details and assumptions about the directory
// where Node.js modules must be placed.
//
// Parameters needed for libgdp_h.js and gdpjs_supt.js
// They MUST be adapted to the directory where this program will be run.
// See libgdp_h.js for details.
//var GDP_DIR = "../../../";
//var GDPJS_DIR = GDP_DIR + "./lang/js/gdpjs/";
//var NODE_MODULES_DIR = "";
//
//var LIBGDP_H_DIR = GDPJS_DIR;
// Here we include and evaluate our shared GDP Javascript "header file",
// 'libgdp_h.js', within the global scope/environment.
//var fs = require('fs'); // Node.js's built-in File System module
//eval(fs.readFileSync(LIBGDP_H_DIR + 'libgdp_h.js').toString());
// We similarly include & evaluate some JS support functions.
//eval(fs.readFileSync(LIBGDP_H_DIR + 'gdpjs_supt.js').toString());
//eval(fs.readFileSync(LIBGDP_H_DIR + 'rw_supt.js').toString());


//C  /*
//C  **  WRITER-TEST --- writes records to a GCL
//C  **
//C  **		This reads the records one line at a time from standard input
//C  **		and assumes they are text, but there is no text requirement
//C  **		implied by the GDP.
//C  */

function main(argc, argv) {
    //C  	gdp_gcl_t *gclh;
    var gcl_Ptr; // gclh
    //C  	gcl_name_t gcliname;
    //var gcliname = ref.alloc(gcl_name_t);
    //C  	int opt;
    var opt;
    var opt_parser;
    //C  	EP_STAT estat;
    //?? make sure this JS estat can hold & allow access to gdp EP_STAT's
    // which, as of 2014-10-24, are 32-bit unsigned ints (uint32_t).
    // See ep/epstat.h .
    var estat;
    //C  	char *gdpd_addr = NULL;
    // <host:port> for the GDP Daemon - a String
    // Command line option -G <host:port> ; e.g. -G 127.0.0.1:2468
    // "127.0.0.1:2468" is the default port for a local gdpd
    var gdpd_addr = null;
    //C  	bool append = false;
    var append = false;
    //C  	char *xname = NULL;
    // gets optional name of an existing GCL; if null, generate a new GCL
    var xname = null;
    //C  	char buf[200];
    // Moved earlier before first use:
    //   var char_t = ref.types.char;
    //   var buf_t = ref_array(char_t);
    //var buf_tPtr = ref.refType(buf_t); // not currently used
    // Note, buf is re-allocated below for each item string read from stdin
    //var buf = new buf_t(10);
    //C  	bool show_usage = false;
    var show_usage = false;
    var usage_error = ""; // more detail for some errors in argc/argv

    console.log("writer-test2.js: main()" + argc);

    // Some useful variants for libep printf debugging output
    // Needed for the -D option below.
    gdpjs.ep_dbg_init();
    // ep_dbg_set_js("*=99");  // start general debug output from libgdp

    //C  	// collect command-line arguments
    opt_parser = new mod_getopt.BasicParser("aD:G:", argv);
    while ((opt = opt_parser.getopt()) !== undefined) {
        // Examples of opt_parser.getopt() results:
        // -a        ==> opt = { option: 'a' }
        // -D '*=40' ==> opt = { option: 'D', optarg: '*=40' }
        // -G '127.0.0.1:2648' ==>
        //        opt = { option: 'G', optarg: '127.0.0.1:2648' }
        // -x        ==> opt = { option: '?', optopt: 'x', error: true }

        switch (opt.option) {
        case 'a':
            append = true;
            break;
        case 'D':
            // turn on debugging
            // Note, ep_dbg_init_js() was called far above.
            // Will gdp_init() also call ep_dbg_init()??
            // For some reason we don't get libgdp debug print output??
            gdpjs.ep_dbg_set_js(opt.optarg);
            break;
        case 'G':
            // set the port for connecting to the GDP daemon
            gdpd_addr = opt.optarg;
            break;
        default:
            show_usage = true;
            break;
        } /* end switch */
    } /* end while */
    //C  	argc -= optind;
    argc -= opt_parser.optind();
    //C  	argv += optind;
    var argv_index = opt_parser.optind();

    //C  	// name is optional for a new GCL; if omitted one will be created
    console.log("writer-test2: argc: " + argc);
    if (argc > 0) {
        xname = argv[argv_index];
        argc--;
        argv_index++; //?? not sure why argv is incremented here
        // Also, parsing here can accept zero or more
        // "names"; e.g., writer-test.js name1 name2 name3
    }

    if (show_usage || (argc != 0) || (append && xname == null)) {
        //C  		fprintf(stderr, "Usage: %s [-a] [-D dbgspec] [-G gdpd_addr] [<gcl_name>]\n"
        //C  				"  (name is required for -a)\n",
        //C  				ep_app_getprogname());
        if (argc != 0) {
            usage_error += "\nOnly one <gcl_name> allowed.";
        }
        if (append && xname == null) {
            usage_error += "\nMissing <gcl_name> with -a.";
        }
        console.error(
            "Usage: %s [-a] [-D dbgspec] [-G gdpd_addr] [<gcl_name>]%s",
            path.basename(process.argv[1]), usage_error + "\n");
        //C  		exit(EX_USAGE);
        process.exit(EX_USAGE);
    }

    // Example set ups for calls to write_gcl_records()
    //
    // A: write to gcl from stdin
    // recsrc   = -1;   // read from stdin the gcl records to be written,
    //                  // with optional prompts to and echoing for the
    //                  // user on stdout.
    // conout   = true; // do prompt and echo to stdout.
    // recarray = [ ]; recarray_out = []; // ignored for recsrc = -1
    //
    // B: write to gcl from JS Array, recarray[]
    // recsrc   =  0;      // read the gcl records from the Array recarray[]
    // recarray = [ "Item 01 - from recarray", "Item 02", "Item 03" ];
    // conout   = false;   // don't echo to console.log()
    // recarray_out = [];  // will hold recno's and timestamps for newly
    //                     // written records
    //
    // C: write to gcl N records with integers as contents
    // recsrc >  0;     // write recsrc records with automatically generated
    //                  // content: the integers starting at 1 and going
    //                  // up to recsrc, inclusive.
    // recsrc =  7;
    // conout = false;  // don't echo to console.log()
    // recarray = [ ]; recarray_out = []; // ignored for recsrc > 0
    //
    // write_gcl_records( gdpd_addr, gcl_name, gcl_append, recsrc,
    //                    recarray, conout, recarray_out
    //                  );

    // Set up our particular call to write_gcl_records()
    gdpd_addr = gdpd_addr;
    gcl_name = xname;
    logdxname = os.hostname();
    gcl_append = append;

    recsrc = -1; // read the gcl records to be written from stdin..
    recarray = []; // not used for recsrc = -1
    conout = true; // echo to stdout == console.log()
    recarray_out = []; // not used for recsrc = -1
    /*  Returns:
        { error_isok: false|true, error_code: EP_STAT, error_msg: String,
        gcl_name: String
        }
    */
    console.log("writer-test2.main(): about to call write_gcl_records");
    gdpjs.write_gcl_records(gdpd_addr, gcl_name, logdxname, gcl_append, recsrc,
                            recarray, conout, recarray_out
                           );
    // TBD: return ( ! ep_stat_isok_js(estat) );

} /* end function main() */

// Use the familiar C/*NIX names
var argc = process.argv.length;
var argv = process.argv;

main(argc, argv);
