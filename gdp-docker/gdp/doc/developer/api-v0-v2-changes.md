% Changes Between GDP API v0 and API v2

This document briefly describes the differences between Version 0
of the GDP API and Version 2 (introduced around June 2018).  For
more details of the current API, see `doc/gdp-programmatic-api.html`.
If you are not familiar with the old API, please do not read this
document; go directly to the current documentation.

This only describes changes to the C Programmatic API, but the
concepts should be relevant across all language bindings.

# Overview

The API has been updated to be a better fit to object-oriented
paradigms.  For example, function names beginning with `gdp_gin_`
operate on objects of type `gdp_gin_t` (with a few exceptions
such as `gdp_gin_new`) and will take a pointer to a `gdp_gin_t`
as the first ("self") argument.  In most cases, everything that
has a type (identified by a name ending `_t`) is probably a class.

The asynchronous APIs have become the primary focus versus the
synchronous APIs.  In particular, the asynchronous versions can
handle sets of records in a single call, which improves
performance and makes handling of holes and branches in a log
more elegant.  As a result, the "multiread" routines have been
merged with the "async" routines.

Applications now manipulate a "GDP Instance" (GIN) instead of
a "GDP Channel-Log" (GCL).  This has semantic implications with
asynchronous calls.  This has lead to re-naming above and beyond
the other semantic changes.  The name "GCL" has been deprecated.

These changes also coincide with a change in the on-the-wire network
protocol, which has a few subtle but important implications.  Notably,
the size of an individual PDU (Protocol Data Unit) has been reduced
from approximately 4GB to approximately 65kB in order to avoid large
protocol elements flooding the network, thus creating convoys.  This
in turn limits the size of the maximum size of any log entry (a.k.a.
"record" or "datum").


# Name Changes

Names marked with \* also have parameter changes.

| OLD				| NEW					|
|-------------------------------|---------------------------------------|
| `gdp_gcl_t`			| `gdp_gin_t`				|
| `gdp_gcl_create`		| `gdp_gin_create`			|
| `gdp_gcl_open`		| `gdp_gin_open`			|
| `gdp_gcl_open_info_t`		| `gdp_open_info_t`			|
| `gdp_gcl_open_info_new`	| `gdp_open_info_new`			|
| `gdp_gcl_open_info_free`	| `gdp_open_info_free`			|
| `gdp_gcl_open_info_set_signing_key` | `gdp_open_info_set_signing_key`	|
| `gdp_gcl_open_info_set_signkey_cb`  | `gdp_open_info_set_signkey_cb`	|
| `gdp_gcl_open_info_set_caching`     | `gdp_open_info_set_caching`	|
| `gdp_gcl_open_info_free`	| `gdp_open_info_free`			|
| `gdp_gcl_close`		| `gdp_gin_close`			|
| `gdp_gcl_append`		| `gdp_gin_append`\*			|
| `gdp_gcl_append_async`	| `gdp_gin_append_async`\*		|
| `gdp_gcl_read`		| `gdp_gin_read_by_recno`		|
| `gdp_gcl_read_async`		| `gdp_gin_read_by_recno_async`\*	|
| `gdp_gcl_read_ts`		| `gdp_gin_read_by_ts`			|
| _new_				| `gdp_gin_read_by_ts_async`\*		|
| _new_				| `gdp_gin_read_by_hash`		|
| _new_				| `gdp_gin_read_by_hash_async`\*	|
| `gdp_gcl_subscribe`		| `gdp_gin_subscribe_by_recno`\*	|
| `gdp_gcl_subscribe_ts`	| `gdp_gin_subscribe_by_ts`\*		|
| `gdp_gcl_unsubscribe`		| `gdp_gin_unsubscribe`\*		|
| `gdp_gcl_multiread`		| `gdp_gin_read_by_recno_async`		|
| `gdp_gcl_multiread_ts`	| `gdp_gin_read_by_ts_async`		|
| `gdp_gcl_getmetadata`		| `gdp_gin_getmetadata`			|
| `gdp_gcl_newsegment`		| _deleted_				|
| `gdp_gcl_set_append_filter`	| `gdp_gin_set_append_filter`		|
| `gdp_gcl_set_read_filter`	| `gdp_gin_set_read_filter`		|
| `gdp_gcl_getname`		| `gdp_gin_getname`			|
| `gdp_gcl_getnrecs`		| `gdp_gin_getnrecs`			|
| `gdp_gcl_print`		| `gdp_gin_print`			|
|||
| `gdp_gclmd_t`			| `gdp_md_t`				|
| `gdp_gclmd_id_t`		| `gdp_md_id_t`				|
| `gdp_gclmd_new`		| `gdp_md_new`				|
| `gdp_gclmd_free`		| `gdp_md_free`				|
| `gdp_gclmd_add`		| `gdp_md_add`				|
| `gdp_gclmd_get`		| `gdp_md_get`				|
| `gdp_gclmd_find`		| `gdp_md_find`				|
| `gdp_gclmd_print`		| `gdp_md_print`\*			|
|||
| `GDP_EVENT_EOS`		| `GDP_EVENT_DONE`			|
|||
| _new_				| `gdp_hash_t`				|
| _new_				| `gdp_hash_new`			|
| _new_				| `gdp_hash_free`			|
| _new_				| `gdp_hash_reset`			|
| _new_				| `gdp_hash_set`			|
| _new_				| `gdp_hash_getlength`			|
| _new_				| `gdp_hash_getptr`			|
|||
| _new_				| `gdp_sig_t`				|
| _new_				| `gdp_sig_new`				|
| _new_				| `gdp_sig_reset`			|
| _new_				| `gdp_sig_free`			|
| _new_				| `gdp_sig_set`				|
| _new_				| `gdp_sig_copy`			|
| _new_				| `gdp_sig_dup`				|
| _new_				| `gdp_sig_getlength`			|
| _new_				| `gdp_sig_getptr`			|

# Details

## Appends, Hashes, and Signatures

The long(ish) term intent is that all records (datums) will be
cryptographically linked in an Authenticated Data Structure.
We are discussing many ways of doing this, but all of them involve
hash chains of records.  As a result, the "append" interfaces now
take a `prevhash` parameter which is a hash of the previously
written record.

When writing consecutive records, the GDP library can maintain
the previous hash and insert it automatically if the `prevhash`
parameter is `NULL`.  However, when a writer initializes, it must
determine the hash of the previous record.  Ideally the writer
would not trust the underlying infrastructure, and would instead
save the hash of the previous record written (an exception being
made for the first record in the log).  This should be done by
saving the previous hash on local stable storage.  It's possible
that the GDP library could manage this state, but that is not yet
implemented.  At this point the details of how this should work
are unclear, so this feature is only partly implemented.

Similarly, it is important that readers be able to validate
signatures for themselves.  This is the rationale behind elevating
them (`gdp_sig_t`) to first-class citizens.

Hashes will become more important as readers start checking the
provenance of data returned by servers.  The details of that are
still in the research arena and are out of scope of this document.

## Subscriptions and Asynchronous Reads

Calling `gdp_event_next` with a given GIN will only return events
from asynchronous reads and subscriptions listed on that GIN.
Previously, if a log was opened twice (and hence had two GCL
handles) the data might be returned on a different instance.
For example, consider the (old) code:

``` c
gdp_name_t gcl_name;
gdp_gcl_t *gcl1, *gcl2;
extern gdp_event_cbfunc_t cb1, cb2;
EP_STAT estat;

// open the same log twice
estat = gdp_gcl_open(gcl_name, GDP_MODE_RO, NULL, &gcl1);
estat = gdp_gcl_open(gcl_name, GDP_MODE_RO, NULL, &gcl2);

// subscribe to the end, and read from the beginning
estat = gdp_gcl_subscribe(gcl1, 0, 20, NULL, &cb1, NULL);
estat = gdp_gcl_multiread(gcl2, 1, 100, &cb2, NULL);
```

would cause `cb1` and `cb2` to be called somewhat randomly
with results of the multiread from the beginning of the log and
the results of the subscribe from the end of the log.

The new code would be:

``` c
gdp_name_t log_name;
gdp_gin_t *gin1, *gin2;
extern gdp_event_cbfunc_t cb1, cb2;
EP_STAT estat;

// open the same log twice
estat = gdp_gin_open(log_name, GDP_MODE_RO, NULL, &gin1);
estat = gdp_gin_open(log_name, GDP_MODE_RO, NULL, &gin2);

// subscribe to the end, and read from the beginning
estat = gdp_gin_subscribe_by_recno(gin1, 0, 20, NULL, &cb1, NULL);
estat = gdp_gin_read_by_recno_async(gin2, 1, 100, &cb2, NULL);
```

would return the results from the subscription exclusively
to `cb1` and the results of the read exclusively to `cb2`,
which was probably what was intended.

Similarly, `gdp_gin_unsubscribe` only deletes subscriptions
that were created on a specific GIN; previously it was
somewhat random.

## Appending Multiple Records

The old `gdp_gcl_append_async` call previously only added one
datum to a log.  The new `gdp_gin_append_async` call can
append multiple datums in one call.  Besides sending fewer
network commands, this allows the individual datums to be linked
together in a hash chain with only the last datum digitally
signed, which is much more efficient.

Beware however that all the datums must fit within a single
network PDU (Protocol Data Unit), and the maximum size has been
reduced to approximately 65k to avoid network congestion.

# Compatibility

At some point, if there is demand, we may add a `<gdp_compat_v0.h>`
that will to the extent possible make it feasible to run programs
coded against the old API.  This will only deal with the syntactic
issues however.
