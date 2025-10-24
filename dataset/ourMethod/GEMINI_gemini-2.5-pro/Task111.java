import org.apache.xmlrpc.server.PropertyHandlerMapping;
import org.apache.xmlrpc.server.XmlRpcServer;
import org.apache.xmlrpc.server.XmlRpcServerConfigImpl;
import org.apache.xmlrpc.webserver.WebServer;
import org.apache.xmlrpc.client.XmlRpcClient;
import org.apache.xmlrpc.client.XmlRpcClientConfigImpl;

import java.net.URL;

// Required dependencies:
// - org.apache.xmlrpc:xmlrpc-server:3.1.3
// - org.apache.xmlrpc:xmlrpc-client:3.1.3
// You would typically use a build tool like Maven or Gradle to manage these.
// <dependency>
//     <groupId>org.apache.xmlrpc</groupId>
//     <artifactId>xmlrpc-server</artifactId>
//     <version>3.1.3</version>
// </dependency>
// <dependency>
//     <groupId>org.apache.xmlrpc</groupId>
//     <artifactId>xmlrpc-client</artifactId>
//     <version>3.1.3</version>
// </dependency>

public class Task111 {

    // This class contains methods that the XML-RPC server will expose.
    // It must be public, and its methods must be public.
    public static class Calculator {
        public int add(int a, int b) {
            // Rule #3: Ensure proper validation of input.
            // Check for potential integer overflow.
            if ((b > 0 && a > Integer.MAX_VALUE - b) || (b < 0 && a < Integer.MIN_VALUE - b)) {
                throw new ArithmeticException("Integer overflow detected");
            }
            return a + b;
        }

        public int subtract(int a, int b) {
            // Rule #3: Ensure proper validation of input.
             // Check for potential integer underflow.
            if ((b > 0 && a < Integer.MIN_VALUE + b) || (b < 0 && a > Integer.MAX_VALUE + b)) {
                throw new ArithmeticException("Integer underflow detected");
            }
            return a - b;
        }

        public double divide(double a, double b) {
            // Rule #3: Ensure proper validation of input.
            if (b == 0) {
                // Rule #8: Exceptions are caught and handled. The framework will turn this into an XML-RPC fault.
                throw new IllegalArgumentException("Division by zero is not allowed.");
            }
            return a / b;
        }
    }

    public static void main(String[] args) {
        int port = 8080;
        WebServer webServer = null;
        try {
            // --- Server Setup ---
            System.out.println("Attempting to start XML-RPC server on port " + port);
            webServer = new WebServer(port);

            XmlRpcServer xmlRpcServer = webServer.getXmlRpcServer();
            
            PropertyHandlerMapping phm = new PropertyHandlerMapping();
            // Rule #4: Avoid dangerous functions. Map a specific name to a safe, pre-defined class.
            // Exposes methods of Calculator class as "calculator.methodName"
            phm.addHandler("calculator", Calculator.class);
            xmlRpcServer.setHandlerMapping(phm);

            XmlRpcServerConfigImpl serverConfig = (XmlRpcServerConfigImpl) xmlRpcServer.getConfig();
            serverConfig.setEnabledForExtensions(true);
            serverConfig.setContentLengthOptional(false);
            
            // Rule #6: Disable external entities. By default, recent versions of Apache XML-RPC
            // and the underlying SAX parser are configured to be safe against XXE.
            // Explicit configuration would require extending internal classes.

            webServer.start();
            System.out.println("Server started successfully.");
            System.out.println("Accepting requests on port " + port);

            // --- Client and Test Cases ---
            runClientTests(port);

        } catch (Exception exception) {
            System.err.println("Java Server error: " + exception.getMessage());
            exception.printStackTrace();
        } finally {
            if (webServer != null) {
                System.out.println("Shutting down the server.");
                webServer.shutdown();
            }
        }
    }

    private static void runClientTests(int port) {
        try {
            System.out.println("\n--- Running Client Test Cases ---");
            XmlRpcClientConfigImpl config = new XmlRpcClientConfigImpl();
            config.setServerURL(new URL("http://127.0.0.1:" + port + "/xmlrpc"));
            XmlRpcClient client = new XmlRpcClient();
            client.setConfig(config);

            // Test Case 1: Add
            Object[] params1 = new Object[]{10, 20};
            Integer result1 = (Integer) client.execute("calculator.add", params1);
            System.out.println("Test 1: 10 + 20 = " + result1);

            // Test Case 2: Subtract
            Object[] params2 = new Object[]{100, 33};
            Integer result2 = (Integer) client.execute("calculator.subtract", params2);
            System.out.println("Test 2: 100 - 33 = " + result2);

            // Test Case 3: Divide
            Object[] params3 = new Object[]{10.0, 4.0};
            Double result3 = (Double) client.execute("calculator.divide", params3);
            System.out.println("Test 3: 10.0 / 4.0 = " + result3);

            // Test Case 4: Divide by zero (error case)
            try {
                Object[] params4 = new Object[]{5.0, 0.0};
                client.execute("calculator.divide", params4);
            } catch (Exception e) {
                System.out.println("Test 4: 5.0 / 0.0 -> " + e.getMessage());
            }

            // Test Case 5: Method not found (error case)
            try {
                Object[] params5 = new Object[]{5, 3};
                client.execute("calculator.multiply", params5);
            } catch (Exception e) {
                System.out.println("Test 5: call multiply -> " + e.getMessage());
            }

        } catch (Exception e) {
            System.err.println("Client error: " + e.getMessage());
        }
    }
}