/* An exception for the Global Data Plane (GDP).

   Copyright (c) 2016 The Regents of the University of California.
   All rights reserved.
   Permission is hereby granted, without written agreement and without
   license or royalty fees, to use, copy, modify, and distribute this
   software and its documentation for any purpose, provided that the above
   copyright notice and the following two paragraphs appear in all copies
   of this software.

   IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY
   FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
   ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
   THE UNIVERSITY OF CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF
   SUCH DAMAGE.

   THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
   INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE
   PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
   CALIFORNIA HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
   ENHANCEMENTS, OR MODIFICATIONS.

   PT_COPYRIGHT_VERSION_2
   COPYRIGHTENDKEY

 */

package org.terraswarm.gdp; 

/** 
 * An exception for the Global Data Plane (GDP).
 * @author Christopher Brooks
 */
public class GDPException extends Exception {

    /** Construct an exception for the Global Data Plane.
     */
    public GDPException() {
        super();
    }

    /** Construct an exception for the Global Data Plane.
     *  @param message The detail message.
     */
    public GDPException(String message) {
        super(message);
    }

    /** Construct an exception for the Global Data Plane.
     *  @param message The detail message.
     *  @param estat The status of the system
     */
    public GDPException(String message, EP_STAT estat) {
        super(message +
              "(code: " + estat.code + ": " + GDP.ep_stat_toStr(estat) + ")"); 
    }

    /** Construct an exception for the Global Data Plane.
     *  @param message The detail message.
     *  @param cause The exception that caused this exception.
     */
    public GDPException(String message, Throwable cause) {
        super(message, cause);
    }

    /** Construct an exception for the Global Data Plane.
     *  @param message The detail message.
     *  @param cause The exception that caused this exception.
     *  @param estat The status of the system
     */
    public GDPException(String message, Throwable cause, EP_STAT estat) {
        super(message +
              "(code: " + estat.code + ": " + GDP.ep_stat_toStr(estat) + ")",
              cause);
    }
}
