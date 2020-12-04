#!/usr/bin/env python

# ----- BEGIN LICENSE BLOCK -----
#	GDP: Global Data Plane
#	From the Ubiquitous Swarm Lab, 490 Cory Hall, U.C. Berkeley.
#
#	Copyright (c) 2015-2019, Regents of the University of California.
#	All rights reserved.
#
#	Permission is hereby granted, without written agreement and without
#	license or royalty fees, to use, copy, modify, and distribute this
#	software and its documentation for any purpose, provided that the above
#	copyright notice and the following two paragraphs appear in all copies
#	of this software.
#
#	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
#	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
#	PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
#	EVEN IF REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
#	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
#	FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION,
#	IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO
#	OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS,
#	OR MODIFICATIONS.
# ----- END LICENSE BLOCK -----


## XXX XXX XXX
## XXX Obsolete. Needs update to the latest API version (ver 2)
## XXX XXX XXX


"""

A key value store with checkpoints. All persistent data is stored in a log
called the root. For performance, an in-memory copy is maintained. At the
initilization, the name of the rootlog is to be supplied by a user.
It is the user's responsibility to make sure that a single log is written
to by a single writer, the behavior is undefined otherwise.

Individual records in the root log are serialized versions of python data 
structures. There are multiple kinds of records in the log itself, as 
described later.


### Usage ###

Simplified example:
> kv = KVstore('rootlog', 'keyfile.pem', KVstore.MODE_RW)
> kv['key1'] = 'val1'
> p = kv['key1']
> print p
val1
>

Public interface:

* Initialization: 
  Required parameter: name of the log. A private signature key
  Optional parameters: I/O mode (single writer mode vs Read only mode.) 
  You can specify more optional parameters for tweaking the performance. 
  Example: to create a Read/Write key-value store backed by an existing
    log 'example_log', use the following:
  >>> kvstore = KVstore('exmaple_log', 'keyfile.pem', KVstore.MODE_RW)

* Querying existing keys:
  - either use kvstore[key] to fetch value of 'key' in object 'kvstore'.
  - OR, use kvstore.get(key,ts) to fetch most recent value of 'key' before
    timestamp 'ts'. 'ts' is a float value representing seconds since epoch
  *NOTE*: when using .get(), a tuple (write_ts, value) is returned, where
    'write_ts' is the timestamp associated with 'value'. However, just 'value'
    is returned when using the '[]' interface, in order to maintain 
    compatibility with a dictionary interface.

* Adding new keys/updating existing keys:
  - either use kvstore[key] = value to set 'key' to 'value'.
  - OR use kvstore.set(key,value).
  *NOTE*: A timestamp is automatically generated and included in the data
    appended to the underlying log. This timestamp is different than the 
    commit timestamp for a particular record set by the log server.

* Additional usefule properties:
  - len(kvstore) returns the number of keys in an object 'kvstore'.
  - The `in` operator could be used to do a presence test: 
    `key in kvstore` is True if 'key' exists in 'kvstore', False otherwise
  - It is possible to iterate over all the keys similar to other iterable 
    data structures (like list).
  - keys() returns all unique keys, similar behavior as a dictionary
  - values() returns values of all keys, similar behavior as a dictionary



### Internal structure ###

As mentioned earlier, individual records in the log are serialized versions
of python data structures.

** Records **

All records are in the form:
    (<metadata>, <data>)
where both <metadata> and <data> are dictionaries. <metadata> is maintained
directly by the key-value store and is not under direct user control.


**Typical record**

A typical record that adds/updates/deletes one or more keys in the KVstore.

metadata:
{   timestamp: <ts>,
    type: "regular",
    rec_no: <int>
}

data:
{   key1: val1,
    key2: val2,
    ...
}

The timestamp is accurate only to a certain degree, and is included by the
    key-value store wrapper.

**Checkpoint record**

A record that provides a snapshot of a certain record range in the KVstore.
Each checkpoint has a level associated with it, where level is an integer from
0-9. The levels behave pretty much the same way as the unix 'dump' utility ---
level 0 means a full copy of the data upto the current record, any level above
0 provides a snapshot of all the data since last dump of a lower level.

Each checkpoint contains some metadata, especially the current checkpoint level
number and the range of the first and last record included in the checkpoint.

metadata:
{
    timestamp: <float>,
    type: "checkpoint",
    rec_no: <int>,
    cp_range: (<cp_from>, <cp_upto>),
    cp_level: <int>,
    cp_ts_dict: {...},
    ts_dict: {...}
}

cp_ts_dict is a list of recno=>timestamp mapping, includes all checkpoint 
records so far
ts_dict is a list of recno=>timestamp mapping for all records between the
most recent checkpoint and this. NOTE: It is not the same as the range of 
records covered by the current checkpoint record.


## TODO: 
- handle non-existent keys better than None


"""

import sys
sys.path.append("../")
                # So that we can actually load the python_api module

import gdp      # load the main package
import cPickle  # for serialization
import threading
import time     # for timestamping
from collections import OrderedDict


class KVstore:

    # checkpoint frequency
    __freq = None               # initialized in __init__

    ## Criteria for checkpointing ##
    # Bump up CP level if size(new_cp)>N*size(old_cp)
    #   This is useful for too many unique keys
    __cp_size_factor = None     # initialized in __init__
    # Bump up CP level if old CP and new CP have keys in common
    __cp_X_factor = None        # initialized in __init__

    # cache size
    __cache_size = None         # initialized in __init__


    # modes: Read only, or read-write. There can be multiple kv-instances
    #   all pointing back to a single log. However, at most one can be in
    #   read/write mode. There is no way to enforce this at this level.
    (MODE_RO, MODE_RW) = (0,1)

    # throw this when a read-only key-value store is altered
    class writeToReadOnlyKVstore(Exception):
        pass


    @staticmethod
    def __to_datum(ds):
        """
        serialize a data structure to string and create a 'datum' object 
            that can be passed on to the GDP append.
        """
        datum = {"data": cPickle.dumps(ds)}
        return datum


    @staticmethod
    def __from_datum(datum):
        """
        take the data out of a gdp datum, unserialize the data structure
            and return that data structure
        """
        ds = cPickle.loads(datum["data"])
        return ds


    def __init__(self, root, mode=MODE_RO, keyfile=None,
                            freq=100, cache_size=1000,
                            size_factor=2, X_factor=0.8):
        """
        Initialize the instance with the root log. By default, we open 
             log in read only mode.
          Parameters name: description
        - keyfile        : A private signing key for the log (PEM format)
        - mode           : Read-only or Read-Write mode
        - freq           : checkpoint frequency
        - cache_size     : max records to hold in in-memory cache
        - size_factor    : Change the checkpoint level if the size
                             of the new checkpoint differs by this factor
        - X_factor       : Change the checkpoint level if there is a 
                             certain amount of overlap in keys of old 
                             checkpoint and new checkpoint
        """

        self.__iomode = mode
        self.__freq = freq
        self.__cp_size_factor = size_factor
        self.__cp_X_factor = X_factor
        self.__cache_size = cache_size
        assert self.__cache_size > 0

        gdp_iomode = gdp.GDP_MODE_RO if mode==self.MODE_RO else gdp.GDP_MODE_RA
    
        gdp.gdp_init()  ## XXX: Not sure if this is the best idea
        # Setup the key

        open_info = {}
        if keyfile is not None:
            skey = gdp.EP_CRYPTO_KEY(filename=keyfile,
                                        keyform=gdp.EP_CRYPTO_KEYFORM_PEM,
                                        flags=gdp.EP_CRYPTO_F_SECRET)
            open_info['skey'] = skey

        self.__root = gdp.GDP_NAME(root)
        self.__root_handle = gdp.GDP_GCL(self.__root, gdp_iomode, open_info)

        # a cache for records. recno => datum
        # datum may or may not contain a timestamp and recno
        self.__cache = OrderedDict()

        # find the number of records by querying the most recent record
        try:
            datum = self.__root_handle.read(-1)
            self.__num_records = datum["recno"]
            self.__cache[self.__num_records] = datum
        except (gdp.MISC.EP_STAT_SEV_ERROR, gdp.MISC.EP_STAT_SEV_WARN) as e:
            if "Berkeley:Swarm-GDP:404" in e.msg:
                self.__num_records = 0
            else:
                raise e

        # set up lock for adding new data to the log
        # >> we want the __setitems__ to be atomic, because that also
        #    includes the checkpointing logic
        self.log_lock = threading.Lock()  # unused in MODE_RO

        # for a read-only KVstore, make sure we have a subscription
        #   in a separate thread to keep things most up to date
        if self.__iomode == self.MODE_RO:
            t = threading.Thread(target=self.__subscription_thread)
            t.daemon = True
            t.start()


    def debug(self):
        """ Some debugging functions just to help development """
        return self.__cache


    def __subscription_thread(self):
        """ A separate thread to make sure we have the latest records """

        assert self.__iomode == self.MODE_RO

        self.__root_handle.subscribe(0, 0, None)
        timeout = {'tv_sec':0, 'tv_nsec':100*(10**6), 'tv_accuracy':0.0}

        while True:

            event = self.__root_handle.get_next_event(timeout)
            if event is None: continue 

            assert event["type"] == gdp.GDP_EVENT_DATA
            # should we do some locking? All operations we do are 
            #   atomic, aren't they?

            # make sure cache size doesn't go above
            while len(self.__cache)>=self.__cache_size:
                self.__cache.popitem(last=False)
            self.__num_records += 1
            self.__cache[self.__num_records] = event["datum"]


    def __read(self, recno):
        "Read a single record. A wrapper around the cache"

        if recno<0: recno = self.__num_records+recno
        if recno not in self.__cache:
            datum = self.__root_handle.read(recno)
        else:
            # we need to remove and re-write the item to make LRU work
            datum = self.__cache.pop(recno)

        # make sure cache size doesn't go above
        while len(self.__cache)>=self.__cache_size:
            self.__cache.popitem(last=False)
        self.__cache[recno] = datum

        return self.__from_datum(self.__cache[recno])


    def __append(self, ds):
        "Append a single record. A wrapper around the cache"

        # any attempt to write to a read-only KVstore should be 
        #   caught by now, in __setitems__
        assert self.__iomode == self.MODE_RW

        datum = self.__to_datum(ds)
        self.__root_handle.append(datum)

        # make sure cache size doesn't go above
        while len(self.__cache)>=self.__cache_size:
            self.__cache.popitem(last=False)

        self.__num_records += 1
        self.__cache[self.__num_records] = datum


    def __setitems__(self, kvpairs):
        """
        Update multiple values at once. At the core of all updates.
            kvpairs: a dictionary of key=>value mappings
        >> we want the __setitems__ to be atomic, because that also
           includes the checkpointing logic. We don't want to be 
           in a situation where one thread is doing checkpointing 
           while the other appends another record
        """

        if self.__iomode == self.MODE_RO:
            raise self.writeToReadOnlyKVstore()

        self.log_lock.acquire()

        if (self.__num_records+1)%self.__freq==0:
            self.__do_checkpoint()

        # make sure input is in the desired form
        assert isinstance(kvpairs, dict)

        # create the metadata dict
        metadata = { "timestamp": time.time(), 
                     "type": "regular", 
                     "rec_no": self.__num_records+1 }
        rec = (metadata, kvpairs)

        # append this to the log now.
        self.__append(rec)

        self.log_lock.release()

    
    def __setitem__(self, key, value):
        "set key to value"
        return self.__setitems__({key: value})


    def set(self, key, value):
        """ Just an alias for __setitem__ """
        return self.__setitem__(key, value)


    def __delitem__(self, key):
        "remove an existing entry in the log"
        return self.__setitems__({key: None})


    def __record_iterator(self, start_rec=0):
        """
        a generator function 
        iterates over regular and checkpoint records in reverse 
            chronological order starting from (and including )start_rec
            such that the entire range is covered

        returns the entire record (including metadata) at each yield

        if start_rec == 0, start from the latest record
        """


        if start_rec==0:
            cur = self.__num_records
        else:
            cur = start_rec

        while cur>0:

            rec = self.__read(cur)
            (metadata, data) = rec
            assert metadata["rec_no"] == cur

            if metadata["type"] == "regular":           # regular record
                yield rec
                cur = cur-1
                continue

            elif metadata["type"] == "checkpoint":      # checkpoint record
                assert metadata["cp_range"][1] == cur-1     # sanity check
                yield rec
                cur = metadata["cp_range"][0]-1
                continue

            else:                           # unknown record
                assert False

        assert cur==0


    def __getitem__(self, key):
        "get value of key"

        # sequentially read rcords from the very end till we find
        #   the key we are looking for, or we hit a checkpoint record

        for (metadata, data) in self.__record_iterator(): 
            # data is either dictionary from a regular record, or from checkpoint
            if key in data:
                return data[key]

        return None


    def get(self, key, timestamp):
        """
        get the most recent value of key that is before provided timestamp
        """

        # first we need to find the record number from where we should start
        #   an iteration from. 

        rec_no = -1
        for (metadata, data) in self.__record_iterator():
            if metadata["type"] == "regular":
                if metadata["timestamp"]<timestamp:
                    # this is where we stop this iteration
                    rec_no = metadata["rec_no"]
                    break

            elif metadata["type"] == "checkpoint":

                # we can find exact recno by two record fetches, at most
                cp_ts_dict = metadata["cp_ts_dict"]
                cp_rec_nos = sorted(cp_ts_dict.keys())

                for r in cp_rec_nos:
                    if cp_ts_dict[r]>timestamp: break

                # by now, r should be a record number that is just bigger
                #   than timestamp, or the last record in cp_rec_nos

                (_m, _d) = self.__read(r)
                assert _m["rec_no"] == r and _m["type"] == "checkpoint"
                # XXX: there is a corner case for timestamp assertion.
                # assert _m["timestamp"] > timestamp

                ts_dict = _m["ts_dict"]
                rec_nos = sorted(ts_dict.keys())

                for r in rec_nos:
                    if ts_dict[r]>timestamp: break

                # by now, r should be the desired record number 
                rec_no = max(r-1,0)
                break

            else:
                assert False

        # special case, when timestamp value is less than the oldest record
        if rec_no <= 0: return None

        for (metadata, data) in self.__record_iterator(rec_no): 
            if key in data:
                return (metadata["timestamp"], data[key])

        return None
 

            

    def __dumpall(self):
        """ dump the entire state (all kv pairs) as a single dictionary
            -- only the latest value of a key is returned    
            For internal use only
        """

        ret = {}
        for (metadata, data) in self.__record_iterator():
            for key in data.keys():
                if key not in ret:  # Can't just not include the 'None; keys
                    ret[key] = data[key]

        # Let's not return the keys that have 'None'
        for k in ret.keys():
            if ret[k] is None: ret.pop(k)

        return ret

    def keys(self):
        """ return all the keys as a single list """
        return self.__dumpall().keys()


    def values(self):
        """  return all the values as a single list """
        return self.__dumpall().values()


    def __iter__(self):
        """ Returns the keys """

        # TODO: think of a better implementation
        d = self.__dumpall()
        for k in d.keys():
            yield k

    def __contains__(self, item):
        """ check whether item exists in key-value store """

        # TODO: think of a better implementation
        d = self.__dumpall()
        return (item in d)

    def __len__(self):
        """ returns the length of key-value store """

        # TODO: think of a better implementation
        d = self.__dumpall()
        return (len(d))


    def __do_checkpoint(self):
        """Perform a checkpoint at certain level. 0 means full copy, anything
        above 0 includes data since last checkpoint of a lower level."""

        level = 9
        newmetadata = {}
        newdata = {}
        upper = cur = self.__num_records
        most_recent_cp = False
        cp_ts_dict = {}
        ts_dict = {}

        while cur>0:

            rec = self.__read(cur)
            (metadata, data) = rec
            assert metadata["rec_no"] == cur

            if metadata["type"] == "regular":           # a regular record
                for key in data:             # recent data takes precedence
                    if key not in newdata: newdata[key] = data[key]
                ts_dict[cur] = metadata["timestamp"]
                cur = cur-1
                continue 

            elif metadata["type"] == "checkpoint":      # checkpoint record
                assert metadata["cp_range"][1] == cur-1     # sanity check
                assert 0<=metadata["cp_level"]<=9

                # Just to copy the timestamp dictionary from just the most
                #   recent checkpoint
                if most_recent_cp==False:
                    cp_ts_dict = metadata["cp_ts_dict"]
                    most_recent_cp = True

                if metadata["cp_level"]<level:  # this is where we could stop

                    # but let's check if there is any merit in bumping 
                    #   the level down by combining the two checkpoints
                    ok = data.keys()            # ok => old keys
                    nk = newdata.keys()         # nk => new keys

                    # condition when merging should be performed:
                    len_X = len(set(ok) & set(nk))
                    min_len = min(len(ok), len(nk))
                    if (len_X > self.__cp_X_factor*min_len) or \
                            (len(set(nk))>self.__cp_size_factor*len(set(ok))):

                        # YES, let's merge
                        level = metadata["cp_level"]    # bump level down

                        data.update(newdata)    # dictionary update
                        newdata = data
                        cur = metadata["cp_range"][0]-1
                        continue

                    else:
                        level = metadata["cp_level"]+1
                        break

                else:
                    data.update(newdata)
                    newdata = data
                    cur = metadata["cp_range"][0]-1     # skip range covered
                    continue

            else:                           # unknown type
                assert False

        lower = cur+1       # out of loop because either cur==0, or break

        if lower==1: level = 0          # Makes sense, doesn't it?
        cur_time = time.time()
        cp_ts_dict[upper+1] = cur_time

        newmetadata["type"] = "checkpoint"
        newmetadata["timestamp"] = cur_time
        newmetadata["rec_no"] = self.__num_records+1
        newmetadata["cp_range"] = (lower, upper)
        newmetadata["cp_level"] = level
        newmetadata["cp_ts_dict"] = cp_ts_dict
        newmetadata["ts_dict"] = ts_dict

        newrec = (newmetadata, newdata)
        self.__append(newrec)



def __selftest(logname):
    """ 
    Perform a simple self-test. Creates a new Key Value store based
        on 'logname'.
    """

    N = 100
    kv = KVstore(logname, mode=KVstore.MODE_RW)       # create a kvstore

    tmp = {}
    for idx in xrange(N):       # write N keys
        kv[idx] = idx*idx
        tmp[idx] = idx*idx

    assert len(kv) == N
    for idx in xrange(N):
        assert idx in kv

    # get all the keys, and compare if they match
    for k in kv:                # tests the iteration
        assert k in tmp
        assert tmp[k] == kv[k]
        tmp.pop(k)              # make sure we don't have duplicates
    assert len(tmp)==0

    print "Passed selftest"


if __name__=="__main__":
    import sys
    if len(sys.argv)<2:
        print "Usage: %s <logname>" % sys.argv[0]
        sys.exit(-1)
    __selftest(sys.argv[1])
