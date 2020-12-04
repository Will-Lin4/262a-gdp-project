#!/usr/bin/env python

import matplotlib.pyplot as plt
import matplotlib.dates as dates
import numpy as np
import pytz
from KVstore import KVstore
import time
import sys
import os
import ConfigParser

def generate_plot(kvstore, key, parser, ylabel, ylim, title, tlist, outfile):
    """
    Create a plot for a given key from a key-value store.
        This takes care of fetching data from key value store and invoking
        appropriate matplotlib commands

    *** This is where you would make plots prettier ***

    Arguments:
    kvstore - Key value store instance. (READ ONLY)
    key     - Key that we are interested in
    parser  - a function that takes in the value and returns a float
              that could be plotted on a graph.
              The parser function should not need to worry about 'None'.
    ylabel  - string that should be used for y-axis label. e.g. "lux"
    ylim    - range of y values. Makes graphs a little more sensible.
              e.g. [0,70]
    title   - A title for the plot. We append creation time by default
    tlist   - a list describing sample times relative to current time (in s)
              ex: [-86400, -43200, -21600, -3600, -1800, -900, -300]
    outfile - where should the output image be stored?
    """

    # if the timestamp on the data value from key-value store is off by 
    #   more than the following parameter, we ignore it.
    # This makes sure we don't confuse missing data with constant value
    ACCURACY = 30.0

    cur = time.time()
    x, y, xdates = [], [], []
    for t in tlist:
        tmp = kvstore.get(key, cur+t)
        x.append(t)
        xdates.append(dates.epoch2num(cur+t))
        if tmp is not None and (cur+t)-tmp[0]<=ACCURACY:
            y.append(parser(tmp))
        else:
            y.append(np.nan)

    if y == [np.nan]*len(y): 
        print "Skipping plotting, because no data. Check consistency."
        return

    plt.figure(outfile, figsize=(8,4))

    # Remove the plot frame lines. They are unnecessary chartjunk.    
    ax = plt.subplot(111)    
    ax.spines["top"].set_visible(False)    
    ax.spines["right"].set_visible(False)    

    # Ensure that axis ticks only show up on the bottom and left of the plot.    
    # Ticks on right and top of the plot are generally unnecessary chartjunk.    
    ax.get_xaxis().tick_bottom()    
    ax.get_yaxis().tick_left()   

    ax.grid(True)

    timezone = pytz.timezone('US/Pacific')

    # Fix up our x axis
    hfmt = dates.DateFormatter('%H:%M', tz=timezone)
    ax.xaxis.set_major_locator(dates.HourLocator())
    ax.xaxis.set_major_formatter(hfmt)
    ax.xaxis.set_minor_locator(dates.MinuteLocator())

    # plt.plot(x,y,'.--')
    ax.plot_date(xdates, y, '.--')

    # Also make sure that we plot till the current time
    ax.set_xlim(xmax=dates.epoch2num(cur))

    cur_string = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(cur))
    plt.title("%s (%s)" % (title, cur_string))
    plt.ylabel(ylabel)
    plt.ylim(ylim)
    plt.savefig(outfile)


def main(configfile):
    """
    Read the configuration file and call appropriate generate_plot. Also
        takes care of autogenerating html pages

    *** This is where you will make the HTML pages look prettier ***
    """


    def _create_elem(figname):
        """ create a list entry for a figure """
        return "<li> <img src=%s> </li>\n" % figname


    config = ConfigParser.ConfigParser()
    config.read(configfile)

    # create the top level directory. XXX: This should be a parameter
    try: os.mkdir("webview")
    except OSError: pass


    # FIXME: This should ideally be read from the configuration file as 
    #   well. We should ideally be making multiple plots. But that's for
    #   later.
    tlist = range(-12*3600, 0, 300)

    # Each tuple is:
    #   (parser_function, measurement_unit, ylimit, title)
    sensor_dict = {
        "optical"    : (lambda (t,x):x, "lux", [0,200],
                                    "Optical flux"),
        "temperature": (lambda (t,(x,y)):x, u'\u2103', [0,40], 
                                    "Ambient temperature"),
        "humidity"   : (lambda (t,(x,y)):y, "%", [0,120],
                                    "Relative humidity"),
        "barometer"  : (lambda (t,(x,y)):y, "mbar", [0,1200],
                                    "Barometric pressure")
    }

    index_html = """<!DOCTYPE html> <html> <body> <ul>\n"""

    sensors = config.sections()
    for sensor in sensors:

        index_html += ("<li><a href='%s.html'>%s</a></li>\n" % (sensor, sensor))

        # sensor_html is where we keep the string version of webpage
        sensor_html = """<!DOCTYPE html> <html> <body>\n<ul>\n"""

        kv = KVstore(config.get(sensor, "kvstore"))

        for s in sensor_dict.keys():

            tup = sensor_dict[s]
            if config.has_option(sensor, s):
                figname = "%s_%s.png" % (sensor, s)
                generate_plot(kv, s, tup[0], tup[1], tup[2], tup[3], 
                                            tlist, "webview/%s"%figname)
                sensor_html += _create_elem(figname)

        sensor_html += "</ul>\n</body></html>"

        # Write the actual html page
        with open("webview/"+sensor+".html", "w") as fh:
            fh.write(sensor_html)

    index_html += "</ul>\n</body></html>"

    with open("webview/index.html", "w") as fh:
        fh.write(index_html)


if __name__=="__main__":

    if len(sys.argv)<2:
        print "Usage: %s <config-file>" % sys.argv[0]
        sys.exit(1)

    main(sys.argv[1])
