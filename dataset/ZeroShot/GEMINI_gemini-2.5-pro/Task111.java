/*
 * To compile and run this code, you need the Apache XML-RPC libraries.
 * If you are using Maven, add these dependencies to your pom.xml:
 *
 * <dependency>
 *     <groupId>org.apache.xmlrpc</groupId>
 *     <artifactId>xmlrpc-server</artifactId>
 *     <version>3.1.3</version>
 * </dependency>
 * <dependency>
 *     <groupId>org.apache.xmlrpc</groupId>
 *     <artifactId>xmlrpc-common</artifactId>
 *     <version>3.1.3</version>
 * </dependency>
 *
 * Then compile and run:
 * javac Task111.java
 * java -cp ".:/path/to/xmlrpc-server-3.1.3.jar:/path/to/xmlrpc-common-3.1.3.jar:/path/to/ws-commons-util-1.0.2.jar" Task111
 */
import org.apache.xmlrpc.server.PropertyHandlerMapping;
import org.apache.xmlrpc.server.XmlRpcServer;
import org.apache.xmlrpc.server.XmlRpcServerConfigImpl;
import org.apache.xmlrpc.webserver.WebServer;

public class Task111 {

    /**
     * This class contains the methods that the XML-RPC server will expose.
     * It includes basic input validation for security.
     */
    public static class SecureHandler {
        
        public int add(int i1, int i2) {
            // Secure: Prevent integer overflow
            if ((i2 > 0 && i1 > Integer.MAX_VALUE - i2) || (i2 < 0 && i1 < Integer.MIN_VALUE - i2)) {
                throw new ArithmeticException("Integer overflow detected in add operation.");
            }
            return i1 + i2;
        }

        public int subtract(int i1, int i2) {
            // Secure: Prevent integer overflow/underflow
            if ((i2 > 0 && i1 < Integer.MIN_VALUE + i2) || (i2 < 0 && i1 > Integer.MAX_VALUE + i2)) {
                throw new ArithmeticException("Integer overflow detected in subtract operation.");
            }
            return i1 - i2;
        }

        public double divide(double d1, double d2) {
            // Secure: Prevent division by zero
            if (d2 == 0) {
                throw new IllegalArgumentException("Division by zero is not allowed.");
            }
            return d1 / d2;
        }
    }

    public static void main(String[] args) {
        try {
            int port = 8080;
            System.out.println("Attempting to start XML-RPC Server on port " + port);

            // The web server that will host the XML-RPC server
            WebServer webServer = new WebServer(port);
            
            // The XML-RPC server itself
            XmlRpcServer xmlRpcServer = webServer.getXmlRpcServer();
            
            // A mapping that tells the server how to find handlers
            PropertyHandlerMapping phm = new PropertyHandlerMapping();
            
            // Register our handler class. The first argument is the "name" clients will use.
            phm.addHandler("handler", SecureHandler.class);
            xmlRpcServer.setHandlerMapping(phm);
          
            // Configure the server
            XmlRpcServerConfigImpl serverConfig = (XmlRpcServerConfigImpl) xmlRpcServer.getConfig();
            serverConfig.setEnabledForExtensions(true);
            serverConfig.setContentLengthOptional(false);

            // Start the server
            webServer.start();
            
            System.out.println("Server started successfully.");
            System.out.println("Accepting requests. (Press Ctrl-C to stop)");
            
            System.out.println("\n--- Test Cases (run from a separate client) ---");
            System.out.println("You can use a Python client for testing:");
            System.out.println("----------------------------------------------");
            System.out.println("import xmlrpc.client");
            System.out.println("s = xmlrpc.client.ServerProxy('http://localhost:8080')");
            System.out.println("# Test Case 1: Add");
            System.out.println("print(s.handler.add(5, 3))");
            System.out.println("# Test Case 2: Subtract");
            System.out.println("print(s.handler.subtract(10, 4))");
            System.out.println("# Test Case 3: Divide");
            System.out.println("print(s.handler.divide(20.0, 5.0))");
            System.out.println("# Test Case 4: Divide by zero (error handling)");
            System.out.println("try:\n    print(s.handler.divide(10.0, 0.0))\nexcept Exception as e:\n    print(e)");
            System.out.println("# Test Case 5: Integer overflow (error handling)");
            System.out.println("try:\n    print(s.handler.add(2147483647, 1))\nexcept Exception as e:\n    print(e)");
            System.out.println("----------------------------------------------");

        } catch (Exception e) {
            System.err.println("Server exception: " + e);
            e.printStackTrace();
        }
    }
}