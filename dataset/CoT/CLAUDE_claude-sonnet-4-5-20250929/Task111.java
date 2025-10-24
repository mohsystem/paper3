
import org.apache.xmlrpc.server.PropertyHandlerMapping;
import org.apache.xmlrpc.server.XmlRpcServer;
import org.apache.xmlrpc.server.XmlRpcServerConfigImpl;
import org.apache.xmlrpc.webserver.WebServer;

public class Task111 {
    
    // Handler class for XML-RPC methods
    public static class MathHandler {
        public int add(int a, int b) {
            return a + b;
        }
        
        public int subtract(int a, int b) {
            return a - b;
        }
        
        public int multiply(int a, int b) {
            return a * b;
        }
        
        public double divide(int a, int b) {
            if (b == 0) {
                throw new IllegalArgumentException("Division by zero not allowed");
            }
            return (double) a / b;
        }
        
        public String echo(String message) {
            if (message == null || message.isEmpty()) {
                return "Empty message";
            }
            // Sanitize input to prevent injection attacks
            String sanitized = message.replaceAll("[<>&\\"']", "");
            return "Echo: " + sanitized;
        }
    }
    
    public static void startServer(int port) {
        try {
            // Create web server with specified port
            WebServer webServer = new WebServer(port);
            
            // Get XML-RPC server
            XmlRpcServer xmlRpcServer = webServer.getXmlRpcServer();
            
            // Configure server - disable extensions for security
            XmlRpcServerConfigImpl serverConfig = (XmlRpcServerConfigImpl) xmlRpcServer.getConfig();
            serverConfig.setEnabledForExtensions(false);
            serverConfig.setContentLengthOptional(false);
            
            // Register handler mapping
            PropertyHandlerMapping phm = new PropertyHandlerMapping();
            phm.addHandler("Math", MathHandler.class);
            xmlRpcServer.setHandlerMapping(phm);
            
            // Start server
            webServer.start();
            System.out.println("XML-RPC Server started on port " + port);
            
        } catch (Exception e) {
            System.err.println("Server error: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Starting XML-RPC Server Tests...");
        
        // Test cases demonstrating handler methods
        MathHandler handler = new MathHandler();
        
        System.out.println("\\n=== Test Case 1: Addition ===");
        System.out.println("add(10, 5) = " + handler.add(10, 5));
        
        System.out.println("\\n=== Test Case 2: Subtraction ===");
        System.out.println("subtract(20, 8) = " + handler.subtract(20, 8));
        
        System.out.println("\\n=== Test Case 3: Multiplication ===");
        System.out.println("multiply(7, 6) = " + handler.multiply(7, 6));
        
        System.out.println("\\n=== Test Case 4: Division ===");
        System.out.println("divide(100, 4) = " + handler.divide(100, 4));
        
        System.out.println("\\n=== Test Case 5: Echo with sanitization ===");
        System.out.println("echo('Hello World') = " + handler.echo("Hello World"));
        System.out.println("echo('<script>alert(1)</script>') = " + handler.echo("<script>alert(1)</script>"));
        
        // Start server on port 8080
        // Uncomment to run actual server:
        // startServer(8080);
    }
}
