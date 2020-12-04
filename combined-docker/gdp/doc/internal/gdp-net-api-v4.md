% What I Want to See in a Network API
  for the GDP
% Eric Allman
% 2017-12-13

***This is a proposal, not a specification***

This defines an API, not "bits on the wire" or, to the extent
possible, specifics of implementation.  The goal is that subject
to a few constraints, everything "above the line" can be insulated
from the actual network implementation, which must define the
packet format in whatever way it sees fit.  Conversely, everything
"below the line" will not be in any way affected by changes in the
higher level protocol.  Obviously both of those specifications will
be required, but this document isn't the place.

SEE ALSO: Nitesh's documents.

> [[Note: I'm doing a "proof of concept" implementation, essentially
just adapting the current version, in order to test the API for
completeness and simplicity.  In that process I'm still finding
new problems, so this **WILL** change.]]

> [[NOTE: "PDU", "blob", and "payload" are used somewhat interchangeably,
with a bit of "message" thrown in for good measure.  This is a bug,
not a feature.  However, they differ from a "packet", which is a
property of the underlying physical medium.]]


# Key Points

This is essentially a message-based L4/L5 (Transport/Session
Layer) model.  The L5 part is about advertisements and routing.
Normally routing would be L3 (Network Layer), but our model
seems to be somewhat inverted in that the routing commands
live on top of the reliable transmission layer.  This is part
of the legacy of the "overlay network" model.

For the purposes of this document, "I", "me", "my", "up", and
"application direction" refer to the user of this API.
"You", "your", "down", and "network direction" refer to the
implementer of the API.  "Payload" is an opaque blob for you;
the term is approximately equal to "PDU".

> Using OSI language, a "payload" would be called a "Service
Data Unit" or SDU.  You add a header to that, and then it
becomes the PDU for the next layer down.  This document does
not discuss the internal structure of the payload.

GDP clients and servers ("my side" of the API) see:

* A message-based interface (i.e., PDUs are delimited by the
  network layer).
* Reliable data transmission:
    + Fragmentation, flow control, etc. already handled.
    + Individual PDUs delivered reliably, that is, fragments of
      PDUs will be delivered in order, fragments will not be
      duplicated, and an error will be delivered if a fragment
      is lost.
* Different PDUs may be delivered out of order.
* PDU sizes are not inherently limited by underlying MTUs.

The network ("your side" of the API) sees (that is, I will give
you):

* Source and destination GDPnames (256-bit).
* Advertisements of known GDPnames, authenticated using a
  certificate-based scheme, still not fully defined.
* Probably some hints vis-a-vis client expectations such as
  Quality of Service.  These remain _for further study_.
* A `libevent` event base to be used for registering I/O
  events.  I may register my own events in the same event
  base.
* Callbacks for functions that require higher level interaction,
  such as advertisements.
* _Other information to be determined._
* I will run the event loop base as appropriate.

Your side is responsible for:

* Advertising of known names, including refreshing leases.
* Routing.
* Retransmissions, etc.
* Reestablishing dropped connections (e.g., in presence of
  router failure).
* Fragmentation/Reassembly.
* In order delivery (i.e., all pieces of the PDU delivered in order).
* Compression (notably header compression).
* On-the-wire crypto (TLS or DTLS as appropriate).
* DoS mitigation.  Attack traffic should be stopped as soon as
  possible.
* Invoking callbacks when I/O events occur.

"Your side" of the API can live partially in the client library,
but some amount of it might live elsewhere, either in the
switch/forwarder/router layer or in a separate service.  I assume
that this layer will not rely on threads in client processes to
make it possible to run in low-end (non-MMU) processors.
_Note: this is a change from the V3 design, which assumes a
dedicated thread for I/O.  This is intended to support Kubi's
dream of a semantically limited GDP in a single-threaded
environment such as an Arduino._  However, some GDP applications
may still use dedicated threads for I/O or other functions.

Issues that we should consider:

* All internal structure of messages (including signatures, etc)
  are hidden from the forwarding/routing layer.  Is there anything
  else (other than source and destination identifiers) that needs
  to be exposed to the lower level (e.g., QoS requests)?
    + Nitesh says: ability to address individual replicas.
    + Nitesh says: ability to send to all replicas.
    + Eric says: anything needed for DoS mitigation?  Maybe
      an HMAC key?

* This interface allows streaming receipt of data (within a single
  record) but not streaming transmission.  How serious is this?
  Note: it's impossible to write a header that includes the payload
  length until the size has been determined, which often means that
  the entire payload must be assembled in advance.

* This should be easy to implement (from the network perspective)
  for the existing TCP-based, no fragmentation model, but is it
  reasonably sane for other I/O models?

* Some way of doing multicast, notably for subscriptions.

Design/Implementation Notes:

* This interface uses `libevent` (see <http://libevent.org>).
  There is really no way around my picking the library without
  inverting the control flow, which would leave us forced to
  use threads.  As a result, `libevent` semantics are built
  into this proposed API.

Documentation Notes:

* This version is loosey-goosey with status codes.  If we decide
  that this is a good direction I will provide more detail.


# API

The API is intended to map easily into an object-oriented paradigm
with the first parameter to instance methods being `self`.  Class
methods generally return a status code; if they are allocating a
new object it will be returned through the last parameter.

The parenthetical comments in the titles are intended to provide
a model of how this would map into an OO environment.


## Data Types and Conventions

The following data structures are opaque to "my side" of the API,
but can be defined arbitrarily by "your side":

* `gdp_chan_t` contains the state of the channel itself.  It is
  opaque to "my side" of the API.
* `gdp_adcert_t` is whatever information is needed to advertise
  a GDPname.  This is _for further study_.
* `gdp_adchallenge_t` is data associated with a challenge/response
  interaction when doing advertising.
* `gdp_target_t` is intended for specifying clues as to where to
  deliver a message, for example, any replica, all replicas,
  or a specific replica.  It is undefined as yet.

The following data structures are opaque to "your side" of the API
(i.e., you can never dereference them), but I can define to suit
my needs:

* `gdp_chan_x_t` contains "My" private data.  This evaluates to
  `struct gdp_chan_x`, which I must define if I want to dereference
  that structure.  It is normally referred to as `cdata` in the
  descriptions below.
* `gdp_advert_x_t` is my data structure used to pass information
  vis-a-vis advertising.  It is normally referred to as `adata`
  in the descriptions below.

These data structures are opaque to both of us; their interfaces
are described in the "GDP Programmatic API" document:

* `gdp_buf_t` implements dynamically allocated, variable sized
  buffers (already exists; based on `libevent` `evbuffer`s).
* `gdp_name_t` is the 256-bit version of a GDP name.


## Initialization


### \_gdp\_chan\_init (class method)

I promise to call this routine on startup:

~~~
	EP_STAT _gdp_chan_init(
			struct event_base *evbase,
			gdp_chan_init_t *options);
~~~

The `evbase` parameter is a `libevent` event base that I will
create and pass to you.
The `options` parameter is for future use.  Until then, I'll
always pass it as `NULL`.



## Channel Management

A Channel is a somewhat arbitrary class used to store whatever the
network ("your") side needs.  I promise to create at least one
channel when I start up.

### \_gdp\_chan\_open (constructor)

~~~
	typedef EP_STAT gdp_chan_recv_cb_t(
			gdp_chan_t *chan,
			gdp_name_t *src,
			gdp_name_t *dst,
			gdp_buf_t *payload);
	typedef EP_STAT gdp_chan_send_cb_t(
			gdp_chan_t *chan,
			gdp_buf_t *payload);
	typedef EP_STAT gdp_chan_ioevent_cb_t(
			gdp_chan_t *chan,
			int ioevent_flags);
	typedef EP_STAT gdp_chan_advert_func_t(
			gdp_chan_t *chan,
			int action);

	EP_STAT _gdp_chan_open(
			const char *addrspec,
			gdp_chan_qos_t *qos,
			gdp_chan_recv_cb_t *chan_recv_cb,
			gdp_chan_send_cb_t *chan_send_cb,
			gdp_chan_ioevent_cb_t *chan_ioevent_cb,
			gdp_chan_advert_func_t *advert_func,
			gdp_chan_x_t *cdata,
			gdp_chan_t **chan);
~~~

Creates a channel to a GDP switch located at `addrspec` and stores
the result in `*chan`.  The format is a semicolon-delimited list of
`hostname[:port]` entries.  The entries in the list should be tried
in order.  If `addrspec` is NULL, Zeroconf should be tried if enabled,
and if that fails the `swarm.gdp.routers` runtime parameter is used.
If that is not set, `_gdp_chan_open` may try a default.  If it
cannot connect, it should return a status related to the reason
(i.e., one based on a Posix `errno`) or `GDP_STAT_CHAN_NOT_CONNECTED`.

The `qos` parameter is intended to hold additional open parameters
(e.g., QoS requirements), but for now I promise to pass it as `NULL`.
If it non-NULL, you should return `GDP_STAT_NOT_IMPLEMENTED`.

The callbacks are described below.

The `cdata` parameter is saved and is available to callbacks on this
channel.  It is opaque on the network side.

**`chan_recv_cb`**:  When a new PDU is ready to read on the
associated channel, call `chan_recv_cb`.  Details are described under
"Receiving Messages" below.

**`chan_send_cb`**:  The intent is that this will be called when
the network is able to accept more data.  It is not used at this
time.  For now, if it is non-NULL `_gdp_chan_open` should return
`GDP_STAT_NOT_IMPLEMENTED`.  The parameters to this function are
subject to change.

**`chan_ioevent_cb`**: When a channel is closed by the other end of
the connection, or on I/O error, `gdp_ioevent_cb` is called.  The
`ioevent_flags` parameter is from the following set:

| Flag			| Meaning					|
|-----------------------|-----------------------------------------------|
| `GDP_IOEVENT_CONNECT`	| Connection established			|
| `GDP_IOEVENT_EOF`	| End of file on channel (i.e., it was closed)	|
| `GDP_IOEVENT_ERROR`	| Error occurred on channel			|

Note that this is a bitmap; multiple flags may be set on a single call.

**`chan_advert_func`**: Invoked when advertising or withdrawal needs to
take place.  Advertising is required when the channel is connected or
reconnected and in periodic lease updates.
Withdrawal takes place when the client explicitly requests it or
when the client is shutting down.  The `action` parameter specifies
whether this is for advertisement or withdrawal.

> [[Nitesh brings up the question of `dst` filtering.  It isn't clear
which side of the interface this belongs on.  Probably on your side
since DoS mitigation should be as low level as possible.]]

### \_gdp\_chan\_close (destructor)

~~~
	EP_STAT _gdp_chan_close(
			gdp_chan_t *chan);
~~~

Deallocate `chan`.  All resources are freed.  I promise I will not
attempt to use `chan` after it is freed.

### \_gdp\_chan\_get\_cdata (chan::get\_cdata)

~~~
	gdp_chan_x_t *_gdp_chan_get_cdata(
			gdp_chan_t *chan);
~~~

Returns the `cdata` associated with `chan`.


## Advertising and Certificates

> [[Who is responsible for certificate management and advertisements?
I guess that's likely to be me.  Drat.]]

Note that the naive implementation of this interface would cause at
least one round trip for each known name.  This will be particularly
expensive for log servers with large numbers of logs.  One possible
solution is to allow batching of advertisements, so the caller does
something like:

~~~
	_gdp_chan_advertise(chan, gnameA, ...);
	...
	_gdp_chan_advertise(chan, gnameZ, ...);
	_gdp_chan_advert_commit(chan);
~~~

This would cause an actual send to the routing layer.  This would
mean that challenge callbacks would not be synchronous with the
`_gdp_chan_advertise` calls.

### \_gdp\_chan\_advertise (chan::advertise)

_This interface is still under development._

~~~
	// advertising challenge/response callback function type
	typedef EP_STAT (*gdp_chan_advert_cr_t)(
			gdp_chan_t *chan,
			gdp_name_t gname,
			int action,
			gdp_adchallenge_t *acdata,
			gdp_advert_x_t *adata);

	// advertisement method
	EP_STAT _gdp_chan_advertise(
			gdp_chan_t *chan,
			gdp_name_t gname,
			gdp_chan_adcert_t *adcert,
			gdp_advert_cr_t *challenge_cb,
			gdp_advert_x_t *adata);
~~~

Advertises the name `gname` on the given `chan`.  If a certificate
needs to be presented, it should be passed as `adcert`.  If the
underlying layer needs further interaction (e.g., for challenge/response)
it should call `challenge_cb`.  The `adata` is passed through untouched.

If the routing subsystem challenges `adcert` the `challenge_cb`
function will be invoked with the `chan`, the `gname` being
challenged, an `action` **to be determined**, any challenge data
issued by the router side as `acdata`, and the `adata` field directly
from `_gdp_chan_advertise`.

> [[What is `adcert` exactly?  Where does it come from?]]

> [[We still need to explain how `acdata` is used.]]

### \_gdp\_chan\_withdraw (chan::withdraw)

~~~
	EP_STAT _gdp_chan_withdraw(
			gdp_chan_t *chan,
			gdp_name_t gname,
			gdp_advert_x_t *adata);
~~~

Withdraw a previous advertisement, for example, if a log is removed
from a given server.

> [[Question: should `_gdp_chan_advertise` return a data structure
that can be passed to `_gdp_chan_withdraw` or is the gname enough?]]

> [[Question: is there any point in sending `adata` here?]]


## Sending Messages

### \_gdp\_chan\_send (chan::send)

~~~
	EP_STAT _gdp_chan_send(
			gdp_chan_t *chan,
			gdp_target_t *target,
			gdp_name_t src,
			gdp_name_t dst,
			gdp_buf_t *payload);
~~~

Sends the entire contents of `payload` to the indicated `dst` over
`chan`.  The source address is specified by `src`.

> [[Does this clear `payload` or leave it unchanged?  Should
probably clear it as the data is sent and acknowledged.]]

The `target` give clues as to exactly where to deliver the
message.  For example, it might be any replica of a given log,
all replicas of a given log (e.g., for quorum read), or a specific
replica.  It can also be used to indicate the equivalent of IPv4
"type of service" (precedence, reliability, etc).  How it is specified
is _for further study_.  For now, I promise to pass in NULL; if it
is not NULL, `GDP_STAT_NOT_IMPLEMENTED` should be returned.

> [[Issue: There are issues regarding allowing an arbitrary `src` that
need to be explored.  You should never be permitted to send from
an address you aren't authorized to speak for, but the ultimate
responsibility for avoiding problems falls to the receiver.]]

> [[Implementation Note: in the short run this may return
`GDP_STAT_PDU_TOO_LONG` if the size of `payload` exceeds an
implementation-defined limit.  This should be as large as possible
since it limits the size of any single record stored in the GDP.]]


### \_gdp\_chan\_multicast (chan::multicast)

> [[Note: this is a placeholder.]]

~~~
	EP_STAT _gdp_chan_multicast(
			gdp_chan_t *chan,
			gdp_name_t src,
			gdp_mcaddr_t multicast_addr,
			gdp_buf_t *payload);
~~~

Sends `payload` to multiple destinations as indicated by
`multicast_addr`.  It isn't clear what that is.

This is primarily intended for delivering subscription data.

This will certainly need support to set up a multicast channel,
probably with "new", "join", "leave", and "free" style interface.


## Receiving Messages

Messages are delivered using the `gdp_chan_recv_cb` parameter
to `_gdp_chan_open`.  When a complete message is ready for delivery,
this is function is called.


## Utilities


### \_gdp\_chan\_lock, \_gdp\_chan\_unlock (chan::lock, chan::unlock)

~~~
	void _gdp_chan_lock(
			gdp_chan_t *chan);

	void _gdp_chan_unlock(
			gdp_chan_t *chan);
~~~

Lock or unlock a `chan`.  This is a mutex lock.

> [[Can this be done implicitly?  What in particular is it for?]]


# Status Codes

To be completed.

* `GDP_STAT_KEEP_READING`
* `GDP_STAT_PDU_VERSION_MISMATCH`
* `GDP_STAT_PDU_CORRUPT`
* `GDP_STAT_NOTFOUND`
* `GDP_STAT_PDU_WRITE_FAIL`
* `GDP_STAT_NOT_IMPLEMENTED`
* `GDP_STAT_CHAN_NOT_CONNECTED`


# Implementation Notes

These are primarily notes to myself, so don't be concerned if the
are hard to understand.

__Example startup__ (multi-threaded, my code).  In pseudo-code:

~~~
	_gdp_chan_init
		registers I/O events
	register timeout events
	spawn(event_loop)
	_gdp_chan_open
		(causes advertisements to be sent)
	return to application
~~~

The event loop must be running before `_gdp_chan_open` so advertisement
I/O can be handled as events.

For __single threaded apps__, all condition variables must be replaced.
Unlike mutexes, they can't just turn into no-ops.  In some cases
(notably `_gdp_invoke` and `gdp_event_next`) they turn into single
calls to the event loop (with some trickiness around timeouts).
In other cases (e.g., `chan->cond`) they should disappear
entirely.
