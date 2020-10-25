/////////////////////////

/* vim: set ai sw=4 sts=4 ts=4 : */

var ffi = require('ffi')
var ref = require('ref')
var ref_array = require('ref-array')

var char_t = ref.types.char;
var buf_t = ref_array(char_t);

// From ep/ep_dbg.h
var libep = ffi.Library('../../../libs/libep-3.0', {
    'ep_dbg_init': ['void', []],
    'ep_dbg_set': ['void', ['string']],
    'ep_stat_tostr': ['string', [ref.types.uint32, buf_t, 'size_t']],
    'ep_time_nanosleep': ['void', ['int64']],
})

// From gdp/gdp.h => <inttypes.h> ==> <stdint.h> ==> <sys/_types/_int32_t.h>
var int32_t = ref.types.int32;

// From gdp/gdp.h => <stdbool.h>
var bool_t = ref.types.int;

// From gdp/gdp.h
//CJS // the internal name of a GCL
//CJS typedef uint8_t gcl_name_t[32];
var uint8_t = ref.types.uint8;
var gcl_name_t = ref_array(uint8_t);

// From gdp/gdp.h
//CJS // a GCL record number
//CJS typedef int64_t                         gdp_recno_t;
var gdp_recno_t = ref.types.int64; //?? check this

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
var gdp_iomode_t = ref.types.int; //?? check this - enum === int ?


//CJS // an open handle on a GCL (opaque)
//CJS typedef struct gdp_gcl          gdp_gcl_t;
var gdp_gcl_t = ref.types.void; // opaque for us up here in JS
var gdp_gcl_tPtr = ref.refType(gdp_gcl_t);
var gdp_gcl_tPtrPtr = ref.refType(gdp_gcl_tPtr);

//CJS typedef struct gdp_datum        gdp_datum_t;
var gdp_datum_t = ref.types.void; // opaque for us up here in JS
var gdp_datum_tPtr = ref.refType(gdp_datum_t);
var gdp_datum_tPtrPtr = ref.refType(gdp_datum_tPtr); //?? not used yet??

//CJS typedef struct gdp_event        gdp_event_t;
var gdp_event_t = ref.types.void; // opaque for us up here in JS
var gdp_event_tPtr = ref.refType(gdp_event_t);
var gdp_event_tPtrPtr = ref.refType(gdp_event_tPtr); //?? not used yet??

// From gdp/gdp_buf.h
//CJS typedef struct evbuffer gdp_buf_t;
var gdp_buf_t = ref.types.void; // opaque for us up here in JS
var gdp_buf_tPtr = ref.refType(gdp_buf_t);
var gdp_buf_tPtrPtr = ref.refType(gdp_buf_tPtr); //?? not used yet??


var voidPtrType = ref.refType(ref.types.void);

var libgdp = ffi.Library('../../../libs/libgdp-0.5', {

    // From gdp/gdp.h
    //CJS // free an event (required after gdp_event_next)
    //CJS extern EP_STAT                  gdp_event_free(gdp_event_t *gev);
    'gdp_event_free': ['ulong', [gdp_event_tPtr]],

    //CJS // get next event (fills in gev structure)
    //CJS extern gdp_event_t              *gdp_event_next(bool wait);
    'gdp_event_next': [gdp_event_tPtr, [bool_t]],

    //CJS // get the type of an event
    //CJS extern int                       gdp_event_gettype(gdp_event_t *gev);
    'gdp_event_gettype': ['int', [gdp_event_tPtr]],

    //CJS // get the GCL handle
    //CJS extern gdp_gcl_t                *gdp_event_getgcl(gdp_event_t *gev);
    'gdp_event_getgcl': [gdp_gcl_tPtr, [gdp_event_tPtr]],

    //CJS // get the datum
    //CJS extern gdp_datum_t              *gdp_event_getdatum(gdp_event_t *gev);
    'gdp_event_getdatum': [gdp_datum_tPtr, [gdp_event_tPtr]],

    //CJS // initialize the library
    //CJS EP_STAT gdp_init( const char *gdpd_addr );          // address of gdpd
    'gdp_init': ['ulong', ['string']],

    //CJS // create a new GCL
    //CJS EP_STAT gdp_gcl_create( gcl_name_t, gdp_gcl_t ** ); // pointer to result GCL handle
    'gdp_gcl_create': ['ulong', [gcl_name_t, gdp_gcl_tPtrPtr]],

    //CJS // open an existing GCL
    //CJS extern EP_STAT  gdp_gcl_open( gcl_name_t name, gdp_iomode_t rw, gdp_gcl_t **gclh);              // pointer to result GCL handle
    'gdp_gcl_open': ['ulong', [gcl_name_t, gdp_iomode_t, voidPtrType, gdp_gcl_tPtrPtr]],

    //CJS // close an open GCL
    //CJS EP_STAT  gdp_gcl_close( gdp_gcl_t *gclh);           // GCL handle to close
    'gdp_gcl_close': ['ulong', [gdp_gcl_tPtr]],

    //CJS // parse a (possibly human-friendly) GCL name
    //CJS EP_STAT gdp_parse_name( const char *ext, gcl_name_t internal );
    'gdp_parse_name': ['ulong', ['string', gcl_name_t]],

    //CJS // allocate a new message
    //CJS gdp_datum_t             *gdp_datum_new(void);
    'gdp_datum_new': [gdp_datum_tPtr, []],

    //CJS // free a message
    //CJS void                    gdp_datum_free(gdp_datum_t *);
    'gdp_datum_free': ['void', [gdp_datum_tPtr]],

    // From gdp/gdp.h
    //CJS // get the data buffer from a datum
    //CJS extern gdp_buf_t *gdp_datum_getbuf( const gdp_datum_t *datum );
    'gdp_datum_getbuf': [gdp_buf_tPtr, [gdp_datum_tPtr]],

    // From gdp/gdp_buf.h
    //CJS extern int gdp_buf_write( gdp_buf_t *buf, void *in, size_t sz );
    //'gdp_buf_write': [ 'int', [ gdp_buf_tPtr, 'pointer', 'size_t' ] ],
    //'gdp_buf_write': [ 'int', [ gdp_buf_tPtr, 'pointer', 'int' ] ],
    'gdp_buf_write': ['int', [gdp_buf_tPtr, buf_t, 'size_t']],

    // From gdp/gdp_buf.h
    //CJS extern size_t           gdp_buf_read( gdp_buf_t *buf, void *out, size_t sz);
    'gdp_buf_read': ['size_t', [gdp_buf_tPtr, buf_t, 'size_t']],

    'gdp_gcl_print': ['void', [gdp_gcl_tPtr, 'void']],

    // From gdp/gdp.h
    //CJS // append to a writable GCL
    //CJS extern EP_STAT  gdp_gcl_publish( gdp_gcl_t *gclh, gdp_datum_t *);
    //'gdp_gcl_publish': [ 'ulong', [ gdp_gcl_tPtr, gdp_datum_tPtr ] ],

    //CJS extern EP_STAT  gdp_gcl_subscribe(
    //CJS                                         gdp_gcl_t *gclh,                // readable GCL handle
    //CJS                                         gdp_recno_t start,              // first record to retrieve
    //CJS                                         int32_t numrecs,                // number of records to retrieve
    //CJS                                         EP_TIME_SPEC *timeout,  // timeout
    //CJS                                         gdp_gcl_sub_cbfunc_t cbfunc,
    //CJS
    //CJS         // callback function for next datum
    //CJS                                         void *cbarg);                   // argument passed to callback
    // Note, in our call to this function in do_multiread() below we do not
    //       use the last 3 (pointer) arguments.
    'gdp_gcl_subscribe': ['ulong', [gdp_gcl_tPtr, gdp_recno_t, int32_t, 'pointer', 'pointer', 'pointer']],

    //CJS // read from a readable GCL
    //CJS extern EP_STAT  gdp_gcl_read( gdp_gcl_t *gclh, gdp_recno_t recno, gdp_datum_t *datum);    // pointer to result message
    'gdp_gcl_read': ['ulong', [gdp_gcl_tPtr, gdp_recno_t, gdp_datum_tPtr]],

    // From gdp/gdp.h
    //CJS // get the data length from a datum
    //CJS extern size_t   gdp_datum_getdlen( const gdp_datum_t *datum);
    'gdp_datum_getdlen': ['size_t', [gdp_datum_tPtr]],

})



var gclname_arg = "6zvLBmrn5VUiPweLV9PYqPL_h-SFQZoV_Ht5XtZ9x-Y"
    // var gclname_arg = process.argv[ 3 ];
var firstrec = 1 // first item entered in the gcl
    //    var firstrec = 0  // most recently entered item in the gcl

//C  gcl_name_t gclname;
var gclname = ref.alloc(gcl_name_t);

libep.ep_dbg_set("*=10");

//C  char *gdpd_addr = NULL;
//var gdpd_addr = "127.0.0.1:2468";  // default port for a local gdpd
var gdpd_addr = "127.0.0.1:8007"; // default port for a local gdpd

//C // initialize the GDP library
//C estat = gdp_init(gdpd_addr);
estat = libgdp.gdp_init(gdpd_addr);

//C // allow thread to settle to avoid interspersed debug output
//C ep_time_nanosleep(INT64_C(100000000));		// 100 msec
//var sleep = require('sleep');
//sleep.usleep(100000000);		// 100 msec
//ep_time_nanosleep(100000000);		// 100 msec

estat = libgdp.gdp_parse_name(gclname_arg, gclname);


//C  // open the GCL; arguably this shouldn't be necessary
//C  estat = gdp_gcl_open(gclname, GDP_MODE_RO, &gclh);
var gcl = ref.alloc(gdp_gcl_t);
console.log("simple: gcl: " + gcl);
console.log(gcl);

var gclPtr = ref.alloc(gdp_gcl_tPtr);

console.log("simple: gclPtr: " + gclPtr);
console.log(gclPtr);

console.log("simple: writePointer()");
ref.writePointer(gclPtr, 0, gcl);

console.log("simple: gcl:" + gcl)
console.log(gcl);
console.log("simple: gclPtr:" + gclPtr)
console.log(gclPtr);


var gclPtrPtr = ref.alloc(gdp_gcl_tPtrPtr);

//ref.writePointer(gclPtrPtr, 0, gclPtr);
console.log("simple: gclPtrPtr: " + gclPtrPtr);
console.log(gclPtrPtr);

console.log("simple: about to print the gcl before open, it should be NULL or garbage");
var gcl_Ptr = gclPtrPtr.deref();
libgdp.gdp_gcl_print(gcl_Ptr, 0);

estat = libgdp.gdp_gcl_open(gclname, GDP_MODE_RO, ref.NULL, gclPtrPtr);
console.log("simple: after gdp_gcl_open()");
console.log("simple: gclPtrPtr:" + gclPtrPtr + " " + gclPtrPtr.address().toString(16));
console.log(gclPtrPtr);

gcl_Ptr = gclPtrPtr.deref();
console.log("simple: gcl_Ptr:" + gcl_Ptr + " " + gcl_Ptr.address().toString(16));
console.log(gcl_Ptr);
var gcl_1 = gcl_Ptr.deref();
console.log("simple: gcl_1:");
console.log(gcl_1);


console.log("simple: about to print the gcl after open");
libgdp.gdp_gcl_print(gcl_Ptr, 0);
console.log("simple: after gdp_gcl_print()");

var ebuf = ref.allocCString('123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890');

console.log("About to create ebuf");
//var ebuf = new buf_t(400);

//var ebuf = ref.alloc(ref.types.CString);
console.log("simple: after setting ebuf");
console.log("simple: ebuf: " + ebuf);
//console.log( "simple: ebuf.address(): " + ref.address(ebuf));
//var ebuf = new Array( 200 + 1 ).join( " " );  // long enough??
var ebuf2 = libep.ep_stat_tostr(estat, ebuf, ebuf.length);
console.log("simple: after ep_stat_tostr()");
console.log("after open, estat: " + ebuf2);




// From estat = do_simpleread(gcl_Ptr, firstrec, numrecs);

//C gdp_recno_t recno;
var recno;
recno = firstrec;

//C  while (numrecs < 0 || --numrecs >= 0)
//C  gdp_datum_t *datum = gdp_datum_new();
var datum;
console.log("simple: about to call gdp_datum_new()");
datum = libgdp.gdp_datum_new();
console.log("simple: after calling gdp_datum_new()");
// In this test program we do not free this datum

console.log("simple: About to read record #" + recno + " from GCL '%s'", gclname_arg);
console.log("simple: gcl_PtrPtr: " + gclPtrPtr);
console.log("simple: gclPtrPtr.address(): " + ref.address(gclPtrPtr));
console.log("simple: gcl_Ptr: " + gcl_Ptr);
//C // ask the GDP to give us a record
//C estat = gdp_gcl_read(gclh, recno, datum);
estat = libgdp.gdp_gcl_read(gcl_Ptr, recno, datum);
var ebuf2 = libep.ep_stat_tostr(estat, ebuf, ebuf.length);
console.log("after read, estat: " + ebuf2);

console.log("simple: Done reading record #" + recno + " from GCL '%s'", gclname_arg);
// Print out the contents of the datum's buffer we read
var datum_dlen = libgdp.gdp_datum_getdlen(datum);
var temp_gdp_buf = libgdp.gdp_datum_getbuf(datum);
var buf = new buf_t(1000); // hack size??
var temp_gdp_buf_size = libgdp.gdp_buf_read(temp_gdp_buf, buf, buf.length);
var aJSString = '';
for (var i = 0; i < temp_gdp_buf_size; i++) {
    aJSString = aJSString + String.fromCharCode(buf[i]);
}
console.log("read: '" + aJSString + "'");

//C gdp_gcl_close(gclh);
estat = libgdp.gdp_gcl_close(gcl_Ptr);
