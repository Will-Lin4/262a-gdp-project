% GDP Protocol Version 4 Proposal
% Eric Allman
% 2018-01-30

Everything here is for discussion.

Principles
==========

These are guiding principles for this proposal.

* What we have previously called the routing layer is broken up
  into three sub-layers: a "forwarding" (a.k.a. "switch") layer that
  shovels PDUs from a source to a destination as quickly as
  possible, a "routing" layer that deals with advertisements,
  DHT lookup, etc., and an "ingress/egress layer" that deals with
  communications between GDP Principals (notably clients and log
  servers) and the rest of the routing/switching infrastructure.

* This description defines only the PDU used for the ingress/egress
  layer.  Other details such as Time to Live (TTL), fragmentation,
  and sequencing are not discussed.

* "End to end" PDU contents should be in some language-agnostic format
  (such as Protocol Buffers or Cap'n Proto).

* End-to-end PDU contents should be opaque to the switching layer; in
  particular, it should be possible for it to be encrypted.

* Implementation of some operations (e.g., delivery of subscribed
  data) should play well with multicast.

* Size matters.  PDU headers need to be constrained.  In particular,
  the addresses (totaling 64 bytes when expanded) are just too large;
  they need to be encodable into a "FlowID", which can be thought of
  as a cache of common header information.

* Implementation matters.  Notably, forwarders should be able to view
  most of the PDU as opaque (not just the data), and should be optimized
  for speed.  In particular, forwarder-based header information should
  _not_ be encoded in a format that requires that a complex data
  structure be unserialized and reserialized.


Definitions
===========

* FlowID: a mechanism for encoding source and destination information
  into a smaller size to optimize bandwidth utilization.

* Forwarding: the process of transmitting a PDU from a source to
  a destination, with the assumption that the endpoints are already
  known.  If they are unknown, the Routing layer must get involved.

* Octet: a single eight-bit data element.  This is the network term
  for a "byte" (but since some architectures have non-eight-bit
  bytes, "octet" is used instead).

* Packet: A data block that can be transmitted intact over an
  underlying network protocol.  Depending on the context, this might
  be limited by physical constraints (e.g., the maximum size of an
  ethernet frame) or by logical constraints (e.g., the maximum
  chunk size in SCTP).

* PDU (Protocol Data Unit): A block of data representing a single
  actionable unit (e.g., command or response).  In some cases
  multiple PDUs might be encoded into a larger "Transport PDU".

* Routing: determining the location of a destination.


Protocol Overview
=================

Every PDU consists of two parts.  The first portion is the Protocol
Header.  This is binary encoded, and intended to contain all the
information that the Switching Layer needs to route packets
(assuming that layer has the necessary routing information already
cached).  It is designed to be small and fast.

The second portion is the Payload.  This is intended to be encoded
in some industry-standard, platform agnostic encoding.  Popular
examples include ProtoBuf and Cap'n Proto.  The contents of the
Payload is message dependent.

This alternative assumes that we are running over some Layer 4
protocol that gives us reliable, ordered transmission of arbitrarily
sized PDUs (for example, TCP).  That would make this a Layer 5
protocol.


### Protocol Header

Details are shown after the table.  The **Alternative** field
relates to the flags indicated in the **Flags** field.  [[_Note:
these have been changed, so the **Alternative** column is not
accurate at this time._]]


| Offset 	| Len 	| Alternative 	| Detail								|
|-------:	|----:	|-------------	|-------------------					|
|      0	|   1	|				| Magic/Version [1]						|
|	   1	|   1	|				| Header Length	/ 4 [2]					|
|	   2	|   1	|				| Flags/Control [3]						|
|      3	|   1	|				| Reserved (MBZ) [4]					|
|      4	|   2	|				| Payload Length [5]					|
|      6 	|   4?  | IFLOWID      	| Initiator FlowID						|
|      - 	|   4?  | RFLOWID      	| Return FlowID							|
|      - 	|  32  	| GDPADDR     	| Destination Addr						|
|      - 	|  32  	| GDPADDR     	| Source Addr							|
|      -	|   V	|				| Options								|
|	   -	| 0-3	|				| padding								|


[1]	Magic and Version identify this PDU.  Must be 4.

[2] Size of header in units of 32 bits.  This starts at offset
	zero and includes the options.  This constrains the header to at
	most 1020 octets.  It must be at least 2 [or more for addresses
	and flow ids?].

[3] Flags/Control is a multipurpose field.  The low-order three
	bits define the address fields.  If zero, there are two 32
	octet (256 bit) fields designating the destination and source
	addresses.  Other values are to support address compression
	and are reserved.  If the high order (0x80) bit is set, the
	next four bits are a command for Principal-Router communication,
	where a Principal can be either a GDP client or a GDP log
	server.  Such values are defined below.  If the high order
	bit is zero the remaining bits must be zero as well.  This is
	reserved for future expansion.

[4] The reserved field is partly to improve memory alignment but
	more importantly to allow for future expansion.  It is
	immediately before Payload Length so that it could be used
	to allow larger payloads, should they become necessary.
	It must be zero when the PDU is generated, and ignored when
	the PDU is read.

[5] Size of Payload in units of 8 bits.  It is represented in network
	byte order (big endian).  This constrains the maximum size of a
	PDU payload to 2 ^ 16 - 1 = 65,535 octets.

The total size of the PDU is the sum of the Header Length and the
Payload Length.  Note that since the header length must be a multiple
of four there will always be two octets of options which will
often be zero.  If this is a problem we can reduce the header size
to be in units of 16 bits, at the cost of allowing less space for
options.  However, since IPv4 only allows 40 octets for options,
this is unlikely to be a big issue.

The source and destination address can be specified either explicitly
or by encoding into a FlowID.  Mechanisms for managing FlowIDs are
[[_being explored by Nitesh_]].

[[_Note to Nitesh: I'm using "Initiator FlowID" instead of just
"FlowID" to avoid confusion between the specific and the generic.
For example, consider the statement "the FlowID and the Return
FlowID are both FlowIDs."_]]


### Flag Bits

None defined at this time &mdash; must be zero when a PDU is
created.  PDU interpreters should ignore these bits.


### Options

Options are used to convey additional information to the switching
layer, e.g., Quality of Service.  These are for future use.  Note
that options are included in the header size, so routers that do
not support options can skip this part without additional processing.

Each Option starts with a single octet of option id.  The bottom
three bits of the option id also contains the length of the option
value as a power of two (e.g., a value of zero means zero bytes, one
one means one octet, two means two octets, four means eight octets,
etc.).  If the bottom three bits are 0x7 then the length is taken
from the octet immediately following the id without scaling (that
is, a value of five means five octets, not 2^5 = 32).
If the length is encoded in in the option id octet, that length is
part of the option id.  For example, option 0x10 and 0x13 are
different options, the former of length zero and the latter of length
four.  In comparison, the size of option 0x87 is contained in the
following octet, and the size is not part of the option id, so
0x87 0x00 and 0x87 0x04 are the same option, with values of length
zero and four respectively.

Unrecognized options must be ignored (but passed on).
[[_Perhaps there should be some encoding that indicates that an
option is essential, in the sense of the CoAP [RFC7252] distinction
between "Critical" and "Elective" options._]]

Options:

| Value		| Name			| Detail					|
|------:	|-----:			| -----------------			|
| 0x00		| OPT\_END		| End of Option List		|

[[_Need to define other options._]]

Note that some Options may be implied by a FlowID, in the same way
that a 256-bit address is implied by a FlowID.

[[_It might be useful to have a compact (single octet) encoding
for options with length 32, since that is the length of a GDP
address._]]


Payload Encoding
================

[[_Move this into another document, or see the code._]]


Client-Router Protocol
======================

Sometimes a GDP principal (client or log server) needs to communicate
with the routing layer, for example, for advertising known names.
This is done using the Flags/Control field.  If the high order (0x80)
bit is set in that octet, this PDU is either directed to or sent
from the routing layer.  This approximates ICMP in IP networks.

Note that the bottom three bits of this octet are not part of the
Client-Router protocol, as they are used to represent the address
format.

[[_This is temporary, just so Rick and Eric can get something, anything,
working.  We know advertising has to be done using certs._]]

The following table assumes that the Flags/Control field is masked
with 0xf8 (i.e., the bottom three bits are ignored):

| Value		| Name			| Detail							|
|------:	|-----:			| -----------------					|
| 0x80		| FORWARD		| Forward PDU [1]					|
| 0x90		| ADVERTISE		| Advertise names [2]				|
| 0x98		| WITHDRAW		| Withdraw names [2]				|
| 0xF0		| NOROUTE		| Cannot find route [3]				|

[1] FORWARD passes the PDU to another entity (which could be a
    router or a server), strips off the header, and processes
	the payload as though it were a PDU.  This can be used for
	source routing, particularly during some operations
	associated with replication.  It is the equivalent of the
	IP-in-IP protocol (4) in IPv4.

[2]	ADVERTISE asserts to the routing layer that the source is
	willing to respond on behalf of the list of 256-bit names
	listed in the payload.  WITHDRAW removes that assertion.
	[[_These need to be replaced with a challenge-response
	certificate exchange._]]

[3]	Sent from the routing layer to a GDP Principal when that
	Principal has sent to an address that cannot be found.
	Approximately equivalent to an ICMP "unreachable" code.
	The source address must be the unroutable name.


Things to Address
=================

* Should multiple commands be permitted in one PDU?  If so, the
  Payload and Trailer information needs to be in some sort of
  array.  Everything in these commands MUST have the same
  source and destination addresses, since this is specific to the
  routing/forwarding layer (i.e, forwarding of a partial PDU is not
  permitted).

* Because the header length is specified as the number of 32 bit
  words, and header with no options ends on a 16-bit boundary, most
  headers will probably have two bytes of padding.  This seems
  wasteful.

* Nitesh doesn't include WITHDRAW in his prototype, using timeouts
  instead.  Do we need it?

<!-- vim: set ai sw=4 sts=4 ts=4 : -->
