/*
NOTE: This code requires the Apache XML-RPC library.
You need to have the following jars in your classpath:
- xmlrpc-common-3.1.3.jar
- xmlrpc-server-3.1.3.jar
- ws-commons-util-1.0.2.jar
- xmlrpc-client-3.1.3.jar (for the test client)

You cannot run this as a single source file without these external dependencies.
To compile and run from the command line (assuming jars are in a 'lib' folder):
javac -cp "lib/*" Task111.java
java -cp ".:lib/*" Task111
*/
import org.apache.xmlrpc.server.PropertyHandlerMapping;
import org.apache.xmlrpc.server.XmlRpcServer;
import org.apache.xmlrpc.server.XmlRpcServerConfigImpl;
import org.apache.xmlrpc.webserver.WebServer;
import org.apache.xmlrpc.client.XmlRpcClient;
import org.apache.xmlrpc.client.XmlRpcClientConfigImpl;
import java.net.URL;

public class Task111 {

    // This is the class with methods that will be exposed by the server.
    // It must be public, and its methods must be public.
    public static class Calculator {
        public int add(int x, int y) {
            return x + y;
        }

        public int subtract(int x, int y) {
            return x - y;
        }
    }

    public static void main(String[] args) {
        final int PORT = 8080;
        WebServer webServer = null;

        try {
            System.out.println("Attempting to start XML-RPC server on port " + PORT + "...");
            webServer = new WebServer(PORT);

            XmlRpcServer xmlRpcServer = webServer.getXmlRpcServer();
            PropertyHandlerMapping phm = new PropertyHandlerMapping();

            // Register our Calculator class with the name "calc"
            phm.addHandler("calc", Calculator.class);
            xmlRpcServer.setHandlerMapping(phm);

            XmlRpcServerConfigImpl serverConfig = (XmlRpcServerConfigImpl) xmlRpcServer.getConfig();
            serverConfig.setEnabledForExtensions(true);
            serverConfig.setContentLengthOptional(false);

            // Start the server in a separate thread to not block the main thread
            Thread serverThread = new Thread(() -> {
                try {
                    webServer.start();
                    System.out.println("Server started successfully.");
                } catch (Exception e) {
                    System.err.println("Server thread error: " + e.getMessage());
                }
            });
            serverThread.start();
            
            // Give the server a moment to start up
            Thread.sleep(1000);

            // --- Client Test Cases ---
            System.out.println("\n--- Running Client Test Cases ---");
            runTestClient(PORT);

        } catch (Exception e) {
            System.err.println("Server setup failed: " + e.getMessage());
            e.printStackTrace();
        } finally {
            if (webServer != null) {
                System.out.println("Shutting down the server.");
                webServer.shutdown();
            }
        }
    }

    public static void runTestClient(int port) {
        try {
            XmlRpcClientConfigImpl config = new XmlRpcClientConfigImpl();
            config.setServerURL(new URL("http://127.0.0.1:" + port + "/xmlrpc"));
            XmlRpcClient client = new XmlRpcClient();
            client.setConfig(config);

            // Test Case 1
            Object[] params1 = new Object[]{5, 3};
            Integer result1 = (Integer) client.execute("calc.add", params1);
            System.out.println("Test 1: calc.add(5, 3) = " + result1);

            // Test Case 2
            Object[] params2 = new Object[]{-10, 5};
            Integer result2 = (Integer) client.execute("calc.add", params2);
            System.out.println("Test 2: calc.add(-10, 5) = " + result2);

            // Test Case 3
            Object[] params3 = new Object[]{0, 0};
            Integer result3 = (Integer) client.execute("calc.add", params3);
            System.out.println("Test 3: calc.add(0, 0) = " + result3);

            // Test Case 4
            Object[] params4 = new Object[]{12345, 67890};
            Integer result4 = (Integer) client.execute("calc.add", params4);
            System.out.println("Test 4: calc.add(12345, 67890) = " + result4);

            // Test Case 5
            Object[] params5 = new Object[]{10, 4};
            Integer result5 = (Integer) client.execute("calc.subtract", params5);
            System.out.println("Test 5: calc.subtract(10, 4) = " + result5);

        } catch (Exception e) {
            System.err.println("Client test failed: " + e.getMessage());
        }
    }
}