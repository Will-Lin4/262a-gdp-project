package org.terraswarm.gdp.apps;

import org.terraswarm.gdp.*;
import java.util.*;
import java.text.SimpleDateFormat;

import com.sun.jna.Memory;
import com.sun.jna.Pointer;
import com.sun.jna.ptr.PointerByReference;
import org.terraswarm.gdp.NativeSize;

/**
 * A simple example to demonstrate and verify log creation
 * @author nitesh mor
 *
 */


public class CreateTestWithKey {

    public static void main(String[] args) throws GDPException{

        // Just to check any error output
        GDP.gdp_init();
        GDP.dbg_set("*=10");

        if (args.length<3) { // usage
            System.out.print("Usage: <logname> <keyfile-name> <creation service name>");
            System.out.println();
            return;
        }

        String logName = args[0];
        String keyfile = args[1];
        String creationServiceName = args[2];

        GDP_CREATE_INFO gci = new GDP_CREATE_INFO();
        gci.set_creation_service(creationServiceName);

        // Not sure I understand keyform quite yet. My guess is that it's DER vs PEM specification
        EP_CRYPTO_KEY k = new EP_CRYPTO_KEY(keyfile, 0, 0);

        gci.set_owner_key(k, "sha256");
        gci.set_writer_key(k, "sha256"); 

        System.out.println("Creating log " + args[0]);
        
        GDP_GIN.create(gci, logName);
        return;
        
    }
}
