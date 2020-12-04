#!/usr/bin/env python

"""
A log creation service that receives Log Creation commands from an
unmodified client and passes them to a log-server. The purpose of
such a log-creation service is to provide a layer of indirection
between the clients and log-servers (and ensure log-duplication
is handled cleanly, before we can come up with a better solution).


"""

from GDPService import GDPService
import gdp
import gdp_pb2
import random
import argparse
import time
import sqlite3
import threading
import logging
import os
import sys
import struct
from hashlib import sha256
import mysql.connector as mariadb
import mysql.connector.pooling as mariadbpooling
import Queue


SVC_NAME = gdp.GDP_NAME('logcreationservice').internal_name()
DEFAULT_ROUTER_PORT = 8007
DEFAULT_ROUTER_HOST = "172.30.0.1"


########################################################
### these come from gdp/gdp_pdu.h from the GDP C library

# Acks/Naks
GDP_ACK_MIN = 128
GDP_ACK_MAX = 191
GDP_NAK_C_MIN = 192
GDP_NAK_C_MAX = 223
GDP_NAK_C_MIN = 223
GDP_NAC_C_MAX = 239
GDP_NAK_R_MIN = 240
GDP_NAK_R_MAX = 254
GDP_NAK_C_CONFLICT = 201
GDP_NAK_S_NOTIMPL = 225

# specific commands for our use
GDP_CMD_CREATE = 66
GDP_NAK_C_BADREQ = 192

########################################################

class PoolExhausted(Exception):
    pass

class UnknownResponse(Exception):
    pass


class MySQLConnectionPool(mariadbpooling.MySQLConnectionPool):
    """
    Subclass of default connection pool with our little
    enhancements to 'get_connection' and a new function
    'return_connection' to match the SQLiteConnPool
    """

    def get_connection(self, retries=3, timeout=0.500):
        """
        Attempt to get a connection from the pool. Retry if necessary,
        but return a PoolExhausted error if it doesn't work.
        """
        retries_left = retries

        while retries_left > 0:
            try:
                conn = super(MySQLConnectionPool, self).get_connection()
                return conn
            except (mariadb.PoolError, mariadb.Error) as e:
                logging.debug("Waiting for a free connection to MySQL")
                retries_left = retries_left - 1
                time.sleep(timeout)
                if retries_left <= 0:
                    logging.warning("No free connection to MySQL")
                    raise PoolExhausted

        assert False    # should never get here
        
    def return_connection(self, conn):
        """ simply return the connection back to the pool """
        conn.close()


class SQLiteConnPool(object):

    """
    Implement a connection pool for SQLite connections. We cheat
    a bit by telling SQLite to not check whether the connections
    are being shared across threads. Instead, we ensure ourselves
    that such connections are not shared across threads *at the
    same time*.
    """

    def __init__(self, dbname, pool_size=16):
        self.dbname = dbname
        self.pool_size = pool_size
        self.pool = Queue.Queue()
        self.conn_list = [] ## list of all connections we created

        ## fill the queue
        for i in xrange(self.pool_size):
            conn = sqlite3.connect(self.dbname, check_same_thread=False,
                                            timeout=10)
            self.pool.put(conn)
            self.conn_list.append(conn)


    def __del__(self):
        ## close all the connections
        for conn in self.conn_list:
            conn.close() 


    def get_connection(self, retries=3, timeout=0.500):
        """
        return a connection from the pool. Attempt some retries, but
        raise a PoolExhausted error if we can't get it to work
        """
        retries_left = retries

        while retries_left > 0:
            try:
                conn = self.pool.get(block=False)
                return conn
            except Queue.Empty:
                logging.debug("Waiting for a free connection to SQLite")
                retries_left = retries_left - 1
                time.sleep(timeout)
                if retries_left <= 0:
                    logging.warning("No free connection to SQLite")
                    raise PoolExhausted

        assert False    # should never get here


    def return_connection(self, conn):
        """
        return a connection back to the pool. The caller may not
        use the connection once they have returned it back to the pool
        """
        self.pool.put(conn)


########################################################


class logCreationService(GDPService):

    def __init__(self, dbname, router, GDPaddrs, logservers, namedb_info):
        """
        router:         a 'host:port' string representing the GDP router
        GDPaddrs:       a list of 256-bit addresses of this particular service
        logservers:     a list of log-servers on the backend that we use
        dbname:         sqlite database location
        namedb_info:    a remote database (mariadb) where we populate a
                        human name => internal name mapping.
        """

        ## First call the __init__ of GDPService
        super(logCreationService, self).__init__(SVC_NAME, router, GDPaddrs)

        ## Setup instance specific constants
        self.GDPaddrs = GDPaddrs
        self.logservers = logservers
        self.dbname = dbname
        self.namedb_info = namedb_info

        ## Setup connection pools to the databases
        self.__setup_dupdb()
        self.__setup_namedb()


    def __setup_dupdb(self):
        """
        Setup connection pool to our "duplicate detection" database.
        Initialize the database tables if needed.
        """
        need_initializing = not os.path.exists(self.dbname)

        logging.info("Creating connection pool, %r", self.dbname)
        self.dupdb_cpool = SQLiteConnPool(self.dbname, pool_size=16)
        if need_initializing:
            self.__initiate_dupdb()


    def __initiate_dupdb(self):
        """create required tables, indices, etc. """

        logging.info("Initializing database with tables and such")

        ## just create a completely separate connection, since we
        ## haven't really started doing anything interesting yet
        conn = sqlite3.connect(self.dbname)
        cur = conn.cursor()
        cur.execute("""CREATE TABLE logs(logname TEXT UNIQUE, srvname TEXT,
                                ack_seen INTEGER DEFAULT 0,
                                ts DATETIME DEFAULT CURRENT_TIMESTAMP,
                                creator TEXT, rid INTEGER)""")
        cur.execute("CREATE UNIQUE INDEX logname_ndx ON logs(logname)")
        cur.execute("CREATE INDEX srvname_ndx ON logs(srvname)")
        cur.execute("CREATE INDEX ack_seen_ndx ON logs(ack_seen)")
        conn.commit()
        conn.close()


    def __setup_namedb(self):
        """ Setup connection pool to "human=>internal name" database. """

        logging.info("Setting up connection to human=>internal name database")

        if namedb_info.get("host", None) is not None:
            logging.info("Initiating connection to directory server")

            _host = namedb_info.get("host", "gdp-hongd.cs.berkeley.edu")
            _user = namedb_info.get("user", "gdp_creation_service")
            _password = namedb_info.get("passwd", "")
            _database = namedb_info.get("database", "gdp_hongd")
            logging.info("Opening database %r host %r user %r",
                                _database, _host, _user)
            ## the max pool size is 32; this number should certainly
            ## be larger than our local database because of the difference
            ## in latency
            self.namedb_cpool = MySQLConnectionPool(
                                    pool_size=32, host=_host, user=_user,
                                    password=_password, database=_database)
        else:
            logging.info("No information provided for namedb. Skipping")
            self.namedb_cpool = None


    def add_to_hongd(self, humanname, logname):
        """perform the database operation"""

        logging.info("Adding HONGD mapping %s => %s" % 
                            (humanname, self.printable_name(logname)))
        assert self.namedb_cpool is not None

        ## First get a connection from the pool
        conn = self.namedb_cpool.get_connection()
        assert conn is not None

        table = self.namedb_info.get("table", "human_to_gdp")
        query = "insert into "+table+" (hname, gname) values (%s, %s)"

        try: 

            cur = conn.cursor()
            cur.execute(query, (humanname, logname))
            conn.commit()

        except Exception as e:

            ### XXX this is bad; we need to catch specific exceptions, but
            ### I don't know what exact exception to catch.
            logging.warning("Got exception: %r", e)

            ## maybe the connection was lost. If so, attempt to reconnect
            ## and give it another try
            if not conn.is_connected():
                logging.warning("HONGD connection lost. Reconnecting")
                conn.reconnect(attempts=3, delay=1)
                if not conn.is_connected():
                    logging.warning("Could not reconnect with HONGD")

            ## now we still raise the exception for this specific
            ## request, but hopefully we repaired the connection if
            ## that was the issue
            raise e

        finally:
            self.namedb_cpool.return_connection(conn)


    def add_to_dupdb(self, __logname, __srvname, __creator, rid):
        """Add new entry to dupdb"""

        ## First get a connection from the pool
        conn = self.dupdb_cpool.get_connection()
        cur = conn.cursor()

        logging.debug("inserting to database %r, %r, %r, %d",
                            __logname, __srvname, __creator, rid)
        cur.execute("""INSERT INTO logs (logname, srvname, creator, rid)
                            VALUES(?,?,?,?);""",
                            (__logname, __srvname, __creator, rid))
        conn.commit()
        rid = cur.lastrowid
        cur.close()

        ## don't forget to return the connection
        self.dupdb_cpool.return_connection(conn)

        return rid


    def fetch_creator(self, rid):
        """
        When we received a response, lookup the specific row id in the
        dupdb and return appropriate information for creating a spoofed
        response back to the creator.

        Also pdate the specific row to indicate that we have seen
        a response for the given log.

        Raise an UnknownResponse exception in case something is wrong.
        """

        ## First get a connection from the pool
        conn = self.dupdb_cpool.get_connection()
        cur = conn.cursor()

        try:

            ## Fetch the original creator and rid from our database
            cur.execute("""SELECT creator, rid, ack_seen 
                             FROM logs WHERE rowid=?""", (rid,))
            dbrows = cur.fetchall()
            ## We should know about this specific unique request ID
            assert len(dbrows) == 1

            (__creator, orig_rid, ack_seen) = dbrows[0]
            creator = gdp.GDP_NAME(__creator).internal_name()
            ## We should not have seen a response already for this rid
            assert ack_seen == 0

            logging.debug("Setting ack_seen to 1 for row %d", rid)
            cur.execute("UPDATE logs SET ack_seen=1 WHERE rowid=?", (rid,))
            conn.commit()

            ## don't forget to return the connection back to the pool
            cur.close()
            self.dupdb_cpool.return_connection(conn)
            return (creator, orig_rid)

        except AssertionError as e:
            ## XXX handling errors via exceptions is probably better
            ## don't forget to return the connection back to the pool
            cur.close()
            self.dupdb_cpool.return_connection(conn)
            raise UnknownResponse


    def request_handler(self, req):
        """
        The routine that gets invoked when a PDU is received. In our case,
        it can be either a new request (from a client), or a response from
        a log-server.
        """

        # first, deserialize the payload from req pdu.
        payload = gdp_pb2.GdpMessage()
        payload.ParseFromString(req['data'])

        # early exit if a router told us something (usually not a good
        # sign)
        if payload.cmd >= GDP_NAK_R_MIN and \
                         payload.cmd <= GDP_NAK_R_MAX:
            logger.warning("Routing error, src: %r",
                            self.printable_name(req['src']))
            return

        # check if it's a request from a client or a response from a logd.
        if payload.cmd < GDP_ACK_MIN:      ## it's a command

            ## First check for any error conditions. If any of the
            ## following occur, we ought to send back a NAK
            if req['src'] in self.logservers:
                logging.warning("error: received cmd %d from server",
                                                          payload.cmd)
                return self.gen_nak(req, gdp_pb2.NAK_C_BADREQ)

            if payload.cmd != gdp_pb2.CMD_CREATE:
                logging.warning("error: recieved unknown request")
                return self.gen_nak(req, gdp_pb2.NAK_S_NOTIMPL)

            ## By now, we know the request is a CREATE request from a client

            ## figure out the data we need to insert in the database
            humanname, logname = self.extract_name(payload.cmd_create)

            ## Add this to the humanname=>logname mapping directory
            #FIXME this shouldn't be done before the log is successfully
            #FIXME created on the server!
            if humanname is not None and self.namedb_cpool is not None:
                try:
                    # Note that logname is the internal 256-bit name
                    # (and not a printable name)
                    self.add_to_hongd(humanname, logname)

                except mariadb.Error as error:
                    # XXX we should probably handle the situation where
                    # this is simply a retransmit from an aggressive client.
                    logging.warning("Couldn't add mapping. %s", str(error))
                    return self.gen_nak(req, gdp_pb2.NAK_C_CONFLICT)

                except PoolExhausted as error:
                    logging.warning("DB connection pool exhausted")
                    return self.gen_nak(req, gdp_pb2.NAK_S_INTERNAL)

                except Exception as e:
                    ## report back to the client that something went wrong
                    logging.warning("Generic exception: %r", e)
                    return self.gen_nak(req, gdp_pb2.NAK_S_INTERNAL)

            srvname = random.choice(self.logservers)
            ## private information that we will need later
            creator = req['src']
            rid = payload.rid

            ## log this to our backend database. Generate printable
            ## names (except the null character, which causes problems)
            __logname = self.printable_name(logname)
            __srvname = self.printable_name(srvname)
            __creator = self.printable_name(creator)

            logging.info("Received Create request for logname %r, "
                                "picking server %r", __logname, __srvname)

            try:
                ## Add this information to the dupdb, and get back the
                ## row ID in the database that we will include in the spoofed
                ## request to an actual log-server
                __rid = self.add_to_dupdb(__logname, __srvname, __creator, rid)

                spoofed_req = req.copy()
                spoofed_req['src'] = req['dst']
                spoofed_req['dst'] = srvname
                ## make a copy of payload so that we can change rid
                __spoofed_payload = gdp_pb2.GdpMessage()
                __spoofed_payload.ParseFromString(req['data'])
                __spoofed_payload.rid = __rid
                spoofed_req['data'] = __spoofed_payload.SerializeToString()

                # now return this spoofed request back to transport layer
                # Since we have overridden the destination, it will go
                # to a log server instead of the actual client
                return spoofed_req

            except sqlite3.IntegrityError:

                ## We reach here if we are trying to add a duplicate
                ## entry in our database (which is the whole purpose
                ## of such bookkeeping).
                ## We send a NAK to the creator.
                logging.warning("Log already exists")
                return self.gen_nak(req, gdp_pb2.NAK_C_CONFLICT)

            except Exception as e:

                ## Generic exception
                logging.warning("Got generic exception %r", e)
                return self.gen_nak(req, gdp_pb2.NAK_S_INTERNAL)


        else: ## response.

            ## Sanity checking
            if req['src'] not in self.logservers:
                logging.warning("error: received a response from non-logserver")
                return self.gen_nak(req, gdp_pb2.NAK_C_BADREQ)

            logging.info("Response from log-server, row %d", payload.rid)

            try:
                (creator, orig_rid) = self.fetch_creator(payload.rid)
            except UnknownResponse as e:
                ## this happens because either we don't know about the
                ## specific request ID, or we have already received a
                ## response for the said request ID. In any case, it is
                ## appropriate to return a NAK back to the log-server
                logging.warning("error: bogus response")
                return self.gen_nak(req, gdp_pb2.NAK_C_BADREQ)
            except Exception as e:
                ## We shouldn't be returning any NAK back to a log-server.
                ## Ideally, we should be returning a NAK back to the original
                ## client, but we don't have a name for the client yet. The
                ## most appropriate strategy is to simply report the error on
                ## the console.
                logging.warning("Generic exception: %r", e)
                return None

            # create a spoofed reply and send it to the client
            spoofed_reply = req.copy()
            spoofed_reply['src'] = req['dst']
            spoofed_reply['dst'] = creator

            spoofed_payload = gdp_pb2.GdpMessage()
            spoofed_payload.ParseFromString(req['data'])
            spoofed_payload.rid = orig_rid

            spoofed_reply['data'] = spoofed_payload.SerializeToString()

            # return this back to the transport layer
            return spoofed_reply


    def gen_nak(self, req, nak=gdp_pb2.NAK_C_BADREQ):

        logging.info("sending a NAK(%d) [src:%r, dst:%r]" %
                                    (nak, self.printable_name(req['dst']),
                                          self.printable_name(req['src'])))
        resp = dict()
        resp['src'] = req['dst']
        resp['dst'] = req['src']

        resp_payload = gdp_pb2.GdpMessage()
        resp_payload.ParseFromString(req['data'])
        resp_payload.cmd = nak
        resp_payload.nak.ep_stat = 0

        resp['data'] = resp_payload.SerializeToString()
        return resp


    @classmethod
    def extract_name(cls, create_msg):
        """
        returns a tuple (human name, internal name) from the Protobuf
        CmdCreate message. Any changes to the create message format
        should ideally only need changes here.

        Note that this needs to peek into the serialized metadata..
        """

        def __deserialize_md(data):
            """returns a dictionary"""
            ret = {}

            try:
                nmd = struct.unpack("!H", data[0:2])[0]
                offset = 2
                tmplist = []    ## list of (md_id, md_len) tuples
                for _ in xrange(nmd):
                    md_id = struct.unpack("!I", data[offset:offset+4])[0]
                    md_len = struct.unpack("!I", data[offset+4:offset+8])[0]
                    offset += 8
                    tmplist.append((md_id, md_len))

                for (md_id, md_len) in tmplist:
                    ret[md_id] = data[offset:offset+md_len]
                    offset += md_len

                if offset!=len(data):
                    logging.warning("%d bytes leftover when parsing metadata",
                                                            len(data)-offset)

            except struct.error as e:
                logging.warning("%s", str(e))
                logging.warning("Incomplete data when parsing metadata")
                ret = {}

            return ret


        ## Let's see if there's a human name in the metadata. Could be null
        md_dict = __deserialize_md(create_msg.metadata.data)
        humanname = md_dict.get(0x00584944, None)   # XID

        if create_msg.HasField("logname"):
            _logname = create_msg.logname
        else:
            _logname = None

        ## Let's take the hash of the metadata, and see what we have
        smd = create_msg.metadata.SerializeToString()
        logname = sha256(smd).digest()
        if _logname is not None and _logname != logname:
            logging.debug("Overriding hash of metadata with provided name")
            logname = _logname

        return (humanname, logname) 


    @staticmethod
    def printable_name(x):
        """ return the printable name without the null character """
        return gdp.GDP_NAME(x, force_internal=True).printable_name()[:-1]


if __name__ == "__main__":

    ## argument parsing
    parser = argparse.ArgumentParser(description="Log creation service")

    parser.add_argument("-v", "--verbose", action='store_true',
                                help="Be quite verbose in execution.")
    parser.add_argument("-i", "--host", type=str, default=DEFAULT_ROUTER_HOST,
                                help="host of gdp_router instance. "
                                     "default = %s" % DEFAULT_ROUTER_HOST)
    parser.add_argument("-p", "--port", type=int, default=DEFAULT_ROUTER_PORT,
                                help="port for gdp_router instance. "
                                     "default = %d" % DEFAULT_ROUTER_PORT)
    parser.add_argument("-d", "--dbname", type=str, required=True,
                                help="filename for sqlite database")
    parser.add_argument("-a", "--addr", type=str, nargs='+', required=True,
                                help="Address(es) for this service, typically "
                                     "human readable names.")
    parser.add_argument("-s", "--server", type=str, nargs='+', required=True,
                                help="Log server(s) to be used for actual log "
                                     "creation, typically human readable names")

    ## The following for namedb/hongd
    parser.add_argument("--namedb_host", help="Hostname for namedb")
    parser.add_argument("--namedb_user", help="Username for namedb")
    parser.add_argument("--namedb_passwd", help="Password for namedb")
    parser.add_argument("--namedb_pw_file", help="File with namedb passwd")
    parser.add_argument("--namedb_database", help="Database name for namedb")
    parser.add_argument("--namedb_table", help="Table name for namedb")


    args = parser.parse_args()

    ## done argument parsing, instantiate the service
    if args.verbose:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.INFO)

    ## parse arguments
    router = "%s:%d" % (args.host, args.port)
    addrs = [gdp.GDP_NAME(x).internal_name() for x in args.addr]
    servers = [gdp.GDP_NAME(x).internal_name() for x in args.server]

    namedb_info = {}
    if args.namedb_host is not None:
        namedb_info["host"] = args.namedb_host
    if args.namedb_user is not None:
        namedb_info["user"] = args.namedb_user
    if args.namedb_passwd is not None:
        namedb_info["passwd"] = args.namedb_passwd
    elif args.namedb_pw_file is not None:
        with open(args.namedb_pw_file) as pwfile:
            namedb_info["passwd"] = pwfile.readline().rstrip()
    if args.namedb_database is not None:
        namedb_info["database"] = args.namedb_database
    if args.namedb_table is not None:
        namedb_info["table"] = args.namedb_table


    logging.info("Starting a log-creation service...")
    logging.info(">> Connecting to %s", router)
    logging.info(">> Servicing names %r", args.addr)
    logging.info(">> Using log servers %r", args.server)
    logging.info(">> Human name directory at %r", args.namedb_host)

    ## instantiate the service
    service = logCreationService(args.dbname, router, addrs,
                                                servers, namedb_info)

    ## all done, start the service (and sleep indefinitely)
    logging.info("Starting logcreationservice")
    service.start()

    try:
        while True:
            time.sleep(1)
    except (KeyboardInterrupt, SystemExit):
        service.stop()
