/*
 * This code requires the Apache XML-RPC library.
 * You need to have the following jars in your classpath:
 * - xmlrpc-common-3.1.3.jar
 * - xmlrpc-server-3.1.3.jar
 * - ws-commons-util-1.0.2.jar
 * - commons-logging-1.1.1.jar (or another logging implementation)
 *
 * You can download them from the Apache XML-RPC project website.
 *
 * To compile and run:
 * javac -cp ".;xmlrpc-common-3.1.3.jar;xmlrpc-server-3.1.3.jar;ws-commons-util-1.0.2.jar" Task111.java
 * java -cp ".;xmlrpc-common-3.1.3.jar;xmlrpc-server-3.1.3.jar;ws-commons-util-1.0.2.jar;commons-logging-1.1.1.jar" Task111
 */
import org.apache.xmlrpc.server.PropertyHandlerMapping;
import org.apache.xmlrpc.server.XmlRpcServer;
import org.apache.xmlrpc.server.XmlRpcServerConfigImpl;
import org.apache.xmlrpc.webserver.WebServer;
import java.util.HashMap;
import java.util.Map;

public class Task111 {

    // This class provides the methods that the XML-RPC server will expose.
    // Methods must be public.
    public static class RpcHandler {
        // Test Case 1: Add two integers
        public int add(int i1, int i2) {
            System.out.println("Called add(" + i1 + ", " + i2 + ")");
            return i1 + i2;
        }

        // Test Case 2: Subtract two doubles
        public double subtract(double d1, double d2) {
            System.out.println("Called subtract(" + d1 + ", " + d2 + ")");
            return d1 - d2;
        }

        // Test Case 3: Greet a user by name
        public String greet(String name) {
            System.out.println("Called greet(\"" + name + "\")");
            return "Hello, " + name + "!";
        }
        
        // Test Case 4: Return a struct (Map)
        public Map<String, Object> getStruct(String name, int age) {
             System.out.println("Called getStruct(\"" + name + "\", " + age + ")");
             HashMap<String, Object> result = new HashMap<>();
             result.put("name", name);
             result.put("age", age);
             result.put("isAdult", age >= 18);
             return result;
        }
        
        // Test Case 5: Return an array of strings
        public Object[] getArray() {
            System.out.println("Called getArray()");
            return new Object[] {"Java", "Python", "CPP", "C"};
        }
    }

    public static void main(String[] args) {
        try {
            int port = 8080;
            System.out.println("Starting XML-RPC server on port " + port + "...");

            WebServer webServer = new WebServer(port);
            XmlRpcServer xmlRpcServer = webServer.getXmlRpcServer();

            PropertyHandlerMapping phm = new PropertyHandlerMapping();
            
            // The key "handler" is what clients will use to call methods.
            // e.g., client.execute("handler.add", new Object[]{5, 3})
            phm.addHandler("handler", RpcHandler.class);

            xmlRpcServer.setHandlerMapping(phm);

            XmlRpcServerConfigImpl serverConfig =
                (XmlRpcServerConfigImpl) xmlRpcServer.getConfig();
            serverConfig.setEnabledForExtensions(true);
            serverConfig.setContentLengthOptional(false);

            webServer.start();
            System.out.println("Server started successfully.");
            System.out.println("Available methods: handler.add, handler.subtract, handler.greet, handler.getStruct, handler.getArray");

        } catch (Exception exception) {
            System.err.println("Java Server Error: " + exception);
        }
    }
}