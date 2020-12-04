/* vim: set ai sw=4 sts=4 ts=4 : */

// Support routines for Node.js Javascript programs accessing the
// GDP dynamic libraries, libgdp.xxx and libep.xxx as well as some
// GDP macros, and system (e.g., libc) routines.
//
// See libgdp_h.js for Node.js ffi "declarations" for these C routines
// to allow invoking them from JS.
//
// Alec Dara-Abrams
//  2014-10-28


#include "gdpjs_supt.h"
#include "gdp/gdp.h"
// We really don't want to include gdp_priv.h but we need struct gpd_gcl
// for our use of gclh->pname below.
#include "gdp/gdp_priv.h"
#include "ep/ep.h"
#include "ep/ep_stat.h"
// Seems to be a problem defining EP_STAT if ep/ep.h is not included before
// ep/ep_stat.h .  Check includes in ep_stat.h .


// DEBUG = 1 to include fprintf(stderr, ) debugging output; else = 0.
#define DEBUG 0


// EP_STAT_INT_EQUIV is needed for some casts below.
// We determine this by examning EP_STAT in ep/ep_stat.h .
// We assume these BIT counts exhaust EP_STAT.
#define EP_STAT_NBITS  ( _EP_STAT_SEVBITS + _EP_STAT_REGBITS +  \
            _EP_STAT_MODBITS + _EP_STAT_DETBITS )

#if   (EP_STAT_NBITS ==  32)
#define EP_STAT_INT_EQUIV  uint32_t
#elif (EP_STAT_NBITS ==  64)
#define EP_STAT_INT_EQUIV  uint64_t
#else
#error "The type EP_STAT (in ep/ep_stat.h) must be 32 or 64 bits long"
#endif

// EP_STAT_32_64 is needed for some casts below
// Note, even though EP_STAT is all caps, it is a C typedef struct, not a
// cpp macro.  Yes, we may be making some assumptions here about padding, etc.
// but we only use this type for casts, and don't access internal structure.
typedef union
{ EP_STAT            as_EP_STAT;
    EP_STAT_INT_EQUIV  as_32_64_t;
} EP_STAT_32_64;



// Some general libc-related functions

// Forces a flush() on stdout, on stderr, and on all open file descriptors.
// Node.js console.log(), console.error(), and process.stdxxx.write() are,
// evidently, buffered when writing to a file.  Output to a terminal seems
// to be unbuffered.
int fflush_stdout() { return ( fflush(stdout) ); }
int fflush_stderr() { return ( fflush(stderr) ); }
int fflush_all()    { return ( fflush( NULL ) ); }



// Some general libgdp/libep-related functions


// Returns the size in bytes of the libep error status code, EP_STAT .
size_t
sizeof_EP_STAT_in_bytes()
{
#if DEBUG
    fprintf( stderr,
            "sizeof_EP_STAT_in_bytes: sizeof(EP_STAT) = %lu (decimal-%%lu)\n",
            sizeof(EP_STAT)
             );
#endif
    return (sizeof(EP_STAT));
}


// ====================================================
// Wrap selected functions from gdp/

// From gdp/gdp.h

// We need these specialized ..._print_stdout() wrappers because Node.js JS
// cannot supply a true stdout FILE* for calls to the node-ffi FFI.

// Forwards to gcp_gcl_print( const gdp_gcl_t *gclh, stdout, , , );
//
// print a GCL (for debugging)
// extern void
// gdp_gcl_print(
//     const gdp_gcl_t *gclh,  // GCL handle to print
//     FILE *fp);              // file to print it to
// );
//
void
gdp_gcl_print_stdout(
        const gdp_gcl_t *gclh,  // GCL handle to print
        int detail,             // not used at this time
        int indent              // not used at this time
                     )
{
    gdp_gcl_print( gclh, stdout);
}


// Forwards to gdp_datum_print( const gdp_datum_t *datum, stdout );
//
// print a message (for debugging)
// extern void
// gdp_datum_print(
//     const gdp_datum_t *datum, // message to print
//     FILE *fp                  // file to print it to
// );
//
void
gdp_datum_print_stdout(
        const gdp_datum_t *datum // message to print
                       )
{
    gdp_datum_print( datum, stdout, 0 );
}


// Return the printable gcl name for the open gcl gclh.
// Working version of the immediately below function that currently
// doesn't work.
//
// The current libgdp seems not to provide access via function to gclh->pname.
// We have to include gdp_priv.h to resolve gclh->pname; need a better
// libgdp interface to pname. TBD
char *
gdp_get_pname_from_gclh( const gdp_gcl_t *gclh )
{
	if (gclh->gob == NULL)
		return "(none)";			// better than a seg fault
    return (char *) (gclh->gob->pname);
}
//
// TBD: this routine doesn't work yet -- fix it or delete it.
// Instead use gdp_get_pname_from_gclh( const gdp_gcl_t *gclh ) just above.
// See comments there on why this function is still TBD here.
//
// Get a printable (base64-encoded) GCL name from an open GCL handle
// Note, we are returning a char * to a static variable; copy out its contents
// quickly :-).
// Combines calls to: gdp/gdp.h
//    const gcl_name_t *
//    gdp_gcl_getname(const gdp_gcl_t *gclh)  
// and
//    char *
//    gdp_printable_name(const gcl_name_t internal, gcl_pname_t external)
//    
char *
gdp_get_printable_name_from_gclh( const gdp_gcl_t *gclh )
{
    char               *rv = NULL;
    // handstands to get types correct for the two calls below
    const gdp_name_t    int_gcl_name;
    const gdp_name_t   *int_gcl_name_p = &int_gcl_name;
    static gdp_pname_t  ext_gcl_name;

    // gcp_gcl_getname() has no check on gclh
    int_gcl_name_p = gdp_gcl_getname( gclh );
    // gcp_gcl_printable_name() has no check on either argument
    // assumes caller allocates ext_gcl_name. rv will point to ext_gcl_name.
    rv = gdp_printable_name( int_gcl_name, ext_gcl_name );

#if DEBUG
    // Yes, atrocious debug output == TBD
    // fprintf( stderr, "gdp_get_printable_name_from_gclh: int_gcl_name_p = \'%s\'\n", int_gcl_name_p );
    fprintf( stderr, "gdp_get_printable_name_from_gclh: int_gcl_name = \'%s\'\n", int_gcl_name );
    fprintf( stderr, "gdp_get_printable_name_from_gclh: ext_gcl_name = \'%s\'\n", ext_gcl_name );
    fprintf( stderr, "gdp_get_printable_name_from_gclh: rv = \'%s\'\n", rv );
    fflush(stderr);
#endif

    return rv;
}


// From ep/ep_time.h

// Variant of ep_time_print(const EP_TIME_SPEC *tv, FILE *fp, bool human)
// that doesn't print a timestamp; rather returns it as a string.
// Wraps ep/ep_time.h ep_time_format()
//
// format a time string to a file
// extern void
// ep_time_print(
//     const EP_TIME_SPEC *tv,
//     FILE               *fp,
//     bool                human
// );
char *
ep_time_as_string( const EP_TIME_SPEC *tv, bool human )
{
    // TBD: watch out, we're using a static to hold our return string contents
    static char tbuf[100]; // TBD: need a safely defined constant/macro for 100
    // Maybe even a separate gdp.h type.

    ep_time_format(tv, tbuf, sizeof tbuf, human);
    return tbuf;
}


// Get a timestamp as a string from a datum
// Note, we are returning a char* to a static variable; copy out its contents
// quickly :-).
// Combines calls to:
//    From gdp/gdp.h
//    void
//    gdp_datum_getts( const gdp_datum_t *datum, EP_TIME_SPEC *ts ) 
// and
//    From ep/ep_time.h
//    char *
//    ep_time_as_string( const EP_TIME_SPEC *tv, bool human )
//    
char *
gdp_datum_getts_as_string( const gdp_datum_t *datum, bool human )
{
    char               *rv = NULL;

    // TBD: watch out, ep_time_as_string is using static to hold its
    //      return string contents
    rv = ep_time_as_string( &datum->ts, human );
#if DEBUG
    fprintf( stderr, "gdp_datum_getts_as_string: rv = \'%s\'\n", rv );
    fflush(stderr);
#endif
    return rv;
}


// ====================================================
// Wrap selected macros from gdp/gdp*.h in functions

// From gdp/gdp_stat.h

// #define GDP_STAT_NAK_NOTFOUND GDP_STAT_NEW(ERROR, _GDP_CCODE_NOTFOUND)

// Returns a EP_STAT with severity field ERROR; other fields see below.
EP_STAT
gdp_stat_nak_notfound()
{
    EP_STAT_32_64 rv;
    rv.as_EP_STAT  = (GDP_STAT_NEW(ERROR, _GDP_CCODE_NOTFOUND));
#if DEBUG
    fprintf( stderr, "gdp_stat_nak_notfound:"
            "  Returning GDP_STAT_NAK_NOTFOUND = %x\n", rv.as_32_64_t);
    fflush(stderr);
#endif
    return (rv.as_EP_STAT);
}


// ====================================================
// Wrap selected macros from ep/ep*.h in functions

// From ep/ep_statcodes.h

// generic status codes
// #define EP_STAT_OK            EP_STAT_NEW(OK, 0, 0, 0)

// Returns a EP_STAT with severity field OK; other fields are 0.
EP_STAT
ep_stat_ok()
{
    EP_STAT_32_64 rv;
    rv.as_EP_STAT = (EP_STAT_NEW(OK, 0, 0, 0));
#if DEBUG
    fprintf( stderr, "ep_stat_ok:  Returning EP_STAT_OK = %x\n", rv.as_32_64_t );
    fflush(stderr);
#endif
    return (rv.as_EP_STAT);
}


// From ep/ep_statcodes.h

// common shared errors
// #define EP_STAT_END_OF_FILE   _EP_STAT_INTERNAL(WARN, EP_STAT_MOD_GENERIC, 3)

// Returns a EP_STAT with severity field WARN; other fields see below.
EP_STAT
ep_stat_end_of_file()
{
    EP_STAT_32_64 rv;
    rv.as_EP_STAT = ( _EP_STAT_INTERNAL(WARN, EP_STAT_MOD_GENERIC, 3) );
#if DEBUG
    fprintf( stderr, "ep_stat_end_of_file:"
            "  Returning EP_STAT_END_OF_FILE = %x\n", rv.as_32_64_t );
    fflush(stderr);
#endif
    return (rv.as_EP_STAT);
}


// From ep/ep_stat.h

// predicates to query the status severity
// #define EP_STAT_ISOK(c)       (EP_STAT_SEVERITY(c) < EP_STAT_SEV_WARN)

// Return true iff the status severity field inside ep_stat is less
// than EP_STAT_SEV_WARN.
int /* Boolean */
ep_stat_isok(EP_STAT ep_stat) {
    int rv = EP_STAT_ISOK(ep_stat);
#if DEBUG
    fprintf( stderr, "ep_stat_isok: ep_stat = %x,",
            ((EP_STAT_32_64) ep_stat).as_32_64_t );
    fprintf( stderr, "  Returning EP_STAT_ISOK() = %x\n", rv );
    fflush(stderr);
#endif
    return rv;
}


// From ep/ep_stat.h

// compare two status codes for equality
// #define EP_STAT_IS_SAME(a, b)   ((a).code == (b).code)

// Return true iff the status codes are the same
int /* Boolean */
ep_stat_is_same(EP_STAT a, EP_STAT b) {
    int rv = EP_STAT_IS_SAME(a, b);
#if DEBUG
    fprintf( stderr, "ep_stat_is_same: a = %x, b = %x",
            (((EP_STAT_32_64) a).as_32_64_t ),
            (((EP_STAT_32_64) b).as_32_64_t )
             );
    fprintf( stderr, "  Returning EP_STAT_IS_SAME() = %x\n", rv );
    fflush(stderr);
#endif
    return rv;
}

