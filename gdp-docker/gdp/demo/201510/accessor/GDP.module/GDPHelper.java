package ptolemy.actor.lib.jjs.modules.GDP;

import org.terraswarm.gdp.GDP_GCL;

public class GDPHelper {

    private GDP_GCL g;

    public GDPHelper(String logname, int iomode) {
        this.g = new GDP_GCL(logname, iomode);        
    }

    public String read(long recno) {
        return this.g.read(recno);
    }

    public void append(String data) {
        this.g.append(data);
    }

    public void subscribe(long startrec, int numrec) {
        this.g.subscribe(startrec, numrec);
    }

    public String get_next_data(int timeout_msec) {
        return this.g.get_next_data(timeout_msec);
    }
}
