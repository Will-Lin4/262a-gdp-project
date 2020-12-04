/* vim: set ai sw=4 sts=4 ts=4 : */

// Node.js Javascript version of gdp/apps/reader-test.c


// Usage (via the Node.js runtime): 
//    node reader-test.js [-D dbgspec] [-f firstrec] [-G gdpd_addr] [-m] [-n nrecs] [-s] <gcl_name>

// Provenance:
//
// This file, reader-test.js, is translated by hand from reader-test.c
// In the below let <This file> = "reader-test".
//
// Some lines of C code (including all their comments & blank likes) from
// <This file>.c are prefixed with "//C  ".
// Regular JS comments in write-test.js are prefixed with "//".
//
// If you find the included C code annoying, consider removing these lines
// with something like:
//    sed  -e '/^\/\/C/d' <This file>.js


// debugging:
var fs = require('fs'),
    path = require('path'),
    mod_getopt = require('posix-getopt');
var dirString = path.dirname(fs.realpathSync(__filename));
console.log('working directory:', dirString);

//try {
    var gdpjs = require('@terraswarm/gdp');
//} catch (exception) {
//    var gdpjs = require('../gdpjs/gdpjs');
//}

// Load Node.js modules for calling foreign functions -- C functions here.
// See libgdp_h.js for details and assumptions about the directory
// where Node.js modules must be placed.
//
// Parameters needed for libgdp_h.js and gdpjs_supt.js
// They MUST be adapted to the directory where this program will be run.
// See libgdp_h.js for details.
// var GDP_DIR = process.env.GDP_DIR;
// if (GDP_DIR == undefined) GDP_DIR = "../../..";
// var GDPJS_DIR = GDP_DIR + "/lang/js/gdpjs/";
// var NODE_MODULES_DIR = "";
// //
// var LIBGDP_H_DIR = GDPJS_DIR;
// // Here we include and evaluate our shared GDP Javascript "header file",
// // 'libgdp_h.js', within the global scope/environment.
// var fs = require('fs'); // Node.js's built-in File System module
// eval(fs.readFileSync(LIBGDP_H_DIR + 'libgdp_h.js').toString());
// // We similarly include & evaluate some JS support functions.
// eval(fs.readFileSync(LIBGDP_H_DIR + 'gdpjs_supt.js').toString());
// eval(fs.readFileSync(LIBGDP_H_DIR + 'rw_supt.js').toString());


//C  /*
//C  **  READER-TEST --- read and prints records from a GCL
//C  **
//C  **		This makes the naive assumption that all data values are ASCII
//C  **		text.  Ultimately they should all be encrypted, but for now
//C  **		I wanted to keep the code simple.
//C  **
//C  **		There are two ways of reading.  The first is to get individual
//C  **		records in a loop, and the second is to request a batch of
//C  **		records; these are returned as events that are collected after
//C  **		the initial command completes.  There are two interfaces for
//C  **		this; one only reads existing data, and the other will wait for
//C  **		data to be published by another client.
//C  */


if (false) // Turn off local do_simpleread() and do_multiread() fcns
{
    //C  /*
    //C  **  DO_SIMPLEREAD --- read from a GCL using the one-record-at-a-time call
    //C  */
    //C  
    //C  EP_STAT
    //C  do_simpleread(gdp_gcl_t *gclh, gdp_recno_t firstrec, int numrecs)
    //C  {
    // Note, firstrec is a JS Number not a ref gdp_recno_t and, similarly,
    //       numrecs is a JS Number not a ref int
    /* EP_STAT */
    function do_simpleread(gclh, firstrec, numrecs) {
        console.log("reader-test.js: do_simple_read(, " + firstrec + ", " + numrecs);
        //C  	EP_STAT estat = EP_STAT_OK;
        // ?? make sure this can hold & allow access to gdp EP_STAT's
        var estat = /* EP_STAT */ ep_stat_ok_js();
        //C  	gdp_recno_t recno;
        // but is recno's type correct for gdp_gcl_read() below?
        // Don't seem to work:
        //var recno = ref.alloc( gdp_recno_t, firstrec ); //?? check this
        //var recno = ref.alloc( gdp_recno_t, 0 ); //?? check this
        // OLD var recno = firstrec;
        // Just settle for a JS var and hope for the best - seems to work
        var recno;
        //C  	gdp_datum_t *datum = gdp_datum_new();
        var datum;
        datum = gdp_datum_new_js();
        //C  
        //C  	// change the "infinity" sentinel to make the loop easier
        if (numrecs == 0) numrecs = -1;
        //C  
        //C  	// can't start reading before first record (but negative makes sense)
        if (firstrec == 0) firstrec = 1;
        //C  
        //C  	// start reading data, one record at a time
        recno = firstrec;
        while (numrecs < 0 || --numrecs >= 0) {
            //C  		// ask the GDP to give us a record
            console.log("reader-test.js: do_simple_read(): about to call gdp_gcl_read_js(," + recno);
            estat = gdp_gcl_read_js(gclh, recno, datum);
            //C  
            //C  		// make sure it did; if not, break out of the loop
            if (!ep_stat_isok_js(estat)) {
                break;
            }
            //C  
            //C  		// print out the value returned
            //C  		fprintf(stdout, " >>> ");
            // Note, when writing to a file rather that a terminal writes using
            // process.stdout.write() will not be synchronized with writes 
            // using console.log()
            process.stdout.write(" >>> ");
            // Tried an explicit fflush() here but Node.js prints the ' >>> '
            // and then crashes, a la:  ' >>> Bus error: 10'
            //   libc.fflush();
            // For some reason this call to libgdpjs.fflush_all() works:
            fflush_all_js(); // sometimes Node.js may not empty buffers
            //C  		gdp_datum_print(datum, stdout);
            gdp_datum_print_stdout_js(datum);
            //C  
            //C  		// move to the next record
            recno++;
            //C  
            // For now, we live dangerously & leave any flushing to the next guy
            // TBD??
            //C  		// flush any left over data
            //C  		if (gdp_buf_reset(gdp_datum_getbuf(datum)) < 0)
            //C  		{
            //C  			char nbuf[40];
            //C  
            //C  			strerror_r(errno, nbuf, sizeof nbuf);
            //C  			printf("*** WARNING: buffer reset failed: %s\n",
            //C  					nbuf);
            //C  		}
        } /* while */
        //C  
        //C  	// end of data is returned as a "not found" error: turn it into a warning
        //C  	//    to avoid scaring the unsuspecting user
        if (ep_stat_is_same_js(estat, gdp_stat_nak_notfound_js())) {
            estat = ep_stat_end_of_file_js();
        }
        return estat;
    } /* end do_simpleread() */
    //C  }
    //C  
    //C  
    //C  /*
    //C  **  DO_MULTIREAD --- subscribe or multiread
    //C  **
    //C  **		This routine handles calls that return multiple values via the
    //C  **		event interface.  They might include subscriptions.
    //C  */
    //C  
    //C  EP_STAT
    //C  do_multiread(gdp_gcl_t *gclh, gdp_recno_t firstrec, int32_t numrecs, bool subscribe)
    //C  {
    // Note, firstrec is a JS Number not a ref gdp_recno_t and, similarly,
    //       numrecs is a JS Number not a ref int32_t
    /* EP_STAT */
    function do_multiread(gclh, firstrec, numrecs, subscribe) {
        //C  	EP_STAT estat;
        var estat; //?? make sure this can hold & allow access to gdp EP_STAT's
        //C  
        if (subscribe) {
            //C  		// start up a subscription
            estat = gdp_gcl_subscribe_no_timeout_no_callback_js(
                gclh, firstrec, numrecs);
        } else {
            //C  		// make the flags more user-friendly
            if (firstrec == 0) firstrec = 1;
            //C  
            //C  		// start up a multiread
            //C  		estat = gdp_gcl_multiread(gclh, firstrec, numrecs, NULL, NULL);
            estat = gdp_gcl_multiread_no_callback_js(gclh, firstrec, numrecs);
        }
        //C  
        //C  	// check to make sure the subscribe/multiread succeeded; if not, bail
        if (!ep_stat_isok_js(estat)) {
            //C  		char ebuf[200];
            //C  
            //C  		ep_app_abort("Cannot %s:\n\t%s",
            //C  				subscribe ? "subscribe" : "multiread",
            //C  				ep_stat_tostr(estat, ebuf, sizeof ebuf));
            // No need for ep_app_abort's PRINTFLIKE behavior here - just
            // give an error msg and exit.
            // ??consider var ebuf = new buf_t(100);
            var ebuf = new Array(200 + 1).join(" "); // long enough??
            console.error("Cannot %s:\n\t%s",
                subscribe ? "subscribe" : "multiread",
                ep_stat_tostr_js(estat, ebuf, ebuf.length)
            );
            fflush_all_js(); // sometimes Node.js may not empty buffers
            process.exit(1); // could have better error code; e.g., EX_USAGE
        }
        //C  
        //C  	// now start reading the events that will be generated
        for (;;) {
            //C  		// get the next incoming event
            var gev_Ptr; // gev
            gev_Ptr = gdp_event_next_js(true);
            var evtype_int = gdp_event_gettype_js(gev_Ptr);
            //C  
            //C  		// decode it
            switch (evtype_int) {
            case GDP_EVENT_DATA:
                //C  			// this event contains a data return
                //C  			fprintf(stdout, " >>> ");A
                // See process.stdout.write comments above in do_simpleread() .
                process.stdout.write(" >>> ");
                fflush_all_js(); // sometimes Node.js may not empty buffers
                var datum = gdp_event_getdatum_js(gev_Ptr);
                gdp_datum_print_stdout_js(datum);
                break;
                //C  
            case GDP_EVENT_EOS:
                //C  			// "end of subscription": no more data will be returned
                //C  			fprintf(stdout, "End of %s\n",
                //C  					subscribe ? "Subscription" : "Multiread");
                console.log("End of %s", subscribe ? "Subscription" : "Multiread");
                return estat;
                //C  
            default:
                //C  			// should be ignored, but we print it since this is a test program
                //C  			fprintf(stderr, "Unknown event type %d\n", gdp_event_gettype(gev));
                console.error("Unknown event type %d\n",
                    gdp_event_gettype_js(gev_Ptr));
                fflush_all_js(); // sometimes Node.js may not empty buffers
                //C  
                //C  			// just in case we get into some crazy loop.....
                sleep.sleep(1);
                break;
            } /* switch */
            //C  
            //C  		// don't forget to free the event!
            gdp_event_free_js(gev_Ptr);
        } /* for (;;) */
        //C  	
        //C  	// should never get here
        return estat;
    } /* do_multiread() */
    //C  }
} /* end if (false)  // Turn off local do_simpleread() and do_multiread() */

//C  
//C  
//C  /*
//C  **  MAIN --- the name says it all
//C  */
//C  
//C  int
//C  main(int argc, char **argv)
//C  {
function main(argc, argv) {
    // // Use the familiar C/*NIX names
    // var argc = process.argv.length;
    // var argv = process.argv;

    //C  	gdp_gcl_t *gclh;
    var gcl_Ptr; // gclh
    //C  	EP_STAT estat;
    //?? make sure this JS estat can hold & allow access to gdp EP_STAT's
    // which, as of 2014-10-24, are 32-bit unsigned ints (uint32_t).
    // See ep/epstat.h .
    var estat;
    //C  	char buf[200];
    //C  	gcl_name_t gclname;
    // Why doesn't this work??  var gclname = new gcl_name_t(32);
    // var gclname = ref.alloc(gcl_name_t);
    // Now it seems to work 2014-10-26
    //var gclname = new gcl_name_t(32);
    //C  	gcl_pname_t gclpname;
    // var gclpname = ref.alloc(gcl_pname_t);
    // Now try same as for gclname for gclpname - Seems to work 2014-10-26
    //var gclpname = new gcl_pname_t(GDP_GCL_PNAME_LEN);

    //C  	int opt;
    var opt;
    var opt_parser;
    //C  	char *gdpd_addr = NULL;
    // <host:port> for the GDP Daemon - a String
    // Command line option -G <host:port> ; e.g. -G 127.0.0.1:2468
    // "127.0.0.1:2468" is the default port for a local gdpd
    var gdpd_addr = null;
    //C  	bool subscribe = false;
    var subscribe = false;
    //C  	bool multiread = false;
    var multiread = false;
    //C  	int32_t numrecs = 0;
    // ?? ffi/ref Doesn't work to try to ref.alloc type numrecs below
    //      var int32_t = ref.types.int32;
    //      var numrecs = ref.alloc( int32_t, 2 ); //?? check this
    var numrecs = 0;
    //C  	gdp_recno_t firstrec = 0;
    // ?? ffi/ref Doesn't work to try to ref.alloc type firstrec below
    // var firstrec = ref.alloc( gdp_recno_t, 1 );
    var firstrec = 0; // 1 would be a better default record # than 0
    //C  	bool show_usage = false;
    var show_usage = false;
    var usage_error = ""; // more detail for some errors in argc/argv


    // Some useful variants for libep printf debugging output
    // Needed for the -D option below.
    gdpjs.ep_dbg_init();
    // ep_dbg_set_js("*=99");  // start general debug output from libgdp

    //C  
    //C  	// parse command-line options
    opt_parser = new mod_getopt.BasicParser("D:f:G:mn:s", argv);
    //C  	while ((opt = getopt(argc, argv, "D:f:G:mn:s")) > 0)
    //C  	{
    while ((opt = opt_parser.getopt()) !== undefined) {
        // Examples of opt_parser.getopt() results:
        // -D '*=40' ==> opt = { option: 'D', optarg: '*=40' }
        // -f 123    ==> opt = { option: 'f', optarg: '123' }
        // -G '127.0.0.1:2648' ==>
        //        opt = { option: 'G', optarg: '127.0.0.1:2648' }
        // -m        ==> opt = { option: 'm' }
        // -n 456    ==> opt = { option: 'n', optarg: '456' }
        // -s        ==> opt = { option: 's' }
        // -x        ==> opt = { option: '?', optopt: 'x', error: true }

        switch (opt.option) {
        case 'D':
            //C  			// turn on debugging
            // Note, ep_dbg_init_js() was called far above.
            // Will gdp_init() also call ep_dbg_init()??
            // For some reason we don't get libgdp debug print output??
            gdpjs.ep_dbg_set_js(opt.optarg);
            break;
            //C  
        case 'f':
            //C  			// select the first record
            //C  			firstrec = atol(optarg);
            firstrec = String_to_Int(opt.optarg);
            if (Number.isNaN(firstrec)) {
                usage_error += "\nOption '-" + opt.option +
                    "' requires an integer argument, not '" +
                    opt.optarg + "'. ";
                show_usage = true;
            }
            break;
            //C  
        case 'G':
            //C  			// set the port for connecting to the GDP daemon
            gdpd_addr = opt.optarg;
            break;
            //C  
        case 'm':
            //C  			// turn on multi-read (see also -s)
            multiread = true;
            break;
            //C  
        case 'n':
            //C  			// select the number of records to be returned
            //C  			numrecs = atol(optarg);
            numrecs = String_to_Int(opt.optarg);
            if (Number.isNaN(numrecs)) {
                usage_error += "\nOption '-" + opt.option +
                    "' requires an integer argument, not '" +
                    opt.optarg + "'. ";
                show_usage = true;
            }
            break;
            //C  
        case 's':
            //C  			// subscribe to this GCL (see also -m)
            subscribe = true;
            break;
            //C  
        default:
            show_usage = true;
            break;
        } /* end switch */
    } /* end while */
    //C  	}
    //C  	argc -= optind;
    argc -= opt_parser.optind();
    //C  	argv += optind;
    var argv_index = opt_parser.optind();
    //C  
    //C  	// we require a GCL name
    if (show_usage || argc <= 0) {
        //C  		fprintf(stderr,
        //C  				"Usage: %s [-D dbgspec] [-f firstrec] [-G gdpd_addr] [-m]\n"
        //C  				"  [-n nrecs] [-s] <gcl_name>\n",
        //C  				ep_app_getprogname());
        if (argc <= 0)
            usage_error += "\nMissing <gcl_name>.";
        console.error(
            "Usage: %s [-D dbgspec] [-f firstrec] [-G gdpd_addr] [-m]\n" +
            "  [-n nrecs] [-s] <gcl_name>%s",
            path.basename(process.argv[1]), usage_error + "\n");
        //C  		exit(EX_USAGE);
        fflush_all_js(); // sometimes Node.js may not empty buffers
        process.exit(EX_USAGE);
    }

    var gclname_arg = process.argv[opt_parser.optind()]

    // Set up call to read_gcl_records()
    gdpd_addr = gdpd_addr;
    gcl_name = gclname_arg;
    gcl_firstrec = firstrec;
    gcl_numrecs = numrecs;
    gcl_subscribe = subscribe;
    gcl_multiread = multiread;
    conout = true; // recdest = -1 so output to console.log()
    gdp_event_cbfunc = null; // no callback needed when just logging
    wait_for_events = true; // wait indefinitely in read_gcl_records() for
    // a next gdp event (i.e., record available)

    // Example set ups for calls to read_gcl_records()
    // TBD: Get comment up to date with read_gcl_records() definition
    //
    // A: read gcl and write to stdout
    // recdest = -1;  // writes the gcl records to stdout with
    //                // readable formatting
    //
    // B: read the gcl into a JS Array and return the Array
    // recdest =  0;  // read the gcl records into the Array {... records: }
    //               TBD: Not Yet Implemented
    //
    // Finally, call the function:
    // Returns:
    // { error_code: EP_STAT, records: Array of Strings }
    recdest = -1; // writes the gcl records to stdout...
    // DEBUG - TBD: LEFT OFF HERE making consistent with rw-supt.js
    //         Do the same for writer-test.js
    // console.log( 'In reader-test.js: main: calling read_gcl_records; firstrec = %d, numrecs = %d',
    //              firstrec, numrecs );
    gdpjs.read_gcl_records(gdpd_addr, gcl_name,
        gcl_firstrec, gcl_numrecs,
        gcl_subscribe, gcl_multiread,
        conout, gdp_event_cbfunc,
        wait_for_events
    );

    // TBD: return ( ! ep_stat_isok_js(estat) );

} /* end function main() */


// Use the familiar C/*NIX names
var argc = process.argv.length;
var argv = process.argv;

main(argc, argv);
