/* vim: set ai sw=4 sts=4 ts=4 : */

// Copyright (c) 2016 The Regents of the University of California.
// All rights reserved.

// Permission is hereby granted, without written agreement and without
// license or royalty fees, to use, copy, modify, and distribute this
// software and its documentation for any purpose, provided that the above
// copyright notice and the following two paragraphs appear in all copies
// of this software.

// IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY
// FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
// ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
// THE UNIVERSITY OF CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.

// THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE
// PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
// CALIFORNIA HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
// ENHANCEMENTS, OR MODIFICATIONS.

/**
 * Global Data Plane (GDP) JavaScript interface.
 *
 * See ../apps/writer-test.js and reader-test.js for versions
 * of the gdp-writer and gdp-reader executables.
 
 * @author Christopher Brooks, Alec Dara-Abrams
 * @version $$Id: gdp.js 838 2016-06-23 22:08:30Z cxh $$
 */

// Because of issues with require, it was necessary to combine
// libgdp_h.js gdpjs_supt.js and rw_supt.js into one file.  Those
// three files were created by Alec Dara-Abrams starting on
// 2014-11-02.

// This file supports both use as an installed module and use from
// with the gdp git repo.

// Path to the gdp/ directory.  Importantly, with a libs/ subdirectory
// holding the gdp dynamic libraries:
//    gdp/libs/libep,  gdp/libs/libgdp
// Be aware of particular major and minor version numbers in these names.
// See uses of ffi.Library( GDP_DIR +  ) below.
// Note: Here we are using a relative path for our default value:

// Set GDP_DIR so that we can find the GDP shared libraries.
try {
    if (GDP_DIR === undefined) {
        GDP_DIR = process.env.GDP_DIR;
    }
} catch (exception) {
        GDP_DIR = process.env.GDP_DIR;
}

try {
    if (GDP_DIR == undefined) {
        GDP_DIR = __dirname + "/..";
    }
} catch (exception) {
    GDP_DIR = "./";
}


// Path to the gdp/lang/js/gdpjs/ directory where this file usually resides.
// And which also has a libs/ subdirectory holding the gdpjs dynamic
// library:  gdpjs/../libs/libgdpjs.1.0.dylib
// See uses of ffi.Library( GDPJS_DIR +  ) below.

try {
    if (GDPJS_DIR == undefined) {
        GDPJS_DIR = __dirname;
    }
} catch (exception) {
        GDPJS_DIR = __dirname;
}

// Path to a directory containing the proper node_modules/ directory.
// See the require()'s below.
// Note: a null path prefix, evidently, means use the usual node module
// lookup mechanism (below); otherwise if the prefix is not null, ONLY
// look for the module in the directory:  prefix + <<module name>> .
// NODE_MODULES_DIR is set to force a particular directory to be
// searched, which is usefule for debugging.

try {
    if (NODE_MODULES_DIR == undefined) {
        NODE_MODULES_DIR = "";
    }
} catch (exception) {
    NODE_MODULES_DIR = "";
}

// To effectively "include" the text of this file in a JS program file
// that needs the definitions here, consider the device:
//
//   // Parameters needed for libgdp_h.js
//   // They MUST be adapted to the directory where this program will be run.
//   // See libgdp_h.js for details.
//   var GDP_DIR           = "../../../";
//   var GDPJS_DIR         = GDP_DIR + "./lang/js/gdpjs/";
//   var NODE_MODULES_DIR  = "";
//   //
//   var LIBGDP_H_DIR = GDPJS_DIR;
//   // Here we include and evaluate our shared GDP Javascript "header file",
//   // 'libgdp_h.js', within the global scope/environment.
//   var fs = require('fs');   // Node.js's built-in File System module
//   eval( fs.readFileSync( GDPJS_DIR + 'gdpjs.js').toString() );
//
// Note, we can't usefully wrap the include device within a JS function.  
// In such a case, new names would only be available within the scope of
// that function.
//
// We give due credit for this near-kludge, JS "include" device:
//    http://stackoverflow.com/questions/5797852/in-node-js-how-do-i-include-functions-from-my-other-files


// Provenance:
//
// This file is derived (by hand) from the gdp/gdp/ and gdp/ep/ include files,
// from writer-test.c and reader-test.c, as well as from some system include
// files.  See the comments below for details.
//
// Lines of C code (including all their comments & blank likes) from
// writer-test.c or reader-test.c are prefixed with "//C  ".
//
// Lines of C code imported from other GDP .[ch] files and system .h files
// are included here as JS comments and are prefixed with "//CJS".
//
// Regular (local) JS comments are prefixed with "//".
//
// If you find the included C code comments annoying, consider removing these
// lines with something like:
//    sed  -e '/^\/\/C/d' liggdp_h.js

var debug = false;

////////////////////////////////////
// Load Node.js modules for calling foreign functions; C functions here.
// The node modules below are installed locally in this file's directory
// via npm.
// It's not clear yet what path(s) node (the Node.js executable) uses to
// locate modules.  By default, it seems that node only looks for node_modules
// directories up the path from the current directory; it does not by default
// look in the "global" node_modules directory (which defaults to
// /usr/local/lib/node_modules/ ).
//
var ffi = require(NODE_MODULES_DIR + 'ffi');
var ref = require(NODE_MODULES_DIR + 'ref');
var ref_array = require(NODE_MODULES_DIR + 'ref-array');
var ref_struct = require(NODE_MODULES_DIR + 'ref-struct');
// var ref_union  = require( 'ref-union'  );   // currently, not used

// Some additional supporting Node.js modules:

// Node.js's built-in utilities for file paths
var path = require('path');

// POSIX style getopt()
var mod_getopt = require(NODE_MODULES_DIR + 'posix-getopt');

// escape non-printing characters
var jsesc = require(NODE_MODULES_DIR + 'jsesc');

// for a *NIX-like sleep(3), (synchronous - so not be very Node.js-ish)
var sleep = require(NODE_MODULES_DIR + 'sleep');

// The npm module 'printf', useful for debugging, seems to have problems.
// A use like: printf( "estat = %16X\n", estat ); may cause a core dump??
// var printf = require( 'printf' );


////////////////////////////////////
// Define types and constants
var gdp_gcl_open_info_tPtr = ref.refType(ref.types.void);

// { GRRRRRR... trying to get stdout up to the JS level using libc.fcntl()
// here JUST DOESN'T WORK.  Get a Segmentation fault: 11 on call to
// libgdp.gdp_datum_print().
// We bail out to adding libgdpjs.gdp_datum_print_stdout() below.
// This cost three hours+.

// From <unistd.h>
const STDIN_FILENO = 0; /* standard input file descriptor */
const STDOUT_FILENO = 1; /* standard output file descriptor */
const STDERR_FILENO = 2; /* standard error file descriptor */

// From <fcntl.h> ==> <sys/fcntl.h>
//CJS /* open-only flags */
const O_RDONLY = 0x0000; /* open for reading only */
const O_WRONLY = 0x0001; /* open for writing only */
const O_RDWR = 0x0002; /* open for reading and writing */
const O_ACCMODE = 0x0003; /* mask for above modes */

//CJS /* fcntl(2) command values */
const F_DUPFD = 0; /* duplicate file descriptor */
    // GRRRRRR }

// From libc - for local use here
var libc = ffi.Library('libc', {

    // From <sys/fcntl.h> and see man pages open(2) and fcntl(2)
    // But see note on fflush() below.  Makes these node-ffi functions suspect.

    //CJS int     open(const char *, int, ...) __DARWIN_ALIAS_C(open);
    // Use: fp_stdout_open  = libc.open( "/dev/stdout", /* mode = */ O_WRONLY );
    'open': ['pointer', ['string', 'int']], //?? correct?

    //CJS int     creat(const char *, mode_t) __DARWIN_ALIAS_C(creat);
    'creat': ['pointer', ['string', 'int']], //?? correct?

    //CJS int     fcntl(int, int, ...) __DARWIN_ALIAS_C(fcntl); 
    // Use: fp_stdout_fcntl = libc.fcntl( STDOUT_FILENO, F_DUPFD, 0 );
    'fcntl': ['pointer', ['int', 'int', 'int']], //?? correct?

    // From <stdio.h>  -- see man page fgets/gets(3), of course
    //CJS char    *gets(char *);
    'gets': ['string', ['string']], //?? seems to work

    // <stdio.h>  -- see man page fflush(3)
    // Though, an actual call to libc.fflush() seems to crash Node.js
    // See the note at the first print of " >>> " below.
    // But, libgdpjs.fflush_all() seems to work (no Node.js crash).
    'fflush': ['int', []]

});


// First, some definitions culled from gdp/gdp.h and system includes
// that are referenced in ep/ep_xxxx.h and gdp/gdp_xxxx.h .

// From ?? ==> <sysexits.h>
//CJS #define EX_USAGE        64      /* command line usage error */
var EX_USAGE = 64;

// From gdp/gdp.h => <inttypes.h> ==> <stdint.h> ==> <sys/_types/_int32_t.h>
var int32_t = ref.types.int32;

// From gdp/gdp.h => <stdbool.h> 
var bool_t = ref.types.int;


// Now we provide some definitions and functions from libep.

// First, some C types to define for Node.js ffi/ref modules.
// From ep/ep_stat.h
//CJS typedef struct _ep_stat { uint32_t code; } EP_STAT;
var EP_STAT = 'uint32';

// From ep/ep_time.h
//CJS 
//CJS typedef struct
//CJS {
//CJS 	int64_t		tv_sec;		// seconds since Jan 1, 1970
//CJS 	uint32_t	tv_nsec;	// nanoseconds
//CJS 	float		tv_accuracy;	// clock accuracy in seconds
//CJS } EP_TIME_SPEC;
//CJS 
// Field types - and, hence, sizes
var tv_sec_t = 'int64'; //?? check this size
var tv_nsec_t = 'uint32';
var tv_accuracy_t = 'float';
//
// Define the EP_TIME_SPEC_struct JS "struct" type
// Note, since ep/ep_tine.h doesn't define a typedef here, just struct
// EP_TIME_SPEC, we use the JS naming convention "<C struct name>_struct"
// rather that the "<C typedef name>_t" JS convention we use for 
// typedef struct gdp+gcl gcp_gcl_t below.  Sorry if this is pedantic, but
// we just don't have any compile-time type checking here on the JS side of
// our FFI interface - so we use naming convention as a weak crutch.
// Also note, unlike gdp_gcl_t, EP_TIME_SPEC_struct is NOT opaque for us
// up here in JS.
var EP_TIME_SPEC_struct = ref_struct({
    tv_sec: tv_sec_t,
    tv_nsec: tv_nsec_t,
    tv_accuracy: tv_accuracy_t
});
//
// A pointer to a C "struct EP_TIME_SPEC".
var EP_TIME_SPEC_struct_Ptr = ref.refType(EP_TIME_SPEC_struct);
//?? just below is not used yet
var EP_TIME_SPEC_struct_PtrPtr = ref.refType(EP_TIME_SPEC_struct_Ptr);


// libcrypto and libssl are different under RHEL vs. Ubuntu, so
// we have different shared libraries for RHEL.
var libgdpPath = '/libs/libgdp.0.8';
var libgdpjsPath = '/../libs/libgdpjs.1.0';
try {
    var fs = require('fs');
    fs.accessSync('/etc/redhat-release', fs.F_OK);
    libgdpPath = '/libs/libgdp.0.8-rhel';
    libgdpjsPath = '/../libs/libgdpjs.1.0-rhel';
} catch (exception) {
    // Not under RHEL
}

// The libgdp shared library is linked statically to libep.a, so
// instead of looking for the ep_* symbols in libep, we look in
// libgdp.  If we get the ep_* symbols from the libep shared library,
// then they will be different than the ep_* symbols that are used by
// calls in the libgdp shared library, which means that debugging will
// appear not to work.

//var libep = ffi.Library(GDP_DIR + '/libs/libep.3.0', {
var libep = ffi.Library(GDP_DIR + libgdpPath, {

    // From ep/ep_dbg.h
    //CJS // initialization
    //CJS extern void     ep_dbg_init(void);
    // May be called by gdp_init() .
    'ep_dbg_init': ['void', []],

    //CJS // setting debug flags
    //CJS extern void     ep_dbg_set(const char *s);
    'ep_dbg_set': ['void', ['string']],

    // From ep/ep_app.h
    //CJS extern const char       *ep_app_getprogname(void);
    'ep_app_getprogname': ['string', []],

    // From ep/ep_stat.h
    //CJS // return string representation of status
    //CJS char            *ep_stat_tostr(EP_STAT estat, char *buf, size_t bsize);
    // We only get a string back via the char* return value; buf seems unchanged
    'ep_stat_tostr': ['string', [EP_STAT, 'string', 'size_t']],

    // From ep/ep_time.h
    //CJS // return current time
    //CJS extern EP_STAT	ep_time_now(EP_TIME_SPEC *tv);
    // TBD add this calling information to other node-ffi functions
    // How to call from JS:
    //   var ts = new EP_TIME_SPEC_struct;
    //   estat  = libep.ep_time_now( ts.ref() );
    //   now_secs = ts.tv_sec; now_nsecs = ts.tv_nsec; now_acc = tspec.tv_accuracy;
    'ep_time_now': [EP_STAT, [EP_TIME_SPEC_struct_Ptr]],

    // From ep/ep_time.h
    //CJS // return putative clock accuracy
    //CJS extern float	ep_time_accuracy(void);
    'ep_time_accuracy': ['float', []],

    // From ep/ep_time.h
    //CJS // set the clock accuracy (may not be available)
    //CJS extern void	ep_time_setaccuracy(float acc);
    //  'ep_time_setaccuracy': [ 'void', [ 'float' ] ],

    // From ep/ep_time.h
    // There are problems accessing ep_time_format() using node-ffi with this sig.
    // Use ep_time_as_string_js() instead for access from JS -- via
    // gdpjs_supt.c/ep_time_as_string().  TBD
    //CJS // format a time string into a buffer
    //CJS extern void	ep_time_format(const EP_TIME_SPEC *tv,
    //CJS 				char *buf,
    //CJS 				size_t bz,
    //CJS 				bool human);
    // 'ep_time_format': [ 'void', 
    //                     [ EP_TIME_SPEC_struct_Ptr, 'string', 'size_t', bool_t ] ],

    // From ep/ep_time.h
    //CJS // parse a time string
    //CJS extern EP_STAT	ep_time_parse(const char *timestr,
    //CJS 				EP_TIME_SPEC *tv);
    'ep_time_parse': [EP_STAT, ['string', EP_TIME_SPEC_struct_Ptr]]

});


// From gdp/gdp.h
//CJS // the internal name of a GCL
//CJS typedef uint8_t gcl_name_t[32];
// YET TO DO: review all these node-ffi/ref "types" for portability
var uint8_t = ref.types.uint8;
var gdp_name_t = ref_array(uint8_t);
var gcl_name_t = ref_array(uint8_t);
// DEBUG
// console.log("gcl_name_t.size = ", gcl_name_t.size ); Outputs 8
// Below outputs: "function ArrayType(data, length) { .... many lines more }
// console.log("var gcl_name_t = ref_array(uint8_t);", gcl_name_t )

// From gdp/gdp.h
//CJS // the printable name of a GCL
//CJS #define GDP_GCL_PNAME_LEN 43   // length of an encoded pname
var GDP_GCL_PNAME_LEN = 43;
//CJS typedef char gcl_pname_t[GDP_GCL_PNAME_LEN + 1];
var char_t = ref.types.char;
var gcl_pname_t = ref_array(char_t);

// From gdp/gdp.h
//CJS // a GCL record number
//CJS typedef int64_t                         gdp_recno_t;
var gdp_recno_t = ref.types.int64; //?? check this size

// From gdp/gdp.h
//CJS typedef enum
//CJS {
//CJS         GDP_MODE_ANY = 0,       // no mode specified
//CJS         GDP_MODE_RO = 1,        // read only
//CJS         GDP_MODE_AO = 2,        // append only
//CJS } gdp_iomode_t;
var GDP_MODE_ANY = 0;
var GDP_MODE_RO = 1;
var GDP_MODE_AO = 2;


var GDP_GCLMD_XID =	0x00584944;	// XID (external id)
var GDP_GCLMD_PUBKEY =	0x00505542;	// PUB (public key)
var GDP_GCLMD_CTIME =	0x0043544D;	// CTM (creation time)
var GDP_GCLMD_CID = 0x00434944;	// CID (creator id)

var gdp_iomode_t = ref.types.int; //?? check this - enum === int ?

var char_t = ref.types.char;
var buf_t = ref_array(char_t);

// From gdp/gdp.h
//CJS  // an open handle on a GCL (opaque)
//CJS  typedef struct gdp_gcl gdp_gcl_t;
//
// So far, we don't need to look inside this struct or other GDP structs
// These comments are reminders of the internal structure and we hope can
// be removed in the future.
// From gdp/gdp_priv.h
// struct gdp_gcl
// {
//         EP_THR_MUTEX     mutex;
// //      EP_THR_COND      cond;       // Note, commented out in gdp_priv.h
//         struct req_head  reqs;
//         gcl_name_t       gcl_name;
//         gdp_iomode_t     iomode;
//         long             ver;
//         FILE             *fp;
//         void             *log_index;
//         off_t            data_offset;
// };
//
var gdp_gcl_t = ref.types.void; // opaque for us up here in JS
// a pointer to a C "typedef gdp_gcl_t"
var gdp_gcl_tPtr = ref.refType(gdp_gcl_t);
var gdp_gcl_tPtrPtr = ref.refType(gdp_gcl_tPtr); // pointer to a pointer

var gdp_gclmd_t = ref.types.void; // opaque for us up here in JS
var gdp_gclmd_tPtr = ref.refType(gdp_gclmd_t); // opaque for us up here in JS

// typedef uint32_t			gdp_gclmd_id_t;
var gdp_gclmd_id_t = ref.types.uint32;

//CJS typedef struct gdp_datum        gdp_datum_t;

var gdp_datum_t = ref.types.void; // opaque for us up here in JS
var gdp_datum_tPtr = ref.refType(gdp_datum_t);
var gdp_datum_tPtrPtr = ref.refType(gdp_datum_tPtr); //?? not used yet

// From gdp/gdp_buf.h
//CJS typedef struct evbuffer gdp_buf_t;

var gdp_buf_t = ref.types.void; // opaque for us up here in JS
var gdp_buf_tPtr = ref.refType(gdp_buf_t);
var gdp_buf_tPtrPtr = ref.refType(gdp_buf_tPtr); //?? not used yet

// From gdp/gdp.h
//CJS typedef struct gdp_event        gdp_event_t;

var gdp_event_t = ref.types.void; // opaque for us up here in JS
var gdp_event_tPtr = ref.refType(gdp_event_t);
var gdp_event_tPtrPtr = ref.refType(gdp_event_tPtr);


// Again, just a reminder of the internal structure. Remove in the future.
// From gdp/gdp_event.h
// struct gdp_event
// {       
//         TAILQ_ENTRY(gdp_event)  queue;          // free/active queue link
//         int                     type;           // event type
//         gdp_gcl_t               *gcl;           // GCL handle for event
//         gdp_datum_t             *datum;         // datum for event
// };   

// From gdp/gdp.h
//CJS // event types
//CJS #define GDP_EVENT_DATA          1       // returned data
//CJS #define GDP_EVENT_EOS           2       // end of subscription
var GDP_EVENT_DATA = 1 // returned data
var GDP_EVENT_EOS = 2 // end of subscription


var libgdp = ffi.Library(GDP_DIR + libgdpPath, {

    // From gdp/gdp.h
    //CJS // free an event (required after gdp_event_next)
    //CJS extern EP_STAT                  gdp_event_free(gdp_event_t *gev);
    'gdp_event_free': [EP_STAT, [gdp_event_tPtr]],

    //CJS // get next event (fills in gev structure)
    //CJS extern gdp_event_t              *gdp_event_next(gdp_gcl_t *gcl, EP_TIME_SPEC *timeout);
    'gdp_event_next': [gdp_event_tPtr, [gdp_gcl_tPtr, EP_TIME_SPEC_struct_Ptr]],

    //CJS // get next event (fills in gev structure)
    //CJS extern gdp_event_t              *gdp_event_next(bool wait);

    //CJS // get the type of an event
    //CJS extern int                       gdp_event_gettype(gdp_event_t *gev);
    'gdp_event_gettype': ['int', [gdp_event_tPtr]],

    //CJS // get the GCL handle
    //CJS extern gdp_gcl_t                *gdp_event_getgcl(gdp_event_t *gev);
    'gdp_event_getgcl': [gdp_gcl_tPtr, [gdp_event_tPtr]],

    //CJS // get the datum
    //CJS extern gdp_datum_t              *gdp_event_getdatum(gdp_event_t *gev);
    'gdp_event_getdatum': [gdp_datum_tPtr, [gdp_event_tPtr]],

    // From gdp/gdp_event.h
    //CJS // allocate an event
    //CJS extern EP_STAT                  _gdp_event_new(gdp_event_t **gevp);
    // _gdp_event_new is no longer present.
    //   '_gdp_event_new': [ EP_STAT, [ gdp_event_tPtrPtr ] ],

    //CJS // add an event to the active queue
    //CJS extern void                      _gdp_event_trigger(gdp_event_t *gev);
    // _gdp_event_trigger is no longer present.
    //   '_gdp_event_trigger': [ 'void', [ gdp_event_tPtr ] ],

    // From gdp/gdp.h
    //CJS // initialize the library
    //CJS EP_STAT gdp_init( const char *gdpd_addr );          // address of gdpd
    'gdp_init': [EP_STAT, ['string']],

    // From gdp/gdp.h
    //CJS // create a new GCL
    //CJS EP_STAT gdp_gcl_create( gcl_name_t, gdp_name_t, gdp_gclmd_t*, gdp_gcl_t ** ); // pointer to result GCL handle
    'gdp_gcl_create': [EP_STAT, [gcl_name_t, gcl_name_t, gdp_gclmd_tPtr, gdp_gcl_tPtrPtr]],

    // From gdp/gdp.h
    //CJS // open an existing GCL
    //CJS extern EP_STAT  gdp_gcl_open( gcl_name_t name, gdp_iomode_t rw, gdp_gcl_t **gclh);
    'gdp_gcl_open': [EP_STAT, [gcl_name_t, gdp_iomode_t, gdp_gcl_open_info_tPtr, gdp_gcl_tPtrPtr]],

    // From gdp/gdp.h
    //CJS // close an open GCL
    //CJS EP_STAT  gdp_gcl_close( gdp_gcl_t *gclh);           // GCL handle to close
    'gdp_gcl_close': [EP_STAT, [gdp_gcl_tPtr]],

    'gdp_gcl_print': ['void', [gdp_gcl_tPtr, 'void *']],

    // From gdp/gdp.h
    //CJS // make a printable GCL name from a binary version
    //CJS char *gdp_printable_name( const gcl_name_t internal, gcl_pname_t external);
    'gdp_printable_name': ['string', [gcl_name_t, gcl_pname_t]],

    // From gdp/gdp.h
    //CJS // parse a (possibly human-friendly) GCL name
    //CJS EP_STAT gdp_parse_name( const char *ext, gcl_name_t internal );
    'gdp_parse_name': [EP_STAT, ['string', gcl_name_t]],

    // From gdp/gdp.h
    //CJS // create a new metadata set
    // gdp_gclmd_t		*gdp_gclmd_new(int entries); 
    'gdp_gclmd_new': [gdp_gclmd_tPtr, ['int']],

    // add an entry to a metadata set
    // EP_STAT			gdp_gclmd_add(gdp_gclmd_t *gmd, gdp_gclmd_id_t id, size_t len, const void *data);
    'gdp_gclmd_add': [EP_STAT, [gdp_gclmd_tPtr, gdp_gclmd_id_t, 'size_t', gdp_datum_tPtr]],
    // From gdp/gdp.h
    //CJS // allocate a new message
    //CJS gdp_datum_t             *gdp_datum_new(void);
    'gdp_datum_new': [gdp_datum_tPtr, []],

    // From gdp/gdp.h
    //CJS // free a message
    //CJS void                    gdp_datum_free(gdp_datum_t *);
    'gdp_datum_free': ['void', [gdp_datum_tPtr]],

    // From gdp/gdp.h
    // print a message (for debugging)
    //CJS extern void gdp_datum_print( const gdp_datum_t *datum, FILE *fp );
    'gdp_datum_print': ['void', [gdp_datum_tPtr, 'pointer']],

    // From gdp/gdp.h
    //CJS // get the data buffer from a datum
    //CJS extern gdp_buf_t *gdp_datum_getbuf( const gdp_datum_t *datum );
    'gdp_datum_getbuf': [gdp_buf_tPtr, [gdp_datum_tPtr]],

    // From gdp/gdp_buf.h
    //CJS extern int gdp_buf_write( gdp_buf_t *buf, void *in, size_t sz );
    'gdp_buf_write': ['int', [gdp_buf_tPtr, buf_t, 'size_t']],

    // From gdp/gdp_buf.h
    //CJS extern size_t gdp_buf_read( gdp_buf_t *buf, void *out, size_t sz);
    'gdp_buf_read': ['size_t', [gdp_buf_tPtr, buf_t, 'size_t']],


    // From gdp/gdp.h
    //CJS // append to a writable GCL
    //CJS extern EP_STAT  gdp_gcl_append( gdp_gcl_t *gclh, gdp_datum_t *);
    'gdp_gcl_append': [EP_STAT, [gdp_gcl_tPtr, gdp_datum_tPtr]],

    // From gdp/gdp.h
    // subscribe to a readable GCL
    //CJS extern EP_STAT  gdp_gcl_subscribe(
    //CJS                   gdp_gcl_t *gclh,        // readable GCL handle
    //CJS                   gdp_recno_t start,      // first record to retrieve
    //CJS                   int32_t numrecs,        // number of records to retrieve
    //CJS                   EP_TIME_SPEC *timeout,  // timeout
    //CJS                   // callback function for next datum
    //CJS                   gdp_gcl_sub_cbfunc_t cbfunc,
    //CJS                   // argument passed to callback
    //CJS                   void *cbarg);
    // Note, in our call to this function in do_multiread() below we do not
    //       use the last three (pointer) arguments.
    'gdp_gcl_subscribe': [EP_STAT, [gdp_gcl_tPtr, gdp_recno_t, int32_t,
        'pointer', 'pointer', 'pointer'
    ]],

    // From gdp/gdp.h
    //CJS // read multiple records (no subscriptions)
    //CJS extern EP_STAT  gdp_gcl_multiread(
    //CJS                   gdp_gcl_t *gclh,        // readable GCL handle
    //CJS                   gdp_recno_t start,      // first record to retrieve
    //CJS                   int32_t numrecs,        // number of records to retrieve
    //CJS                   // callback function for next datum
    //CJS                   gdp_gcl_sub_cbfunc_t cbfunc,
    //CJS                   // argument passed to callback
    //CJS                   void *cbarg);
    // Note, in our call to this function in do_multiread() below we do not
    //       use the last two (pointer) arguments.
    'gdp_gcl_multiread': [EP_STAT, [gdp_gcl_tPtr, gdp_recno_t, int32_t,
        'pointer', 'pointer'
    ]],

    // From gdp/gdp.h
    //CJS // read from a readable GCL
    //CJS extern EP_STAT  gdp_gcl_read( gdp_gcl_t *gclh, gdp_recno_t recno, gdp_datum_t *datum);
    'gdp_gcl_read': [EP_STAT, [gdp_gcl_tPtr, gdp_recno_t, gdp_datum_tPtr]],


    // From gdp/gdp.h
    //CJS // get the record number from a datum
    //CJS extern gdp_recno_t      gdp_datum_getrecno(
    //CJS                                         const gdp_datum_t *datum);
    'gdp_datum_getrecno': [gdp_recno_t, [gdp_datum_tPtr]],

    // From gdp/gdp.h
    //CJS // get the timestamp from a datum
    //CJS extern void             gdp_datum_getts(
    //CJS                                         const gdp_datum_t *datum,
    //CJS                                         EP_TIME_SPEC *ts);
    // TBD: awaits our setup and testing of Node.js FFI access to structs
    // For now use ep_time_as_string()    -- in gdpjs_supt.c
    //         or  ep_time_as_string_js() -- in gdpjs_supt.js

    // From gdp/gdp.h
    //CJS // get the data length from a datum
    //CJS extern size_t   gdp_datum_getdlen( const gdp_datum_t *datum);
    'gdp_datum_getdlen': ['size_t', [gdp_datum_tPtr]],

    //   Below was used to test calling a C function with a very simple signature
    //   'log_view_ls': [ 'int', [ ] ]

});

exports.gdp_gcl_open = libgdp.gdp_gcl_open;
exports.gdp_gcl_read = libgdp.gdp_gcl_read;
exports.gdp_init = libgdp.gdp_init;
exports.gdp_parse_name = libgdp.gdp_parse_name;

// JS-to-GDP onion skin layer on selected GDP functions and macros.
//
var libgdpjs = ffi.Library(GDPJS_DIR + libgdpjsPath, {

    // Some general libc-related functions

    // Forces a flush() on stdout, on stderr, and on all open file descriptors.
    // Node.js console.log(), console.error(), and process.stdxxx.write() are,
    // evidently, buffered when writing to a file.  Output to a terminal seems
    // to be unbuffered.
    'fflush_stdout': ['int', []],
    'fflush_stderr': ['int', []],
    'fflush_all': ['int', []],

    // Some general libgdp/libep-related functions

    // Returns the size in bytes of the libep error status code, EP_STAT .
    // Currently (2014-10-26) defined in ep/ep-stat.h, 
    //CJS	typedef struct _ep_stat
    //CJS	{   
    //CJS		uint32_t        code;
    //CJS	} EP_STAT;
    // This is a very specialized libgdp function.  I hope to use it to construct
    // node-ffi/ref types at JS execution time for some of the ep functions here.
    'sizeof_EP_STAT_in_bytes': ['size_t', []],

    // From gdp/gdp.h
    // print a GCL (for debugging)
    // Forwards to gcp_gcl_print( const gdp_gcl_t *gclh, stdout, , , );
    'gdp_gcl_print_stdout': ['void', [gdp_gcl_tPtr]],

    // From gdp/gdp.h
    // print a message (for debugging)
    // Forwards to gdp_datum_print( const gdp_datum_t *datum, stdout );
    'gdp_datum_print_stdout': ['void', [gdp_datum_tPtr]],

    // From gdp/gdp.h
    // Get a printable (base64-encoded) GCL name from an open GCL handle
    // Accesses gchlh->pname field directly
    'gdp_get_pname_from_gclh': ['string', [gdp_gcl_tPtr]],

    // From gdp/gdp.h
    // Get a printable (base64-encoded) GCL name from an open GCL handle
    // Uses gdp_gcl_getname() and gdp_printable_name()
    'gdp_get_printable_name_from_gclh': ['string', [gdp_gcl_tPtr]],

    // Get a timestamp as a string from an EP_TIME_SPEC
    // Note, we are returning a char* to a static variable; copy out its contents
    // quickly :-).  See gdpjs_supt.c for details.
    // Wraps ep/ep_time.h ep_time_format()
    'ep_time_as_string': ['string', [EP_TIME_SPEC_struct_Ptr, bool_t]],

    // Get a timestamp as a string from a datum
    // Note, we are returning a char* to a static variable; copy out its contents
    // quickly :-).  See gdpjs_supt.c for details.
    // Wraps gdp/gdp.h gdp_datum_getts() & ep/ep_time.h ep_time_format()
    'gdp_datum_getts_as_string': ['string', [gdp_datum_tPtr, bool_t]],

    // From gdp/gdp_stat.h
    //CJS #define GDP_STAT_NAK_NOTFOUND   GDP_STAT_NEW(ERROR, GDP_COAP_NOTFOUND)
    // 'gdp_stat_nak_notfound': [ 'int' /* (int) EP_STAT */, [ ] ],
    'gdp_stat_nak_notfound': [EP_STAT, []],

    // From ep/ep_statcodes.h
    //CJS // generic status codes
    //CJS #define EP_STAT_OK         EP_STAT_NEW(OK, 0, 0, 0)
    'ep_stat_ok': [EP_STAT, []],

    // common shared errors
    // #define EP_STAT_END_OF_FILE   _EP_STAT_INTERNAL(WARN, EP_STAT_MOD_GENERIC, 3)
    'ep_stat_end_of_file': [EP_STAT, []],

    // From ep/ep_stat.h
    //CJS // predicates to query the status severity
    //CJS #define EP_STAT_ISOK(c)    (EP_STAT_SEVERITY(c) < EP_STAT_SEV_WARN)
    'ep_stat_isok': ['int' /* Boolean */ , [EP_STAT]],

    // From ep/ep_stat.h
    //CJS // compare two status codes for equality
    //CJS // #define EP_STAT_IS_SAME(a, b)   ((a).code == (b).code)
    'ep_stat_is_same': ['int' /* Boolean */ , [EP_STAT, EP_STAT]]
})

exports.ep_dbg_init = libep.ep_dbg_init;
exports.ep_dbg_set = libep.ep_dbg_set;
exports.ep_stat_isok = libgdpjs.ep_stat_isok;
exports.ep_stat_tostr = libep.ep_stat_tostr;



////////////////////////////////////
// JavaScript support routines for reading and writing gcl's
//
// TBD: lean up code; bring internal doc up to date;
//      regularize indenting with JS-aware tool
//      Check for possible error returns from libgdp calls - see TBD1 .
//

// ========================================================================
// Example set ups for calls to read_gcl_records()
// TBD1 Bring this doc up to date
// TBD update these examples and those in reader-test.js
// TBD recdest has been replaced by conout ??
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
// recdest = -1;  // writes the gcl records to stdout...
// read_gcl_records( gdpd_addr, gcl_name,
//                   gcl_firstrec, gcl_numrecs,
//                   gcl_subscribe, gcl_multiread, recdest
//                 );
//
// Note, read_gcl_records() calls:
//   do_multiread(gcl_Ptr, gcl_firstrec, gcl_numrecs, gcl_subscribe, ... );
//   do_simpleread(gcl_Ptr, gcl_firstrec, gcl_numrecs, ... );

/**
 * Read from a log.
 *
 * @param gdpdAddress  gdp daemon's <host:port>; if null, use default "127.0.0.1:2468"
 * @param gclName      name of existing GCL 
 * @param firstRecord  The first record number to be read
 * @param numberOfRecords The number of records to read.
 * @param consoleOut   Iff recdest == 0 and consoleOut == true; the Array entries written to the gcl will also be echoed to console.log().  The other recdest destinations will
 * ALL result in console.log() output; conout is ignored. Note, there still may be undesired output via console.log() and console.error().
 * @return  error_isok: false|true, error_code: EP_STAT, error_msg: String,  records: Array of records, each element with record data  where an element of the Array is: recno:     <integer record number>, timestamp: <String timestamp of record>, value:     <String contents of record>}
 */
function read_gcl_records(gdpd_addr, gcl_name,
    gcl_firstrec, gcl_numrecs,
    gcl_subscribe, gcl_multiread,
    conout, gdp_event_cbfunc,
    /* Boolean */
			  wait_for_events,
			  /* Boolean */ gcl_get_next_event
) {
    if (debug) {
	console.log("gdpjs.js: read_gcl_records(..., wait_for_events: " + wait_for_events + ", gcl_get_next_event: " + gcl_get_next_event + ")");
    }
    // Local working variables
    var gcl_Ptr; // gclh
    var estat; // EP_STAT
    var gclname = new gcl_name_t(32);
    var gclpname = new gcl_pname_t(GDP_GCL_PNAME_LEN);
    var recarray_out = []; // will hold contents of records read

    var ebuf = new Array(200 + 1).join(" "); // long enough??

    if (debug) {
        console.log("gdpjs.js: read_gcl_records");
    }

    ep_dbg_init_js();
    //ep_dbg_set_js('*=40');
    estat = gdp_init_js( /* String */ gdpd_addr);
    if ( ! ep_stat_isok_js(estat) ) {
        var emsg = "gdpjs.js: read_gcl_records(): gdp_init_js() is not ok";
        console.log(emsg);
        console.log(ep_stat_tostr_js(estat, ebuf, ebuf.length));
        rv = {
            err: {
                error_isok: ((ep_stat_isok_js(estat) == 0) ? false : true),
                error_code: ("0x" + estat.toString(16)),
                error_msg: emsg,
            },
            records: recarray_out
        };
        return rv;
    }

    // Allow thread to settle to avoid interspersed debug output.
    if (conout == true) { 
        // TBD check this
        sleep.sleep(1); // needed only for stdout and stderr
    }

    estat = gdp_parse_name_js(gcl_name, gclname);
    if (debug) {
        console.log("gdpjs.js: read_gcl_records: gclname: " + gclname.length);
        for (i=0; i<gclname.length; i++) {
            process.stdout.write(gclname[i].toString(16));
        }
        console.log("\n");
    }

    if ( ! ep_stat_isok_js(estat) ) {
        var emsg = "gdpjs.js: read_gcl_records(): gdp_parse_name_js() is not ok";
        console.log(emsg);
        console.log(ep_stat_tostr_js(estat, ebuf, ebuf.length));
        rv = {
            err: {
                error_isok: ((ep_stat_isok_js(estat) == 0) ? false : true),
                error_code: ("0x" + estat.toString(16)),
                error_msg: emsg,
            },
            records: recarray_out
        };
        return rv;
    }

    var rv_str = gdp_printable_name_js(gclname, gclpname);
    if (conout == true) {
        console.log("Reading GCL %s", array_to_String(gclpname));
        //console.log("Reading GCL %s", gcl_name);
    }

    // TBD: is this ref.alloc() necessary?
    var gclPtrPtr = ref.alloc(gdp_gcl_tPtrPtr);
    // TBD: check signature
    var gclopenrv = gdp_gcl_open_js(gclname, GDP_MODE_RO, gclPtrPtr);
    estat = gclopenrv.error_code;
    gcl_Ptr = gclopenrv.gclH;
    if ( ! ep_stat_isok_js(estat) ) {
        var emsg = "gdpjs.js: read_gcl_records(): gdp_gcl_open_js() is not ok.  Check to be sure the log \"" + gcl_name + "\" exists.";
        console.log(emsg);
        console.log(ep_stat_tostr_js(estat, ebuf, ebuf.length));
        rv = {
            err: {
                error_isok: ((ep_stat_isok_js(estat) == 0) ? false : true),
                error_code: ("0x" + estat.toString(16)),
                error_msg: emsg,
            },
            records: recarray_out
        };
        return rv;
    }

    var called = "";
    if (gcl_subscribe || gcl_multiread) {
	if (gdp_event_cbfunc !== null) {
            if (debug) {
	        console.log('gdpjs.js: read_gcl_records(): before do_multiread_subscribe()' );
            }
	    estat = libgdp.gdp_gcl_subscribe(gcl_Ptr, gcl_firstrec, gcl_numrecs,
					     null, gdp_event_cbfunc, null);
	    called = "gdp_gcl_subscribe()";
	} else {
	    // true for reader-test.js; false for gdpREST_server.js
	    if (!gcl_get_next_event) {
		// gcl_get_next_event is false, so we have not yet subscribed to the log.
		console.log('gdpjs.js: read_gcl_records(): before do_multiread()' );
		estat = do_multiread(gcl_Ptr, gcl_firstrec, gcl_numrecs, gcl_subscribe,
				     wait_for_events,
				     recarray_out, conout, gdp_event_cbfunc
				     );
		called = "do_multiread()";
	    } else {
		// gcl_get_next_event is true, so we have already subscribed to the log.
		console.log('gdpjs.js: read_gcl_records(): before do_multiread_inner()' );
		estat = do_multiread_inner(gcl_Ptr, gcl_firstrec, gcl_numrecs, gcl_subscribe,
					   wait_for_events,
					   recarray_out, conout, gdp_event_cbfunc,
					   /* Timeout in seconds */ 1
					   );
		called = "do_multiread_inner()";
	    }
	}
    } else {
        estat = do_simpleread(gcl_Ptr, gcl_firstrec, gcl_numrecs,
            recarray_out, conout
        );
	called = "do_simplread()";
    }

    if ( ! ep_stat_isok_js(estat) ) {
        var emsg = "gdpjs.js: read_gcl_records(): " + called + " is not ok";
        console.log(emsg);
        console.log(ep_stat_tostr_js(estat, ebuf, ebuf.length));
        rv = {
            err: {
                error_isok: ((ep_stat_isok_js(estat) == 0) ? false : true),
                error_code: ("0x" + estat.toString(16)),
                error_msg: emsg,
            },
            records: recarray_out
        };
        return rv;
    }

    gdp_gcl_close_js(gcl_Ptr);

    // TBD: fix this error return - see corresponding location in reader-test.js
    // string.repeat not available for us here in ECMASscript<6
    var str = new Array(200 + 1).join(" "); // long enough??
    var emsg = ("gdpjs.js: read_gcl_records(): exiting with status " +
        ep_stat_tostr_js(estat, str, str.length));
    if (conout == true) {
        fflush_all_js(); // sometimes Node.js may not empty buffers
        console.error(emsg);
    }
    // console.error( "exiting with status %s",
    // 				ep_stat_tostr_js(estat, str, str.length) );
    // OLD return ( ! ep_stat_isok_js(estat) );
    rv = {
        err: {
            error_isok: ((ep_stat_isok_js(estat) == 0) ? false : true),
            error_code: ("0x" + estat.toString(16)),
            error_msg: emsg,
        },
        records: recarray_out
    };
    if (debug) {
	console.log("gdpjs.js: read_gcl_records(): done");
    }
    return rv;
} /* end read_gcl_records( ) */


exports.read_gcl_records = read_gcl_records;

// ========================================================================

function setDebug(debugValue) {
    // We have a separate function to set debug so that Coverity Scan does not warn about unreachable code.
    debug = debugValue;
}

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
function do_simpleread(gclh, firstrec, numrecs,
    recarray_out, conout
) {
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
    // TBD where is datum freed?
    //C  
    //C  	// change the "infinity" sentinel to make the loop easier
    if (numrecs == 0) numrecs = -1;
    //C  
    //C  	// can't start reading before first record (but negative makes sense)
    if (firstrec == 0) firstrec = 1;
    //C  
    //C  	// start reading data, one record at a time
    recno = firstrec;
    var crec = 0; // counts records read - an index into recarray_out[]
    while (numrecs < 0 || --numrecs >= 0) {
        //C  		// ask the GDP to give us a record
        estat = gdp_gcl_read_js(gclh, recno, datum);
        //C  
        //C  		// make sure it did; if not, break out of the loop
        if (!ep_stat_isok_js(estat)) {
            break;
        }
        //C  
        if (conout == true) {
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
        }
        //C  
        // TBD1 - error checks
        // grab record contents for this newly read record
        var val = gdp_datum_buf_as_string(datum);
        // grab record number and timestamp for this newly read record
        var ts = gdp_datum_getts_as_string_js(datum, true /* format */ );
        // TBD: below check for 64-bit integer return type, gdp_recno_t
        var rn = gdp_datum_getrecno_js(datum);
        // TBD: check that recno and rn agree - which to use here?
        recarray_out[crec] = {
            recno: rn, // for now we use gdp's record number
            timestamp: ts,
            value: val
        };
        crec++;
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
// ========================================================================
//C  
//C  /*
//C  **  DO_MULTIREAD --- subscribe or multiread
//C  **
//C  **		This routine handles calls that return multiple values via the
//C  **		event interface.  They might include subscriptions.
//C  */
//   TBD: document the additional functionality we have added to the original
//   C version of this function:
//	      wait_for_events, recarray_out, conout, gdp_event_cbfunc
//   Note: gdp_event_cbfunc is, currently, not all that useful since it's
//   not really Node.js asynchrouous -- there is no handling of C-level
//   callbacks (which, BTW, aren't yet implemented).
//   TBD ensure wait_for_events == true/false works for gdp_gcl_multiread..()
//   TBD to better handle errors, put error info in a structured return value
//C  
//C  EP_STAT
//C  do_multiread(gdp_gcl_t *gclh, gdp_recno_t firstrec, int32_t numrecs, bool subscribe)
//C  {
// Note, firstrec is a JS Number not a ref gdp_recno_t and, similarly,
//      true numrecs is a JS Number not a ref int32_t
/* EP_STAT */
function do_multiread(gclh, firstrec, numrecs, subscribe,
    /* Boolean */  wait_for_events,
    recarray_out, conout, gdp_event_cbfunc
) {
    if (debug) {
	console.log("gdpjs.js: do_multiread(" + gclh + ", " + firstrec + ", " + numrecs + ", subscribe: " + subscribe + ", wait_for_events: " + wait_for_events + ", recarray_out: " + recarray_out + ", conout: " + conout + ", " + gdp_event_cbfunc + ")");
    }
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
        // TBD1 - does this case still work for testing and for server use?
        estat = gdp_gcl_multiread_no_callback_js(gclh, firstrec, numrecs);
    }
    //C  
    //C  	// check to make sure the subscribe/multiread succeeded; if not, bail
    // DEBUG
    if (debug) {
	console.log('gdpjs.js: do_multiread() after call to gdp_gcl_multiread_no_callback_js()');
    }
    if (!ep_stat_isok_js(estat)) {
        //C  		char ebuf[200];
        //C  
        //C  		ep_app_abort("Cannot %s:\n\t%s",
        //C  				subscribe ? "subscribe" : "multiread",
        //C  				ep_stat_tostr(estat, ebuf, sizeof ebuf));
        // No need for ep_app_abort's PRINTFLIKE behavior here - just
        // give an error msg and exit.
        // ??consider var ebuf = new buf_t(100);
        // TBD place this error info in a structured return value
        var ebuf = new Array(200 + 1).join(" "); // long enough??
        console.error("Cannot %s:\n\t%s",
            subscribe ? "subscribe" : "multiread",
            ep_stat_tostr_js(estat, ebuf, ebuf.length)
        );
        fflush_all_js(); // sometimes Node.js may not empty buffers
        // Not acceptable error behavior for a server!
        //process.exit(1); // could have better error code; e.g., EX_USAGE
    }
    if (debug) {
	console.log("gdpjs.js: do_multiread(): about to loop");
    }

    // Skip trying to read, we do that with a second call to read_gcl_records(), which will call do_multiread_inner()

//     for (;;) {
// 	estat = do_multiread_inner(gclh, firstrec, numrecs, subscribe,
// 			   /* Boolean */  wait_for_events,
// 				   recarray_out, conout, gdp_event_cbfunc, /* timeout in seconds */ 1);
	    
//     }
    if (debug) {        
	console.log("gdpjs.js: do_multiread() done.");
    }
    return estat;

}

/** Subscribe.
 *  Based on do_multiread() as defined in gdp-reader.c.  Modified to only handle subscription
 *  @param gclh The handle to the gcl (gdp_gcl_t)
 *  @param firstrec The first record to be returned  (gdp_recno_t)
 *  @param numrecs The number of records to be read (int_32t)
 *  @param cbfunc The callback function that takes a gdp_event_t * as an argument and returns void
 */
function do_multiread_subscribe(gclh, firstrec, numrecs, cbfunc) {

    return estat;
}
/** Get the next event and update recarray_out. 
 *  @param recarray_out Updated with the output
 *  @return estat
 */
function do_multiread_inner(gclh, firstrec, numrecs, subscribe,
    /* Boolean */  wait_for_events,
			    recarray_out, conout, gdp_event_cbfunc, timeoutSeconds
) {
    // FIXME: This code is based on an old version of do_multiread() from
    // gdp-reader.c.  The version of do_multiread() in gdp-reader.c has changed
    // significantly since this version was created.  See do_multiread_subscribe()
    // for a more up to date version.

    // This function used to be inside the for (;;) loop in do_multiread(),
    // but instead we want getNextEvent(timeout) to call it.
    if (debug) {
	console.log("gdpjs.js: do_multiread_inner(" + gclh + ", " + firstrec + ", " + numrecs + ", subscribe: " + subscribe + ", wait_for_events: " + wait_for_events + ", recarray_out: " + recarray_out + ", conout: " + conout + ", " + gdp_event_cbfunc + ", timeoutSeconds: " + timeoutSeconds + ")");
    }

	
    //C  
    var crec = 0; // counts records read - an index into recarray_out[]
    //C  	// now start reading the events that will be generated


        //C  		// get the next incoming event
        //C         gdp_event_t *gev = gdp_event_next(true);
        var gev_Ptr; // gev
        var evtype_int;
        // OLD gev_Ptr = gdp_event_next_js(true);
        // if wait_for_events == false, do a single
        // synchronous poll here for a currently waiting gdp event; don't 
        // wait for any future events; if there is no currently waiting
        // event, return immediately with a null
        // if wait_for_events == true, wait (indefinitely)
        // for a next gdp event

	// FIXME: passing null as a timeout for now.

	var eptspec = new EP_TIME_SPEC_struct;
	console.log("gdpjs.js: do_multiread_inner(): setting timeout for " + timeoutSeconds + " seconds.");
	eptspec.tv_sec = timeoutSeconds;
        gev_Ptr = gdp_event_next_js(gclh, eptspec.ref());
        // TBD: what if gev_Ptr is null; happens on gdp_event_next_js(false)
        //      with no event available.
	if (debug) {
	    console.log( 'gdpjs.js: do_multiread_inner() after call to gdp_event_next_js(). gev_Ptr = ', gev_Ptr );
	}

        // if ( gev_Ptr == null )
        if (gev_Ptr.isNull()) { // no next event found - just return with no side-effects
            // on recarray_out, or call of gdp_event_cbfunc.
	    if (debug) {        
		console.log( 'gdpjs.js: do_multiread_inner() no event found, return with no side-effects on recarray_out or a call of gdp_event_cbfunc.' );
	    }
            return estat;
        } else { // we have seen an event - process it based on its type
	    if (debug) {
		console.log( 'gdpjs.js: do_multiread_inner() we have seen an event - process it' );
	    }
            evtype_int = gdp_event_gettype_js(gev_Ptr);
            //C  
            //C  		// decode it
            switch (evtype_int) {
            case GDP_EVENT_DATA:
                //C  			// this event contains a data return
                var datum = gdp_event_getdatum_js(gev_Ptr);
                if (conout == true) {
                    //C  			fprintf(stdout, " >>> ");A
                    // See process.stdout.write comments above in do_simpleread() .
                    process.stdout.write(" >>> ");
                    fflush_all_js(); // sometimes Node.js may not empty buffers
                    gdp_datum_print_stdout_js(datum);
                }
		// TBD1 - cleanup here
		// do this stashing into recarray_out even if we
		// don't have a non-null gdp_event_cbfunc()
		// grab record contents for this newly read record
		var val = gdp_datum_buf_as_string(datum);
		// grab record number and timestamp for this newly read record
		var ts = gdp_datum_getts_as_string_js(datum, true /* format */ );
		// TBD: below check for 64-bit integer return type, gdp_recno_t
		var rn = gdp_datum_getrecno_js(datum);
		// TBD: check that recno and rn agree - which to use here?
		recarray_out[crec] = {
		    recno: rn, // for now we use gdp's record number
		    timestamp: ts,
		    value: val
		};
		crec++;
		if (debug) {        
		    console.log( 'gdpjs.js: do_multiread_inner(): GDP_EVENT_DATA: about to call gdp_event_cbfunc()' );
		}
                if (gdp_event_cbfunc) {
                    // TBD1 - cleanup here
		    // FIXME: This gdp_event_cbfunc() has a different call signature
		    // that what is currently in gdp.h
                    gdp_event_cbfunc(evtype_int, datum, recarray_out);
		}

                break;
                //C  
            case GDP_EVENT_EOS:
                //C  			// "end of subscription": no more data will be returned
                //C  			fprintf(stdout, "End of %s\n",
                //C  					subscribe ? "Subscription" : "Multiread");
                // TBD should we also return this as error info?
                if (conout == true) {
                    console.log("End of %s", subscribe ? "Subscription" : "Multiread");
                }
                if (gdp_event_cbfunc) {
		    if (debug) {        
			console.log( 'gdpjs.js: do_multiread_inner() GDP_EVENT_EOS (end of subscription)' );
		    }
		    // FIXME: This gdp_event_cbfunc() has a different call signature
		    // that what is currently in gdp.h
                    gdp_event_cbfunc(evtype_int, null, estat);

                }
                return estat;
                //C  
            default:
                //C  			// should be ignored, but we print it since this is a test program
                //C  			fprintf(stderr, "Unknown event type %d\n", gdp_event_gettype(gev));
                // TBD handle this error situation more usefully
                // TBD should we also return this as error info?
                if (conout == true) {
                    console.error("Unknown event type %d\n",
                        gdp_event_gettype_js(gev_Ptr));
                    fflush_all_js(); // sometimes Node.js may not empty buffers
                }
                //C  
                //C  			// just in case we get into some crazy loop.....
                // TBD check the need for this in server code
                sleep.sleep(1);
                break;
            } /* switch */
            //C  
            //C  		// don't forget to free the event!
            // protect against freeing nulls
            if (gev_Ptr != null) {
                gdp_event_free_js(gev_Ptr);
            }

        } /* end if ( gev_Ptr == null ) else */
    return estat;
} /* end do_multiread_inner() */

//C  }
//C
/* vim: set ai sw=4 sts=4 ts=4 : */

// JavaScript support routines for GDP access from Node.js-based JavaScript
// 2014-11-02
// Alec Dara-Abrams

// TBD: put this code into a Node.js module; add Copyrights


// JS wrappers for calls to libep, libgdp, and libgdpjs FFI functions =======

// Currently, we are using an eval-based mechanism to include the above
// libraries as well as this code; we hope to construct a Node.js module
// to better encapsulate all these support functions and variables.


/* Note:
   EP_STAT      JS<==>C  uint32
   gclHandle_t  JS<==>C  gdp_gcl_t *
   String       JS<==>C  gcl_name_t *
   TBD -- get these JS<-->C type correspondences documented here
   and in each ...._js() function below.
*/

/* EP_STAT */
function ep_stat_ok_js() {
    return libgdpjs.ep_stat_ok();
}

/* C: size_t  <==> JS: Integer */
function sizeof_EP_STAT_in_bytes_js()
// Pull libgdp/libep information into JS.
// Really, a C compile-time constant we bring into JS
{
    return libgdpjs.sizeof_EP_STAT_in_bytes();
}

/* EP_STAT */
function gdp_init_js( /* String */ gdpd_addr) {
    // libgdp.gdp_init() seems to have trouble passing in an explicit
    // null string but an undefined argument works OK to start the
    // library on the default gdpd host:port .
    // So we protect this call from incoming JS null strings.
    if (debug) {
        console.log("gdpjs.js: gdp_init_js(" + gdpd_addr + ")");
    }
    if (gdpd_addr == "") { // DEBUG: console.log( 'then: gdpd_addr = \"' + gdpd_addr + '\"' );
        if (debug) {
            console.log("gdpjs.js: gdp_init_js(" + gdpd_addr + "): calling libgdp.gdp_init(undefined)");
        }
        return libgdp.gdp_init(undefined);
    } else { // DEBUG: console.log( 'else: gdpd_addr = \"' + gdpd_addr + '\"' );
        if (debug) {
            console.log("gdpjs.js: gdp_init_js(" + gdpd_addr + "): calling libgdp.gdp_init(" + gdpd_addr + ")");
        }
        return libgdp.gdp_init(gdpd_addr);
    }
}

exports.gdp_init_js = gdp_init_js;

/* Boolean */
function ep_stat_isok_js( /* EP_STAT */ estat) {
    return libgdpjs.ep_stat_isok(estat);
}

/* EP_STAT */
function gdp_stat_nak_notfound_js()
// return a constant "NAK NOTFOUND" EP_STAT value
{
    return libgdpjs.gdp_stat_nak_notfound();
}

/* EP_STAT */
function ep_stat_end_of_file_js()
// return a constant "END OF FILE" EP_STAT value
{
    return libgdpjs.ep_stat_end_of_file();
}

/* Boolean */
function ep_stat_is_same_js(estat_a, estat_b) {
    return libgdpjs.ep_stat_is_same(estat_a, estat_b);
}

/* { error_code: EP_STAT, gclH: gclHandle_t }; */
function gdp_gcl_create_js( /* gcl_name_t */ gclxname,
			    logdxname) {
    var gcl_Ptr; // gclHandle_t 
    var gclPtrPtr = ref.alloc(gdp_gcl_tPtrPtr); // gclHandle_t *

    var gcliname = new gcl_name_t(32);
    //var gcliname = ref.alloc(gcl_name_t);
    var logdiname = new gcl_name_t(32);
    //var logdiname = ref.alloc(gcl_name_t);

    if (debug) {
        console.log("gdpjs.js: gdp_gcl_create_js(): gclxname: " + gclxname + ", logdxname: " + logdxname);
    }

    gdp_parse_name_js(gclxname, gcliname);
    gdp_parse_name_js(logdxname, logdiname);

    var gmd = libgdp.gdp_gclmd_new(0);

    libgdp.gdp_gclmd_add(gmd, GDP_GCLMD_XID, gclxname.length, ref.allocCString(gclxname));


    var estat = libgdp.gdp_gcl_create(gcliname, logdiname, gmd, gclPtrPtr);

    if ( ! ep_stat_isok_js(estat) ) {
	var ebuf = new Array(200 + 1).join(" "); // long enough??
	var emsg = "gdpjs.js: gdp_gcl_create_js(): gdp_gcl_create() is not ok.";
	console.log(emsg);
	console.log(ep_stat_tostr_js(estat, ebuf, ebuf.length));
    }
    gcl_Ptr = gclPtrPtr.deref();
    return {
        error_code: estat,
        gclH: gcl_Ptr
    };
}

/* EP_STAT */
function gdp_parse_name_js( /* String */ xname, /* gcl_name_t */ gcliname) {
    if (debug) {
        console.log("gdpjs.js: gdp_parse_name_js(): " + xname + ", ...");
    }
    var estat = libgdp.gdp_parse_name(xname, gcliname);

    for (i=0; i<gcliname.length; i++) {
        process.stdout.write(gcliname[i].toString(16));
    }
    if (debug) {
        console.log("\n");
    }
    return estat
};


function gdpGclOpen(name, iomode, logdname) {
    if (debug) {
	console.log("gdpjs.js: 0 gdpGclOpen(" + name + ", " + iomode + ", " + logdname + "): setting debugging (dbg_set_js)"); 
	ep_dbg_set_js("*=8");

    }
    var gcliname = new gcl_name_t(32);
    //var gcliname = ref.alloc(gcl_name_t);

    estat = gdp_parse_name_js(name, gcliname);
    if (debug) {
	console.log("gdpjs.js: 10 gdpGclOpen gcliname: " + gcliname.length);
	for (i=0; i<gcliname.length; i++) {
	    process.stdout.write(gcliname[i].toString(16));
	}
	console.log("\n");
    }

    if ( ! ep_stat_isok_js(estat) ) {
	var ebuf = new Array(200 + 1).join(" "); // long enough??
	var emsg = "gdpjs.js: gdpGclOpen(): gdp_parse_name_js() is not ok";
	console.log(emsg);
	console.log(ep_stat_tostr_js(estat, ebuf, ebuf.length));
        var rv = {
            err: {
                error_isok: ((ep_stat_isok_js(estat) == 0) ? false : true),
                error_code: ("0x" + estat.toString(16)),
                error_msg: emsg,
            }
        };
        return rv;
    }
 
    if (debug) {
	console.log("gdpjs.js: gdpGclOpen(): calling gdp_gcl_open_js(), which may fail, but if it does, we will create the log.");
    }
    var rv = gdp_gcl_open_js(gcliname, iomode);
    if (debug) {
	console.log("gdpjs.js: gdpGclOpen(): gdp_gcl_open_js() returned.");
    }
    if ( ! ep_stat_isok_js(rv.error_code) ) {    
	if (debug) {
	    console.log("gdpjs.js: gdpGclOpen(): gdp_gcl_open() failed, trying to create the log and call gdp_gcl_open() again.");
	}
	rv = gdp_gcl_create_js(name, logdname);
	if ( ! ep_stat_isok_js(rv.error_code) ) {
	    var ebuf = new Array(200 + 1).join(" "); // long enough??
	    var emsg = "gdpjs.js: gdpGclOpen(): gdp_gcl_create_js() is not ok";
	    console.log(emsg);
	    console.log(ep_stat_tostr_js(estat, ebuf, ebuf.length));
	    rv = {
		err: {
		    error_isok: ((ep_stat_isok_js(estat) == 0) ? false : true),
		    error_code: ("0x" + estat.toString(16)),
		    error_msg: emsg,
		}
	    };
	    return rv;
	}
	if (debug) {
	    console.log("gdpjs.js: almost done, about to call gdp_gcl_open_js() again.");
	}
	rv = gdp_gcl_open_js(gcliname, iomode);
    }

    return rv;
}

exports.gdpGclOpen = gdpGclOpen;

/* { error_code: EP_STAT, gclH: gclHandle_t }; */
function gdp_gcl_open_js( /* gcl_name_t */ gcliname,
    /* gdp_iomode_t */
    mode,
    gclPtrPtr
) {
    var gcl_Ptr; // gclHandle_t 
    var gclPtrPtr = ref.alloc(gdp_gcl_tPtrPtr); // gclHandle_t *
    var estat = libgdp.gdp_gcl_open(gcliname, mode, ref.NULL, gclPtrPtr);
    gcl_Ptr = gclPtrPtr.deref();
    return {
        error_code: estat,
        gclH: gcl_Ptr
    };
}

/* void */
function ep_dbg_init_js() {
    libep.ep_dbg_init();
};

/* String */
function ep_stat_tostr_js( /* EP_STAT */ estat,
    /* String */
    str,
    /* Integer */
    str_len
) {
    return libep.ep_stat_tostr(estat, str, str.length);
};

/* EP_STAT */
function ep_time_now_js( /* EP_TIME_SPEC * */ EP_TIME_SPEC_struct_Ptr) {
    return libep.ep_time_now(EP_TIME_SPEC_struct_Ptr);
};

/* void */
function ep_dbg_set_js( /* String */ ep_dbg_pattern) {
    libep.ep_dbg_set(ep_dbg_pattern);
};

/* String */
function gdp_printable_name_js( /* gcl_name_t  */ gclname,
    /* gcl_pname_t */
    gclpname) {
    var rv_str;
    rv_str = libgdp.gdp_printable_name(gclname, gclpname);
    return rv_str;
}

/* void */
function gdp_gcl_print_stdout_js(gcl_Ptr) {
    /* void */
    libgdpjs.gdp_gcl_print_stdout(gcl_Ptr);
}

/* String */
function gdp_get_pname_from_gclh_js(gcl_Ptr) {
    var rv_str;
    rv_str = libgdpjs.gdp_get_pname_from_gclh(gcl_Ptr);
    return rv_str;
}

/* String */
function gdp_get_printable_name_from_gclh_js(gcl_Ptr) {
    var rv_str;
    // Note, the below returns a pointer to a statically allocated gcl_pname_t,
    // currently, typedef char gcl_pname_t[GDP_GCL_PNAME_LEN + 1] in gdp.h
    rv_str = libgdpjs.gdp_get_printable_name_from_gclh(gcl_Ptr);
    return rv_str;
}

/* String */
function ep_time_as_string_js(ep_time_spec, /* Boolean */ human_format) {
    var rv_str;
    // Note, the below returns a pointer to a statically allocated 
    // char tbuf[100] in gdpjs_supt.c/ep_time_as_string() .
    rv_str = libgdpjs.ep_time_as_string(ep_time_spec, human_format);
    // DEBUG console.log("ep_time_as_string_js: rv_str = '" + rv_str + "'" );
    return rv_str;
}

/* String */
function gdp_datum_getts_as_string_js(datum, /* Boolean */ human_format) {
    var rv_str;
    // Note, the below returns a pointer to a statically allocated 
    // char tbuf[100] in gdpjs_supt.c/ep_time_as_string() .
    rv_str = libgdpjs.gdp_datum_getts_as_string(datum, human_format);
    // DEBUG console.log("gdp_datum_getts_as_string_js: rv_str = '" + rv_str + "'" );
    return rv_str;
}

/* datum */
function gdp_datum_new_js() {
    return libgdp.gdp_datum_new();

};

exports.gdp_datum_new2 = function() {
    return libgdp.gdp_datum_new();
};


/* EP_STAT */
exports.gdpGclRead = function (gclh, recno, datum) {
    return gdp_gcl_read_js(gclh, recno, datum);
}
function gdp_gcl_read_js(gclh, recno, datum) {
    return libgdp.gdp_gcl_read(gclh, recno, datum);
}

/* gdp_recno_t */
function gdp_datum_getrecno_js(datum) {
    return libgdp.gdp_datum_getrecno(datum);
}

/* EP_STAT */
function gdp_gcl_publish_buf_js(gcl_Ptr, datum, buf) {
    var rv_estat;
    var temp_gdp_buf = libgdp.gdp_datum_getbuf(datum);
    libgdp.gdp_buf_write(temp_gdp_buf, buf, buf.length);
    rv_estat = libgdp.gdp_gcl_append(gcl_Ptr, datum);
    return rv_estat;
};

// TBD: WARNING - likely will JS blow up at FFI  LEFT OFF HERE
/* EP_STAT */
function gdp_gcl_subscribe_no_callback_js(gclh, firstrec, numrecs,
                                          timeout /* seconds */ )
// This is a limited-functionality version of
//   libgdp.gdp_gcl_subscribe(gclh, firstrec, numrecs, timeout, cbfunc, cbarg);
// See the comments on callbacks below for:
//    gdp_gcl_subscribe_no_timeout_no_callback_js() .
// A smaller issue is passing in the EP_TIME_SPEC timeout argument (a C struct).
// Node.js ref-struct should provide a straight-forward FFI implementation.
// This smaller issue is also TBD.
{
    var rv_estat;
    if (debug) {
	console.log("gdpjs.js: gdp_gcl_subscribe_no_callback_js(" + gclh + ", " + firstrec + ", " + numrecs + ", "+ _timeout + ")");
    }

    // EP_TIME_SPEC *timeout, gdp_gcl_sub_cbfunc_t cbfunc, void *cbarg 
    // are all null here for a simpler subscribe variant.  See gdp/gdp-api.[ch]
    // gdp_gcl_subscribe() for details.
    // TBD: this use of timeout should blow up at the FFI - time to bite the
    //      bullet and use ref-struct ?
    rv_estat = libgdp.gdp_gcl_subscribe(gclh, firstrec, numrecs,
                                        timeout, null, null);
    if (debug) {
	console.log("gdpjs.js: gdp_gcl_subscribe_no_callback_js() done: " + rv_estat);
    }
    return rv_estat;
};

/* EP_STAT */
function gdp_gcl_subscribe_no_timeout_no_callback_js(gclh, firstrec, numrecs)
// This is a limited-functionality version of
//   libgdp.gdp_gcl_subscribe(gclh, firstrec, numrecs, timeout, cbfunc, cbarg);
// We currently do not have a mechanism for invoking a JS callback function
// from down in the C level.  We are looking for a solution here using the
// Node.js FFI mechanism. TBD
// A smaller issue is passing in the EP_TIME_SPEC timeout argument (a C struct).
// Node.js ref-struct should provide a straight-forward FFI implementation.
// This smaller issue is also TBD.
{
    var rv_estat;

    if (debug) {
	console.log("gdpjs.js: gdp_gcl_subscribe_no_timeout_no_callback_js(" + gclh + ", " + firstrec + ", " + numrecs + ")");
    }

    // EP_TIME_SPEC *timeout, gdp_gcl_sub_cbfunc_t cbfunc, void *cbarg 
    // are all null here for a simpler subscribe variant.  See gdp/gdp-api.[ch]
    // gdp_gcl_subscribe() for details.
    rv_estat = libgdp.gdp_gcl_subscribe(gclh, firstrec, numrecs,
                                        null, null, null);
    if (debug) {
	console.log("gdpjs.js: gdp_gcl_subscribe_no_timeout_no_callback_js(): done: " + rv_estat);
    }

    return rv_estat;
};

/* EP_STAT */
function gdp_gcl_multiread_no_callback_js(gclh, firstrec, numrecs)
// This is a limited-functionality version of
//   libgdp.gdp_gcl_multiread(gclh, firstrec, numrecs, cbfunc, cbarg);
// See the comments on callbacks above for:
//    gdp_gcl_subscribe_no_timeout_no_callback_js() .
{

    var rv_estat;
    // gdp_gcl_sub_cbfunc_t cbfunc, void *cbarg are both null here for a
    // simpler multiread variant.  See gdp/gdp-api.[ch] gdp_gcl_multiread() .
    rv_estat = libgdp.gdp_gcl_multiread(gclh, firstrec, numrecs,
                                        null, null);
    return rv_estat;
}

/* C: gdp_event_t *  <==>  node-ffi: gdp_event_tPtr  <==>  JS: Object */
function gdp_event_next_js( gclh, /* EP_TIME_SPEC */ timeout)
// Return value can be viewed by JS as an opaque handle for a gdp_event.
// Returns null iff there is no event pending when wait == true.
{
    var gev_Ptr;
    if (debug) {
	console.log("gdpjs.js: gdp_event_next_js(" + gclh + ", " + timeout + ")");
    }

    gev_Ptr = libgdp.gdp_event_next(gclh, timeout);
    return gev_Ptr;
}

/* Integer */
function gdp_event_gettype_js(gev_Ptr)
// Arg gev_Ptr can be viewed by JS as an opaque handle for a gdp_event.
// Return value is an integer gdp event code -- see libgdp_h.js.
// If gev_Ptr == null, return null.  TBD: what if gev_Ptr is not present?
{
    var evtype_int;
    if (gev_Ptr == null) return null; // protect libgdp. call below
    // Note, if gev_Ptr == null, fails libgdp assertion with a runtime Abort.
    evtype_int = libgdp.gdp_event_gettype(gev_Ptr);
    return evtype_int
}

/* C: gdp_gcl_t *  <==>  JS: Object */
function gdp_event_getgcl_js(gev_Ptr)
// Arg gev_Ptr can be viewed by JS as an opaque handle for a gdp_event.
// Return value can be viewed by JS as an opaque handle for a gdp_gcl.
// If gev_Ptr == null, return null.  TBD: what if gev_Ptr is not present?
{
    if (gev_Ptr == null) return null; // protect libgdp. call below
    // Note, if gev_Ptr == null, fails libgdp assertion with a runtime Abort.
    return libgdp.gdp_event_getgcl(gev_Ptr);
}

/* C: gdp_datum_t *  <==>  JS: Object */
function gdp_event_getdatum_js(gev_Ptr)
// Arg gev_Ptr can be viewed by JS as an opaque handle for a gdp_event.
// Return value can be viewed by JS as an opaque handle for a gdp_datum.
// If gev_Ptr == null, return null.  TBD: what if gev_Ptr is not present?
{
    if (gev_Ptr == null) return null; // protect libgdp. call below
    // Note, if gev_Ptr == null, fails libgdp assertion with a runtime Abort.
    return libgdp.gdp_event_getdatum(gev_Ptr);
}

exports.gdp_event_getdatum_js = gdp_event_getdatum_js;

/* void */
function gdp_datum_print_stdout_js(datum)
// Arg datum can be viewed by JS as an opaque handle for a gdp_datum.
{
    /* void */
    libgdpjs.gdp_datum_print_stdout(datum);
}

exports.gdp_datum_print_stdout_js = gdp_datum_print_stdout_js;

/* EP_STAT */
function gdp_event_free_js(gev_Ptr)
// Arg gev_Ptr can be viewed by JS as an opaque handle for a gdp_event.
{
    return libgdp.gdp_event_free(gev_Ptr);
}

exports.gdp_event_free_js = gdp_event_free_js;

/* EP_STAT */
function gdp_gcl_close_js(gcl_Ptr) {
    return libgdp.gdp_gcl_close(gcl_Ptr);
}

exports.gdp_gcl_close_js = gdp_gcl_close_js;

/* void */
function fflush_all_js()
// Explicit print buffer flush; sometimes Node.js may not empty buffers
{
    /* void */
    libgdpjs.fflush_all();
}

/* void */
function gdp_datum_free_js(datum) {
    /* void */
    libgdp.gdp_datum_free(datum);
}



// Some GDP-related utility string manipulation functions ===================


/* Integer */
function String_to_Int( /* String */ str) {
    if (/^(\-|\+)?([0-9]+|Infinity)$/.test(str)) return Number(str);
    return NaN;
}


/* String */
function array_to_escaped_String( /* Array */ arry)
// Helpful for printing SHA-256 internal gcl names.
{
    var iToH = ["0", "1", "2", "3", "4", "5", "6", "7",
                "8", "9", "A", "B", "C", "D", "E", "F"
               ];
    var arry_as_string = "";
    for (var i = 0; i < arry.length; i++) {
        var hiHexDigit = iToH[Math.floor(arry[i] / 16)];
        var loHexDigit = iToH[arry[i] % 16];
        arry_as_string = arry_as_string + jsesc(String.fromCharCode(arry[i]));
        // console.log("array_to_escaped_String: ", i, arry[i],
        //              hiHexDigit, loHexDigit, arry_as_string ); 
    }
    return arry_as_string;
}


/* String */
function array_to_String( /* Array */ arry)
// Helpful for printing SHA-256 internal gcl names.
{
    var iToH = ["0", "1", "2", "3", "4", "5", "6", "7",
                "8", "9", "A", "B", "C", "D", "E", "F"
               ];
    var arry_as_string = "";
    for (var i = 0; i < arry.length; i++) {
        var hiHexDigit = iToH[Math.floor(arry[i] / 16)];
        var loHexDigit = iToH[arry[i] % 16];
        arry_as_string = arry_as_string + String.fromCharCode(arry[i]);
        // console.log("array_to_String: ", i, arry[i],
        //              hiHexDigit, loHexDigit, arry_as_string ); 
    }
    return arry_as_string;
}


/* String */
function array_to_HexDigitString( /* Array */ arry)
// Helpful for printing SHA-256 internal gcl names.
{
    var iToH = ["0", "1", "2", "3", "4", "5", "6", "7",
                "8", "9", "A", "B", "C", "D", "E", "F"
               ];
    var arry_as_hex = "";
    for (var i = 0; i < arry.length; i++) {
        var hiHexDigit = iToH[Math.floor(arry[i] / 16)];
        var loHexDigit = iToH[arry[i] % 16];
        arry_as_hex = arry_as_hex + hiHexDigit + loHexDigit;
        // console.log("array_to_HexDigitString: ", i, arry[i],
        //              hiHexDigit, loHexDigit, arry_as_hex ); 
    }
    return arry_as_hex;
}


/* String */
function buffer_to_HexDigitString( /* Buffer */ bfr)
// Helper function for buffer_info_to_string() .
{
    var iToH = ["0", "1", "2", "3", "4", "5", "6", "7",
                "8", "9", "A", "B", "C", "D", "E", "F"
               ];
    var bfr_as_hex = "";
    for (var i = 0; i < bfr.length; i++) {
        var hiHexDigit = iToH[Math.floor(bfr[i] / 16)];
        var loHexDigit = iToH[bfr[i] % 16];
        bfr_as_hex = bfr_as_hex + hiHexDigit + loHexDigit;
        // console.log("buffer_to_HexDigitString: ", i, bfr[i],
        //              hiHexDigit, loHexDigit, bfr_as_hex ); 
    }
    return bfr_as_hex;
}


/* String */
function buffer_info_to_string( /* string */ bfrName, /* Buffer */ bfr)
// Used for debugging node-ref "types" which may be implemented as a JS Buffer.
{
    var isNonNullObject = (bfr != null) && (typeof (bfr) == 'object');
    var str = "";
    // console.log("buffer_info_to_string: " );
    // console.log("isNonNullObject =\"" + isNonNullObject + "\"" );
    // console.log("bfr =\"" + bfr + "\"" );
    // console.log("typeof(bfr) =\"" + typeof(bfr) + "\"" );
    //if ( typeof(bfr) == 'object' && bfr.constructor.name == "Buffer" )
    if (isNonNullObject) {
        if (bfr.constructor.name == "Buffer") {
            str = str + bfrName + " =\"" + bfr + "\"\n";
            str = str + "typeof(" + bfrName + ") =\"" + typeof (bfr) + "\"\n";
            str = str + bfrName + ".constructor.name =\"" +
                bfr.constructor.name + "\"\n";
            str = str + bfrName + ".length =\"" + bfr.length + "\"\n";
            str = str + "buffer_to_HexDigitString(" + bfrName + ") =\"" +
                buffer_to_HexDigitString(bfr) + "\"\n";
            str = str + bfrName + ".toString() =\"" + bfr.toString() + "\"\n";
            str = str + "ref(" + bfrName + ").getType =\"" +
                ref.getType(bfr) + "\"\n";
            str = str + bfrName + ".deref =\"" + bfr.deref() + "\"";
        } else {
            str = "\"" + bfrName + "\" is not a Buffer object, it is a \"" +
                bfr.constructor.name + "\"";
        }
    } else {
        str = "\"" + bfrName +
            "\" is not a non-null Buffer object, typeof = \"" +
            typeof (bfr) + "\", value = \"" + bfr + "\"";
    }
    return str;
} /* end function buffer_info_to_string() */



exports.getDatumAsString = function(datum) {
    return gdp_datum_buf_as_string(datum);
}

/* String */
function gdp_datum_buf_as_string(datum)
// Return the bytes of a GDP datum as a JS String (which is able to
// contain nulls).
{
    var datum_dlen = libgdp.gdp_datum_getdlen(datum);

    // DEBUG - keep this output here until we've looked this code over more.
    DEBUG_get_datum_buf_as_string = false;

    if (DEBUG_get_datum_buf_as_string) {
        console.log("get_datum_buf_as_string: After call to gdp_datum_print() gcp_datum_getdlen() = \"" + datum_dlen + "\"");
    }
    var gdp_buf = libgdp.gdp_datum_getbuf(datum);
    if (DEBUG_get_datum_buf_as_string) {
        console.log("get_datum_buf_as_string: After call to gdp_datum_getbuf() gcp_datum_getbuf() = \"" + gdp_buf + "\"");
    }
    // curItem will hold the contents of the GCL item to be read here
    var curItem = new buf_t(1000); // TBD check size??
    var gdp_buf_size = libgdp.gdp_buf_read(gdp_buf, curItem, curItem.length);
    if (DEBUG_get_datum_buf_as_string) {
        console.log("get_datum_buf_as_string: After call to gdp_buf_read() gdp_buf_read() = \"" + gdp_buf_size + "\"");
        console.log("get_datum_buf_as_string: After call to gdp_buf_read() curItem = \"" + curItem.toString() + "\"\n");
    }
    var rvString = '';
    for (var i = 0; i < gdp_buf_size; i++) {
        rvString = rvString + String.fromCharCode(curItem[i]);
    }
    if (DEBUG_get_datum_buf_as_string) {
        console.log("get_datum_buf_as_string: rvString = '" + rvString + "'");
    }
    return rvString;
} /* function get_datum_buf_as_string() */

exports.gdp_datum_buf_as_string = gdp_datum_buf_as_string;

// Some temporary test output ===============================================

// We usually call this test routine just after we set up the server in the 
// Node.js global scope.  Eyeball the console.log() output to check the tests.
// This code also serves as temporary examples of use.
// If still useful, we will move this code to a real test routine.

function misc_lower_level_inline_tests( /* Boolean */ do_tests) {
    /* Simple tests of:
       libep.ep_time_accuracy()
       libep.ep_time_now()
       ep_time_as_string_js()
       ep_time_now_js()
       and EP_TIME_SPEC_struct to pass arguments in and out.
       Write to console.log().
    */

    if (!do_tests) return;

    console.log('A:');
    var tacc /* float */ = libep.ep_time_accuracy();
    console.log('ep_time_accuracy() = ' + tacc);

    var eptspec = new EP_TIME_SPEC_struct;
    estat = libep.ep_time_now(eptspec.ref());
    console.log('eptspec.tv_sec = \"' + eptspec.tv_sec + '\"');
    console.log('eptspec.tv_nsec = \"' + eptspec.tv_nsec + '\"');
    console.log('eptspec.tv_accuracy = \"' + eptspec.tv_accuracy + '\"');

    // timebuf will hold the EP_TIME_SPEC we read above in String format.
    var timebuf = ep_time_as_string_js(eptspec.ref(), true);
    console.log('timebuf = \"' + timebuf + '\"');

    // to slow things down to be able to follow console.log()
    // sleep.sleep(5);
    console.log('B:');
    estat = ep_time_now_js(eptspec.ref());
    console.log('eptspec.tv_sec = \"' + eptspec.tv_sec + '\"');
    console.log('eptspec.tv_nsec = \"' + eptspec.tv_nsec + '\"');
    console.log('eptspec.tv_accuracy = \"' + eptspec.tv_accuracy + '\"');

    var timebuf = ep_time_as_string_js(eptspec.ref(), true);
    console.log('timebuf = \"' + timebuf + '\"');


    // DEBUG
    // Tests of setTimeout() and setInteral() - remove when gdpREST_server.js
    // subscription has been proven.

    if (false) {
        // The only way out now is for the Node.js event loop to terminate.
        // For now, this is by a ^C from the terminal it was started in or some
        // other process signal.

        var delay_Tout = 5000; // milliseconds
        var aTimeoutObject;
        var nCalls_cbf_Tout = 0;

        function cbf_Tout(a1, a2) {
            nCalls_cbf_Tout++;
            console.log('In cbf_Tout nCalls_cbf_Tout = %d', nCalls_cbf_Tout);
            console.log('a1 = \"' + a1 + '\"');
            console.log('a2 = \"' + a2 + '\"');
            clearInterval(anIntervalObject);
        }

        var cbf_Tout_arg_1 = 1,
            cbf_Tout_arg_2 = 2;

        // setTimeout(callback, delay, [arg], [...])
        aTimeoutObject = setTimeout(cbf_Tout, delay_Tout,
                                    cbf_Tout_arg_1, cbf_Tout_arg_2);

        var delay_Int = 1000; // milliseconds
        var anIntervalObject;
        var nCalls_cbf_Int = 0;

        function cbf_Int(a1, a2) {
            nCalls_cbf_Int++;
            console.log('In cbf_Int nCalls_cbf_Int = %d', nCalls_cbf_Int);
            console.log('a1 = \"' + a1 + '\"');
            console.log('a2 = \"' + a2 + '\"');
        }

        var cbf_Int_arg_1 = 1,
            cbf_Int_arg_2 = 2;

        // setInterval(callback, delay, [arg], [...])
        anIntervalObject = setInterval(cbf_Int, delay_Int,
                                       cbf_Int_arg_1, cbf_Int_arg_2);


        // clearTimeout( aTimeoutObject );

        // clearInterval( anIntervalObject );
    } /* end if ( false ) */


    if (false) {
        // var timeout  = 20;  // seconds
        var timeout = 5; // seconds
        // will be set true after timeout seconds by cbf_Tout() as a result of
        // setTimeout( cbf_Tout, ...).  We stop polling below when it goes true.
        var monitoring_done = false;
        console.log('monitoring_done = \"' + monitoring_done + '\"');

        // var delay_Tout = 5000;  // milliseconds
        var delay_Tout = timeout * 1000; // seconds to milliseconds
        var aTimeoutObject;
        var nCalls_cbf_Tout = 0;

        function cbf_Tout(a1, a2) {
            nCalls_cbf_Tout++;
            console.log('In cbf_Tout nCalls_cbf_Tout = %d', nCalls_cbf_Tout);
            console.log('a1 = \"' + a1 + '\"');
            console.log('a2 = \"' + a2 + '\"');
            fflush_all_js(); // sometimes Node.js may not empty buffers
            clearInterval(anIntervalObject)
            monitoring_done = true;
            console.log('monitoring_done = \"' + monitoring_done + '\"');
        }

        var cbf_Tout_arg_1 = 1,
            cbf_Tout_arg_2 = 2;

        // setTimeout(callback, delay, [arg], [...])
        aTimeoutObject = setTimeout(cbf_Tout, delay_Tout,
                                    cbf_Tout_arg_1, cbf_Tout_arg_2);

        var delay_Int = 1000; // milliseconds
        var anIntervalObject;
        var nCalls_cbf_Int = 0;

        function cbf_Int(a1, a2) {
            nCalls_cbf_Int++;
            console.log('In cbf_Int nCalls_cbf_Int = %d', nCalls_cbf_Int);
            console.log('a1 = \"' + a1 + '\"');
            console.log('a2 = \"' + a2 + '\"'); {
                console.log('Do a loop');
            }
        }

        var cbf_Int_arg_1 = 1,
            cbf_Int_arg_2 = 2;

        // setInterval(callback, delay, [arg], [...])
        anIntervalObject = setInterval(cbf_Int, delay_Int,
                                       cbf_Int_arg_1, cbf_Int_arg_2);


        // while ( ! monitoring_done )
        // {	console.log( 'Do a loop' ); }
        console.log('Done looping');

    } /* end if ( false ) */



} /* end function misc_lower_level_inline_tests() */

// ========================================================================
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

/** Write to a Global Data Plane log.
 *  
 * @param gdpd_addr gdp daemon's <host:port>; if null, use default
 * "127.0.0.1:2468" @param gclName name of existing GCL @param
 * logdxname String: the name of the log server.  Use os.hostname()
 * for local 
 *
 * @param gcl_name If gcl_append is true, name of existing GCL; 
 * if gcl_append is false, ignored.  A new GCL will be created.
 *
 * @param logdxname String: the name of the log server.  Use
 * os.hostname() for local.
 * 
 * @param gcl_append Boolean: append to an existing GCL
 *
 * @param recsrc The source of the records. recordSource == -1:
 * read the gcl records to be written from stdin with prompts to and
 * echoing for the user on stdout. recordSource = 0: read the gcl
 * records from the Array recordArray. In this case only, for each gcl
 * record written we will return in the parallel array recordArrayOut:
 * recno: Integer, time_stamp: <timestamp_as_String>. Note,
 * recordArrayOut must be in the incoming parameter list. recordSource
 * > 0 write recordSource records with automatically generated
 * content: the integers starting at 1 and going up to recsrc,
 * inclusive.
 *
 * @param recarray if recsrc is 0, then the records are written to 
 * recordArray.
 *
 * @param conout Boolean: iff true, for recsrc = -1, prompt user
 * and echo written records on stdout; for recsrc = 0, echo written
 * records on stdout, not recommended; for recsrc > 0, Note, echoed
 * written records also include GCL record number (recno) and
 * timestamp.
 *
 * @param recarray_out Array: see recsrc = 0, above.
 *
 * @return error_isok: false|true, error_code: EP_STAT, error_msg: String, gcl_name: String
 */

function write_gcl_records(gdpd_addr, gcl_name, logdxname, gcl_append,
    recsrc, recarray, conout, recarray_out)

// gdpd_addr    gdp daemon's <host:port>; if null, use default "127.0.0.1:2468"
// gcl_name     if gcl_append is true, name of existing GCL; 
//              if gcl_append is false, ignored.  A new GCL will be created.
// logdxname    String: the name of the log server.  Use os.hostname() for local
// gcl_append   Boolean: append to an existing GCL
// recsrc = -1  read the gcl records to be written from stdin with
//              prompts to and echoing for the user on stdout
// recsrc =  0  read the gcl records from the Array recarray
//              In this case only, 
//              For each gcl record written we will return in the parallel array
//              recarray_out:
//                 { recno: Integer, time_stamp: <timestamp_as_String> }.
//              Note, recarray_out must be in the incoming parameter list above.
// recsrc >  0  write recsrc records with automatically generated
//              content: the integers starting at 1 and going up to
//              recsrc, inclusive.
// conout       Boolean: iff true,
//              for recsrc = -1, prompt user and echo written records on stdout;
//              for recsrc = 0, echo written records on stdout, not recommended;
//              for recsrc > 0,  "
//              Note, echoed written records also include GCL record number
//              (recno) and timestamp.
// recarray_out Array: see recsrc = 0, above.
//
// TBD: Note, there still may be undesired output via console.log() and
// console.error(). Check all uses of  if ( conout == true ) below.
// TBD: We could also return recarray_out[] for recsrc > 0. And, even
// augmented with the manually entered record content, for recsrc = -1.
{
    if (debug) {
        console.log("gdpjs.js: write_gcl_records() start: recsrc: " + recsrc);
    }
    // internal variables for historical reasons
    var xname = gcl_name;
    var append = gcl_append;
    var numrecs = recsrc;

    var ebuf = new Array(200 + 1).join(" "); // long enough??

    // Local working variables
    var gcl_Ptr; // gclh
    var estat; // EP_STAT
    //var gcliname = ref.alloc(gcl_name_t);
    var gcliname = new gcl_name_t(32);

    // Note, buf is re-allocated below for each item string read from stdin
    var buf = new buf_t(10);


    if (debug) {
        console.log("gdpjs.js: write_gcl_records() about to call gdp_init_js()");
    }
    estat = gdp_init_js( /* String */ gdpd_addr);
    if ( ! ep_stat_isok_js(estat) ) {
        var emsg = "gdpjs.js: write_gcl_records(): gdp_init_js() is not ok";
        console.log(emsg);
        console.log(ep_stat_tostr_js(estat, ebuf, ebuf.length));
        rv = {
            error_isok: ((ep_stat_isok_js(estat) == 0) ? false : true),
            error_code: ("0x" + estat.toString(16)),
            error_msg: emsg,
            gcl_name: gcl_name
        };
        return rv;
    }
    // TBD: check for errors:  if ( ! ep_stat_isok_js(estat) )

    if (conout == true) { // allow thread to settle to avoid interspersed debug output
        // TBD check this
        sleep.sleep(1); // needed only for stdout and stderr
    }

    if (xname == null) {
        // TBD: check signature
        if (debug) {
            console.log("gdpjs.js: write_gcl_records() xname is null. About to call gdp_gcl_create_js()");
        }
        var gclcreaterv = gdp_gcl_create_js(null);
        estat = gclcreaterv.error_code;
        gcl_Ptr = gclcreaterv.gclH;
        // grab the name of the newly created gcl
        gcl_name = gdp_get_pname_from_gclh_js(gcl_Ptr);
    } else {
        if (debug) {
            console.log("gdpjs.js: write_gcl_records() about to call gdp_parse_name_js():" + xname + ", " + gcliname);
            console.log("gdpjs.js: write_gcl_records() gcliname:");

            for (i=0; i<gcliname.length; i++) {
                process.stdout.write(gcliname[i].toString(16));
            }
            console.log("\n");
        }

        gdp_parse_name_js(xname, gcliname);

        if (debug) {
            console.log("gdpjs.js: write_gcl_records() gcliname:");
            for (i=0; i<gcliname.length; i++) {
                process.stdout.write(gcliname[i].toString(16));
            }
            console.log("\n");
        }

        // TBD: especially check for gcliname already existing??!!

        if (append) {
            // TBD: check signature
            if (debug) {
                console.log("gdpjs.js: write_gcl_records() about to call gdp_gcl_open_js()");
            }
            var gclopenrv = gdp_gcl_open_js(gcliname, GDP_MODE_AO);
            estat = gclopenrv.error_code;
            gcl_Ptr = gclopenrv.gclH;
            if ( ! ep_stat_isok_js(estat) ) {
                var emsg = "gdpjs.js: write_gcl_records(), gdp_gcl_open_js() returned a non-ok ep_stat.  Check to be sure that the log name exists.";
                console.log(emsg);
                console.log(ep_stat_tostr_js(estat, ebuf, ebuf.length));
                rv = {
                    error_isok: ((ep_stat_isok_js(estat) == 0) ? false : true),
                    error_code: ("0x" + estat.toString(16)),
                    error_msg: emsg,
                    gcl_name: gcl_name
                };
                return rv;            
            }
        } else {
            // TBD: check signature
            if (debug) {
                console.log("gdpjs.js: write_gcl_records() about to call gdp_gcl_create_js()");
            }
            var gclPtrPtr = ref.alloc(gdp_gcl_tPtrPtr);
            var gclcreaterv = gdp_gcl_create_js(xname, logdxname, gclPtrPtr);
            estat = gclcreaterv.error_code;
            if ( ! ep_stat_isok_js(estat) ) {
                var emsg = "gdpjs.js: write_gcl_records(), gdp_gcl_create_js() returned a non-ok ep_stat";
                console.log(emsg);
                console.log(ep_stat_tostr_js(estat, ebuf, ebuf.length));
                rv = {
                    error_isok: ((ep_stat_isok_js(estat) == 0) ? false : true),
                    error_code: ("0x" + estat.toString(16)),
                    error_msg: emsg,
                    gcl_name: gcl_name
                };
                return rv;

            }
            gcl_Ptr = gclcreaterv.gclH;
        }
    }
    // TBD: check for errors:  if ( ! ep_stat_isok_js(estat) )

    // don't always send gcl name to stdout
    if (conout == true) {
        gdp_gcl_print_stdout_js(gcl_Ptr);
    }

    var datum;
    datum = gdp_datum_new_js();

    if (numrecs < 0) {
        // Read records from stdin, prompting & echoing to user on stdout

        // TBD is if(conout) is correct here?
        if (conout == true) {
            console.log("\nStarting to read input - ^D to end");
        }
        var rvget; /* String */
        // really a dummy for gets's parameter; we ignore its value.
        // Just trying to avoid possible buffer overruns inside gets().
        var strbuf = new Array(200 + 1).join(" "); // long enough??

        while ((rvgets = libc.gets(strbuf)) != null) {
            var buf = new buf_t(rvgets.length + 1); // we'll tack on a \0
            for (var i = 0; i < rvgets.length; i++) {
                buf[i] = rvgets.charCodeAt(i); // not sure if really necessary
            }
            buf[rvgets.length] = 0; // Hopefully, interpreted in C as \0
            // TBD is if(conout) is correct here?
            if (conout == true) {
                console.log("Got input %s%s%s", "<<", rvgets, ">>");
            }

            if (debug) {
                console.log("gdpjs.js: write_gcl_records() about to call gdp_gcl_publish_buf_js()");
            }
            estat = gdp_gcl_publish_buf_js(gcl_Ptr, datum, buf);
            // TBD: check for errors:  if ( ! ep_stat_isok_js(estat) )
            if (debug) {
                console.log("gdpjs.js: write_gcl_records() done with to call gdp_gcl_publish_buf_js()");
            }

            // TBD is if(conout) is correct here?
            if (conout == true) {
                gdp_datum_print_stdout_js(datum);
            }

        } /* end while */
    } else if (numrecs > 0) {
        // Generate numrecs records with contents = integers 1 to numrecs.

        // For each gcl record written we will return in the parallel array
        // recarray_out:
        //    { recno: Integer, time_stamp: <timestamp_as_String> }.
        // Note, recarray_out must be in the incoming parameter list above.

        for (var crec = 1; crec <= numrecs; crec++) {
            var rvgets; /* String */
            rvgets = crec.toString();

            var buf = new buf_t(rvgets.length + 1); // we'll tack on a \0
            for (var i = 0; i < rvgets.length; i++) {
                buf[i] = rvgets.charCodeAt(i); // not sure if really necessary
            }
            buf[rvgets.length] = 0; // Hopefully, interpreted in C as \0

            // TBD is if(conout) is correct here?
            if (conout == true) {
                console.log("Got input %s%s%s", "<<", rvgets, ">>");
            }

            estat = gdp_gcl_publish_buf_js(gcl_Ptr, datum, buf);
            // TBD: check for errors:  if ( ! ep_stat_isok_js(estat) )

            // grab record number and timestamp for this newly written record
            var ts = gdp_datum_getts_as_string_js(datum, true /* format */ );
            // TBD: below check for 64-bit integer return type, gdp_recno_t
            var rn = gdp_datum_getrecno_js(datum);
            recarray_out[crec - 1] = {
                recno: rn,
                time_stamp: ts
            };

            // TBD is if(conout) is correct here?
            if (conout == true) {
                gdp_datum_print_stdout_js(datum);
            }
        } /* end for ( var crec = 1 ...) */
    } else {
        // numrecs == 0 

        // Write contents of recarray[] to the gcl

        // For each gcl record written we will return in the parallel array
        // recarray_out:
        //    { recno: Integer, time_stamp: <timestamp_as_String> }.
        // Note, recarray_out must be in the incoming parameter list above.

        for (var crec = 0; crec < recarray.length; crec++) {
            var rvgets; /* String */
            rvgets = recarray[crec].toString();
            
            var buf = new buf_t(rvgets.length); // No need to tack on a \0
            for (var i = 0; i < rvgets.length; i++) {
                buf[i] = rvgets.charCodeAt(i); // not sure if really necessary
            }
            //buf[rvgets.length] = 0; // Hopefully, interpreted in C as \0
            if (conout == true) {
                console.log("Got input %s%s%s", "<<", rvgets, ">>");
            }

            estat = gdp_gcl_publish_buf_js(gcl_Ptr, datum, buf);
            // TBD: check for errors:  if ( ! ep_stat_isok_js(estat) )

            // grab record number and timestamp for this newly written record
            var ts = gdp_datum_getts_as_string_js(datum, true /* format */ );
            // TBD: below check for 64-bit integer return type, gdp_recno_t
            var rn = gdp_datum_getrecno_js(datum);
            recarray_out[crec] = {
                recno: rn,
                time_stamp: ts
            };

            if (conout == true) {
                gdp_datum_print_stdout_js(datum);
            }
        } /* end for ( var crec = 0 ...) */
    }

    gdp_datum_free_js(datum);

    estat = gdp_gcl_close_js(gcl_Ptr);

    // TBD: fix this error return - see corresponding location in writer-test.js
    // string.repeat not available for us here in ECMASscript<6
    var str = new Array(200 + 1).join(" "); // long enough??
    var emsg = ("exiting with status " +
        ep_stat_tostr_js(estat, str, str.length));
    if (conout == true) {
        fflush_all_js(); // sometimes Node.js may not empty buffers
        console.error(emsg);
    }
    // console.error( "exiting with status %s",
    // 				ep_stat_tostr_js(estat, str, str.length) );
    // OLD return ( ! ep_stat_isok_js(estat) );
    rv = {
        error_isok: ((ep_stat_isok_js(estat) == 0) ? false : true),
        error_code: ("0x" + estat.toString(16)),
        error_msg: emsg,
        gcl_name: gcl_name
    };
    return rv;

} /* end function write_gcl_records() */

exports.write_gcl_records = write_gcl_records;

