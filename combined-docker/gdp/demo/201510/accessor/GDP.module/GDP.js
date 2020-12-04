var GDPHelper = Java.type('ptolemy.actor.lib.jjs.modules.GDP.GDPHelper');

exports.GDP = function(name, iomode) {
    this.helper = new GDPHelper(name, iomode);
    return this.helper;
}

exports.GDP.read = function(recno) {
    var data = this.helper.read(recno);
    return data;
}

exports.GDP.append = function(data) {
    this.helper.append(data);
}

exports.GDP.subscribe = function(startrec, numrecs) {
    this.helper.subscribe(startrec, numrecs);
}

exports.GDP.get_next_data = function(timeout_msec) {
    return this.helper.get_next_data(timeout_msec);
}
