package org.terraswarm.gdp.apps;

import org.terraswarm.gdp.*;

/**
 * A simple example to demonstrate and verify GDP_NAME
 * @author nitesh mor
 *
 */


public class NameTest {

    public static void main(String[] args) throws GDPException {

        // Just to check any error output
        GDP.dbg_set("*=10");

        // the human readable name that we will convert
        String name = "HelloWorld";
        
        // Create an object of type GDP_NAME
        GDP_NAME n = new GDP_NAME(name);
        
        byte[] in = n.internal_name();
        byte[] pn = n.printable_name();

        print_internal(in);
        
        // now check whether we can regenerate the same name using
        // the printable name and/or internal name.
        
        GDP_NAME n1 = new GDP_NAME(new String(pn));
        print_internal(n1.internal_name());

        
        GDP_NAME n2 = new GDP_NAME(in);
        print_internal(n2.internal_name());
    
        
    }
    
    /**
     * print the internal name in a little pretty format
     * @param name
     */
    private static void print_internal(byte[] name) {

        System.out.print("Internal name: ");
        for (byte x: name) {
            System.out.print(Integer.toString((int) (x&0xFF)) + " ");
        }
        System.out.println();
    }
}
