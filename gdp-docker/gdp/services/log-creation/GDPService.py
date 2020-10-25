#!/usr/bin/env python

"""

A library to perform network related operations (setting up TCP
connections, parsing incoming PDUs, sending out replies, etc) for
a GDP service.

To use GDPService for your own service, at the very minimum, you
need to subclass GDPService and override the method 'request_handler'.
Then, you need to create an instance of this service with the 256-bit
GDP address that this service should listen on and the address of
a GDP router (as a "IP-address:port" string).

Any GDP PDU destined to the GDP address specified for at the
instantiation time is parsed and passed as a dictionary to the
'request_handler'. This request handler should act on the received
data and return a python dictionary containing field=>value pairs
for a PDU to be sent as a reply. GDPService adds some defaults
(such as source address, destination address, etc) unless these
are overridden in the response dictionary.

"""

from twisted.internet.protocol import ReconnectingClientFactory, Protocol
from twisted.internet.task import LoopingCall
from twisted.internet import reactor
import threading
import struct
import logging


def to_hex(data):
    hex_repr = "".join([hex(ord(t))[2:] for t in data])
    if len(data) > 0:
        hex_repr = '\\x' + hex_repr
    return hex_repr

def to_hex_debug(data):
    """with 20 chars in each line"""
    hex_repr = data.encode("hex")
    s = ""
    for i in xrange(0, len(hex_repr), 40):
        s += hex_repr[i:i+40] + "\n"
    return s

def pprint(d):
    """
    Just our own pprint function that behaves nicely with binary strings
    """

    s = "{" + "\n"
    for k in d.keys():
        tmp = d[k]
        if isinstance(tmp, int):
            p = str(tmp)
        else: p = "'" + to_hex(tmp) + "'"
        s += " '" + k + "': " + p + "," + "\n"
    s += "}"
    return s


class GDPProtocol(Protocol):

    """
    An instance of this class is created for every connection. But
    then, we only have one connection to maintain.
    """

    def __init__(self, servicename, req_handler, GDPaddrs):
        """ The GDP address for the service end point """

        self.GDPaddrs = GDPaddrs
        self.request_handler = req_handler
        self.buffer = ""
        self.advertising_call = None
        self.myaddr = servicename


    def __make_ad(self, addr):
        """return a string that corresponds to an advertisement message"""
        # XXX: what dst address should we be using for advertising?
        # It appears that the clients send their own address as both
        # source and destination in the latest version of GDP library

        # see gdp/gdp_chan.h for format details.
        # These hard-coded values correspond to advertising a single
        # address to the routing layer [the PDU is 76 octet long].
        # TODO change the hard-coded values to something more general.
        ad = ('\x04' + '\x13' + '\x40' + '\x0f' +
                '\x00'*4 + '\x00'*2 + '\x00'*2 +
                 addr + self.myaddr)
        return ad


    def advertise(self):
        # Send the advertisement messages, do network I/O in reactor thread
        logging.info("%r, Advertising %d names", self, len(self.GDPaddrs))
        for addr in [self.myaddr] + self.GDPaddrs:
            advertisement = self.__make_ad(addr)
            logging.debug("Sending advertisement: \n%s",
                                    to_hex_debug(advertisement))
            reactor.callFromThread(self.transport.write, advertisement)


    def connectionMade(self):
        """
        This gets called when the connection gets established
        """
        logging.info("Connection made")
        ## periodic readvertisements, also doubles as a keep-alive
        self.advertising_call = LoopingCall(self.advertise)
        self.advertising_call.start(30, now=True)


    def terminateConnection(self, reason):
        """
        If we can't make sense out of any data on this connection
        anymore for any reason, terminate the connection.
        """

        # This just terminates the connection
        logging.warning("Terminating connection, %r", reason)
        self.transport.abortConnection()


    def dataReceived(self, data):

        """
        parses PDUs and invokes the PDU processing code in a separate
        worker thread
        """

        assert threading.currentThread().getName() == "ReactorThr"

        # Make sure we have some flow control.

        l1 = len(self.buffer)   # what is left from previous calls
        l2 = len(data)          # what we got now
        l = 0                   # seek pointer.  0 <= l < l1+l2

        while reactor.running:

            def __get_byte_num(n):
                """ Reference to bit number 'l+n' in self.buffer + data,
                    0-indexed """
                if l + n < l1:
                    return self.buffer[l + n]
                else:
                    return data[l + n - l1]

            # check the version number
            if (l + 1) > (l1 + l2):
                break       # can't figure out the version
            version = __get_byte_num(0)

            if version == '\x02' or version == '\x03':
                logging.warning("Someone is using old version number")

            if version != '\x04':
                self.terminateConnection("bogus version number")
                logging.info("buflen: %d, datalen: %d, seekptr: %d, "
                        "version: %d", l1, l2, l, ord(__get_byte_num(0)))
                __bogus_data = str(self.buffer) + str(data)
                logging.debug("Bogus data:\n%s", to_hex_debug(__bogus_data))
                break

            # calculate the header length.
            if (l + 2) > (l1 + l2):
                break       # can't figure out the header length
            hdr_len = (ord(__get_byte_num(1)) & 0x3f)*4

            # rest of the header
            if (l + hdr_len) > (l1 + l2):
                # incomplete header for the current message
                # we need at least "hdr_len" bytes for the header of
                # this PDU. TODO make this compatible with flow-id, etc.
                break

            ## length of the actual payload after the header
            data_len = ord(__get_byte_num(10))*256 + ord(__get_byte_num(11))

            # pdu_len is the total length of the PDU
            pdu_len = hdr_len + data_len
            if (l + pdu_len) > (l1 + l2):
                break     # incomplete message

            # 3 cases:
            # - entire message in self.buffer
            # - message split between self.buffer and data
            # - entire message in data
            # case 2 can only happen once every call of this function

            message = None
            if (l + pdu_len) <= l1:  # case 1
                message = buffer(self.buffer, l, pdu_len)
            elif l >= l1:         # case 3
                message = buffer(data, l - l1, pdu_len)
            else:               # case 2
                # This is not a buffer anymore because of concatenation
                message = (buffer(self.buffer, l, l1 - l) +
                           buffer(data, 0, (pdu_len) - (l1 - l)))

            l = l + pdu_len       # update l

            # Work on this PDU, all the PDU processing logic goes here;
            # this gets called in a separate worker thread.
            reactor.callInThread(self.process_PDU, message, hdr_len, data_len)

            # End of while loop

        if reactor.running:

            # Now update the buffer with the remaining data
            # 2 cases: 1) Either the new buffer is a substring of just data
            # OR, 2) it is substring of old buffer + all data

            if l > l1:    # case 1
                self.buffer = buffer(
                    data, l - l1, (l1 + l2) - l)    # just a buffer
            else:       # case 2
                self.buffer = (buffer(self.buffer, l, l1 - l) +
                           buffer(data, 0, len(data)))  # Expensive string
            # if len(self.buffer)>0: assert ord(self.buffer[0])==2


    def process_PDU(self, message, hdr_len, data_len):
        """
        Act on a PDU that is already parsed. All the PDU processing
        logic goes here.

        Note that the message could be a string or a buffer. For now, it
        just parses it into a dictionary, calls the request request
        handler on the parsed dictionary and sends the returned python
        dictionary to the provided destination
        """

        # DEBUG-info:
        # 'message' is a PDU received by gdp_router from remote
        # address 'self.transport.getPeer()'

        # create a dictionary
        msg_dict = {}
        msg_dict['ver'] = message[0]
        msg_dict['hdr_len'] = hdr_len       # byte index 1

        msg_dict['addr_format'] = ord(message[2]) & 0x07
        msg_dict['flags'] = (ord(message[2]) & 0x18)>>3
        msg_dict['type'] = (ord(message[2]) & 0xe0)>>5
        if msg_dict['addr_format'] != 0:
            logging.warning("addr formats not implemented yet")
            return

        msg_dict['ttl'] = ord(message[3]) & 0x3f

        # message[4:5] contains the sequence number, which we can ignore
        for i in xrange(6,10):
            if message[i] != '\x00':
                logging.warning("Fragments not implemented yet")

        msg_dict['data_len'] = data_len     # byte index 10, 11

        msg_dict['dst'] = message[12:12+32]
        msg_dict['src'] = message[44:44+32]

        msg_dict['data'] = message[hdr_len:]

        ## Do some sanity checks here on the PDU contents. Hopefully
        ## we didn't run into any parsing issues so far

        if msg_dict['type'] == 0:       # normal behavior
            pass
        elif msg_dict['type'] == 4:     # router NAK
            ## just report the NAK and return immediately.
            logging.warning('Router NAK; src: %r, dst: %r' %
                                (msg_dict['src'], msg_dict['dst']))
            return
        else:
            logging.warning('PDU type: %d, not implemented' % 
                                msg_dict['type'])
            return

        assert len(msg_dict['data']) == data_len


        # get the response dictionary
        # request_handler is to be supplied by GDPProtocolFactory,
        # which in turn gets it from GDPService
        # XXX: What all information should be made available here?
        keys = set(msg_dict.keys()) & set(['dst', 'src', 'data', 'type'])
        data = {k: msg_dict[k] for k in keys}
        logging.debug("Incoming:\n%s", pprint(data))
        resp = self.request_handler(data)
        logging.debug("Outgoing:\n%s", pprint(resp))

        if resp is not None:
            # if a specific source or a destination is specified
            # in the response, it will take precedence.
            self.send_PDU(resp, src=msg_dict['dst'], dst=msg_dict['src'])


    def send_PDU(self, data, **kwargs):
        """
        Create a GDP message and write it to the network connection
        destination is a 256 bit address. This should *not* be called
        from the reactor thread.

        Qpen question: what all fields should be accepted from the
        caller? So far: dst, src, data.
        Note that 'data' is a serialized protobuf message.
        """

        assert threading.currentThread().getName() != "ReactorThr"

        msg = {}
        msg['ver'] = '\x04'
        msg['hdr_len'] = '\x13'     # enough to hold two full addrs

        msg['addr_format'] = 0 & 0x07
        msg['flags'] = 0 & 0x03
        msg['type'] = 0 & 0x07

        _byte_2 = chr(msg['type']<<5 & msg['flags']<<3 & msg['addr_format'])

        msg['ttl'] = '\x0f'                  # FIXME

        msg['seq_frag'] = '\x00'*6

        msg['data_len'] = struct.pack("!H", len(data.get('data', '')))

        # make sure we do have a source and a destination
        assert 'src' in data.keys() + kwargs.keys()
        assert 'dst' in data.keys() + kwargs.keys()
        msg['dst'] = str(data.get('dst', kwargs['dst']))
        msg['src'] = str(data.get('src', kwargs['src']))
        msg['data'] = str(data.get('data', ''))

        # some assertions (all may not be be necessary)
        valid_size = {  'data_len': 2,
                        'dst': 32, 'src': 32 }
        for k in valid_size.keys():
            assert len(msg[k]) == valid_size[k]

        message = ( msg['ver'] + msg['hdr_len'] + _byte_2 +
                    msg['ttl'] + msg['seq_frag'] + msg['data_len'] +
                    msg['dst'] + msg['src'] +
                    msg['data'])

        ## the following gets called in the reactor thread
        reactor.callFromThread(self.transport.write, message)



class GDPProtocolFactory(ReconnectingClientFactory):

    def __init__(self, servicename, req_handler, GDPaddrs):
        "Initialize with the request handler and list of GDP addresses"
        self.GDPaddrs = GDPaddrs
        self.request_handler = req_handler
        self.protocol = None
        self.myaddr = servicename

    def buildProtocol(self, remoteaddr):
        self.protocol = GDPProtocol(self.myaddr,
                                    self.request_handler, self.GDPaddrs)
        return self.protocol

    def clientConnectionLost(self, connector, reason):
        logging.error("Conenction lost: %r", reason)
        self.protocol.advertising_call.stop()
        ReconnectingClientFactory.clientConnectionLost(
                                                self, connector, reason)

    def clientConnectionFailed(self, connector, reason):
        logging.error("Connection failed: %r", reason)
        ReconnectingClientFactory.clientConnectionFailed(
                                                self, connector, reason)

class GDPService(object):
    """
    A generic GDP Service. In a perfect world, all GDP services
    should be subclasses of this class.

    To implement your own GDP service, override the
    'request_handler' method and put your service's request
    processing logic.

    One can also override 'setup' to do service specific startup
    functions. These functions are called *before* establishing
    connection to the GDP router.
    """


    def __init__(self, servicename, router, GDPaddrs):
        """
        router: ip:port for a GDP router/switch that we connect to
        GDPaddrs: list of 256 bit GDP addresses that we listen to
        """

        assert isinstance(router, str)
        assert isinstance(GDPaddrs, list)

        ## parse the GDP router host:port
        t = router.split(":")
        self.router_host = t[0]
        self.router_port = int(t[1])
        self.GDPaddrs = GDPaddrs
        self.myaddr = servicename

        ProtocolFactory = GDPProtocolFactory(self.myaddr,
                                        self.request_handler, GDPaddrs)

        ## Call service specific setup code
        self.setup()

        ## Establish connection to the router (the reactor isn't
        ## running yet, so this will only get established when
        ## 'start()' is called.
        logging.debug("Connecting to host:%s, port:%d",
                        self.router_host, self.router_port)
        reactor.connectTCP(self.router_host,
                        self.router_port, ProtocolFactory)


    def setup(self):
        "Initial setup for this service. Default: do nothing"
        pass


    def request_handler(self,req):
        """
        Receive a request (dictionary), with the following keys:
            src = Source GDP address (256 bit)
            dst = Destination GDP address,
            data = actual payload (a protobuf that is yet to be parsed)
        Ideally, a GDP service will override this method
        """
        pass


    def start(self):
        """
        Start the reactor (in a separate thread) and returns immediately. One
        should put appropriate logic after calling 'start' to keep main thread
        alive as long as possible.
        """
        reactor_thr = threading.Thread(name="ReactorThr",
                                 target=reactor.run,
                                 kwargs={'installSignalHandlers':False})
        reactor_thr.start()


    def stop(self):
        """ Stop serving requests, (ideally) called from main-thread """
        reactor.callFromThread(reactor.stop)
