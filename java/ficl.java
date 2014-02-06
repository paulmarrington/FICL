import usdlc.FICL;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

public class ficl {
  public static void main(String[] args) {
    try {
      final FICL ficl = new FICL();
      // Called to exit the REPL
      ficl.extend("_exit_", new Runnable() {
        public void run() {
          System.out.println("FICL repl exiting");
          System.exit(ficl.popInt());
        }
      });
      // Used by uSDLC2 to end an instrumentation session
      ficl.extend("_end_", new Runnable() {
        public void run() {
          System.out.println("Errors: " + errors);
          errors = 0;
        }
      });
      // Used by uSDLC2 to check results of a test
      ficl.extend("expected", new Runnable() {
        public void run() {
          Object expected = ficl.pop();
          Object result = ficl.pop();
          if (!result.equals(expected)) {
            System.out.println(
              "\nError: expected '" +
                expected + "', found '" +
                result + "'");
            errors++;
          }
        }
      });
      // REPL
      BufferedReader bufferRead = new BufferedReader(
        new InputStreamReader(System.in));
      while (true) {
        // *R*ead
        String s = bufferRead.readLine();
        if (s == null) {
          System.out.println(
            "FICL repl input closed");
          System.exit(0);
        }
        if (s.length() > 0 && s.charAt(0) != '_') {
          System.out.print(s);
        }
        // *E*xecute
        ficl.run(s);
        if (ficl.context.abort) {
          System.out.println("\nError: " +
            ficl.context.errors.toString());
          errors++;
        } else {
          // *P*rint
          System.out.println(ficl.toString());
        }
        System.out.flush();
        System.err.flush();
        // *L*oop
      }
    } catch (IOException e) {
      System.out.println(e.toString());
      e.printStackTrace();
    }
  }

  public static int errors = 0;
}
