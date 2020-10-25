package org.terraswarm.gdp.apps;

import org.terraswarm.gdp.*;
import java.util.*;
import java.text.SimpleDateFormat;

/**
 * A simple example to demonstrate and verify log creation
 * @author nitesh mor
 *
 */


public class CreateTest {

    public static void main(String[] args) throws GDPException{

        // Just to check any error output
        GDP.gdp_init();
        GDP.dbg_set("*=50");

        if (args.length<2) { // usage
            System.out.print("Usage: <logname> <creation service name>");
            System.out.println();
            return;
        }

        String logName = args[0];
        String creationServiceName = args[1];

        GDP_CREATE_INFO gci = new GDP_CREATE_INFO();
        // this should be set to something more sane
        gci.set_creator("GDPJavaInterface", "BERKELEY.EDU");
        gci.set_creation_service(creationServiceName);

        System.out.println("Creating log " + logName + 
                            " by contacting " + creationServiceName);

        GDP_GIN.create(gci, logName);
        return;
        
    }
}
