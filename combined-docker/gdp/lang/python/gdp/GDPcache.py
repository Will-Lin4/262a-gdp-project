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

import gdp
import time

class GDPcache:
    """
    A caching + query-by-time layer for GDP. We don't need a lock here,
        since all our opreations are atomic (at least for now)

	Example Usage (just for reference):

    ```
        from GDPcache import GDPcache
        import time

        logname = "edu.berkeley.eecs.swarmlab.device.c098e5300003"
        c = GDPcache(logname)
        cur_time = time.time()      # to get a time reference

        # A single record that's one day old
        singleRecord = c.get(cur_time-86400.0)

        # Records from a *sampled* time range
        rangeRecord = c.getRange(cur_time-86400.0, cur_time-86400.0+60.0)

        print singleRecord, rangeRecord
    ```
    """

    def __init__(self, logname, limit=10000):
        """
        Initialize with just the log name, and optionally cache size
        
        limit is the number of records to keep in the cache. This is a soft
        limit, which means that we will go over the limit on various
        occasions, but we will try to be within a certain factor of the
        specified limit (by default, 2). This enables us to minimize the
        cleanup overhead.
        """

        gdp.gdp_init()      # No side-effects of calling this multiple times
        # gdp.dbg_set("*=20")
        self.logname = logname
        self.lh = gdp.GDP_GIN(gdp.GDP_NAME(logname), gdp.GDP_MODE_RO)
        self.limit = limit
        self.cache = {}     # recno => record cache   (limited size)
        self.atime = {}     # recno => time of access (same size as cache)

        ## populate the limits
        self.leastRecent()
        self.mostRecent()

    def __cleanup(self):
        """
        Make sure that we adhere to the size limit on cache. However, as
        an optimization, we never delete the smallest and the largest
        record number we know of. In addition, the limit is a *soft*
        limit, meaning that we avoid doing cleanup as soon as we are
        just one above the limit, for example.
        """

        # A quick return for the case we don't need cleanup for
        if len(self.cache)<=2*self.limit: return

        min_recno = min(self.cache.keys())
        max_recno = max(self.cache.keys())

        # get an ordered list of keys based on inverse LRU
        iLRUorder = sorted(self.cache.keys(), key=lambda x:self.atime[x],
                                             reverse=True )
        if min_recno in iLRUorder: iLRUorder.remove(min_recno)
        if max_recno in iLRUorder: iLRUorder.remove(max_recno)

        while len(self.cache)>self.limit:
            # the last item in the reverse sorted list (least recently used)
            lru = iLRUorder.pop()
            self.cache.pop(lru)
            self.atime.pop(lru)

    
    def __time(self, datum):        # cache for tMap
        """ give us the time function. A way to switch between the log-server
            timestamps and the timestamps in data """
        assert isinstance(datum, gdp.GDP_DATUM)
        return datum['ts']['tv_sec'] + (datum['ts']['tv_nsec']*1.0/10**9)


    def __read(self, recno):        # cache for, of course, records
        """ read a single record by recno, but add to cache """

        ## return from cache, if we have it.
        if recno>0 and recno in self.cache.keys():
            self.atime[recno] = time.time()
            return self.cache[recno]

        try:
            datum = self.lh.read_by_recno(recno)
        except gdp.MISC.EP_STAT_SEV_ERROR as e:
            datum = None
            if "Berkeley:Swarm-GDP:404" not in e.msg:
                raise e

        ## add to the cache, even if we got a 404 not found.
        pos_recno = datum['recno'] if datum is not None else recno

        if pos_recno>0:     # add only positive recno to cache
            self.cache[pos_recno] = datum
            self.atime[pos_recno] = time.time()
            self.__cleanup()

        return datum


    def __multiread(self, start, num, step=1):
        """ same as read, but efficient for a range. Use carefully, because
        I don't check for already-cached entries """

        numRecords = 0
        usingMultiread = False
        if step == 1:
            self.lh.read_by_recno_async(start, num)
            usingMultiread = True
        else:
            # do lots of multireads of size 1
            for i in xrange(start, start+num, step):
                self.lh.read_by_recno_async(i)
                numRecords += 1
        ret = []
        while usingMultiread or numRecords>0:

            try:
                event = self.lh.get_next_event(None)
            except gdp.MISC.EP_STAT_SEV_ERROR as e:
                if "Berkeley:Swarm-GDP:404" in e.msg:
                    numRecords -= 1
                    continue

            if event['type'] == gdp.GDP_EVENT_DONE and usingMultiread:
                break

            if event["type"] not in [gdp.GDP_EVENT_DONE, gdp.GDP_EVENT_DATA]:
                print "Unknown event type", event

            numRecords -= 1
            datum = event['datum']
            recno = datum['recno']
            self.cache[recno] = datum
            self.atime[recno] = time.time()
            ret.append(datum)
        self.__cleanup()
        # Sort the keys. In case of async read, they can arrive out of order
        ret.sort(key=lambda datum: datum['recno'])
        return ret


    def __findRec(self, t):
        """ find the most recent record num before t, i.e. a binary search"""

        ## records
        first, last = self.leastRecent(), self.mostRecent()
        ## record numbers
        start, end = first['recno'], last['recno']

        # first check the obvious out of range condition
        if t<self.__time(first): return start
        if t>=self.__time(last): return end

        # t lies in range [start, end)
        while start < end-1:

            p = (start+end)/2
            rec = self.__read(p)
            if rec is not None:
                if t<self.__time(rec): end = p
                else: start = p
            else:
                # print "found a gap, searching boundaries"
                l = self.__find_left_gap_boundary(start, p)
                r = self.__find_right_gap_boundary(p, end)
                # print l, r

                lrec = self.__read(l-1)
                rrec = self.__read(r+1)
                # assert self.__time(lrec)<self.__time(rrec)

                if t < self.__time(lrec):
                    end = l-1
                else:
                    start = r+1

        return start


    def __find_left_gap_boundary(self, start, end):

        # assert self.__read(start) is not None
        # assert self.__read(end) is None

        while start<end-1:
            mid = (start+end)/2
            rec = self.__read(mid)
            if rec is None:
                end = mid
            else:
                start = mid

        # assert start == end-1
        # assert self.__read(start) is not None
        # assert self.__read(end) is None
        return end


    def __find_right_gap_boundary(self, start, end):

        # assert self.__read(start) is None
        # assert self.__read(end) is not None

        while start<end-1:
            mid = (start+end)/2
            rec = self.__read(mid)
            if rec is None:
                start = mid
            else:
                end = mid

        # assert start == end-1
        # assert self.__read(start) is None
        # assert self.__read(end) is not None
        return start



    def get(self, t):
        """ Get the record just before time t (using a binary search)"""
        recno = self.__findRec(t)
        return self.__read(recno)


    def getRange(self, tStart, tEnd, numPoints=1000):
        """ return a sampled list of records, *roughly* numPoints long """
        start = self.__findRec(tStart)+1
        end = self.__findRec(tEnd)

        if not (start<=end):
            return []
        # Calculate step size
        stepSize = max((end+1-start)/numPoints, 1)
        return self.__multiread(start, (end+1)-start, stepSize)

    def mostRecent(self):
        for idx in xrange(-1,-11,-1):
            rec = self.__read(idx)
            if rec is not None:
                return rec
        return None

    def leastRecent(self):
        for idx in xrange(1,11):
            rec = self.__read(idx)
            if rec is not None:
                return rec
        return None
