
import org.apache.xmlrpc.server.PropertyHandlerMapping;
import org.apache.xmlrpc.server.XmlRpcServer;
import org.apache.xmlrpc.server.XmlRpcServerConfigImpl;
import org.apache.xmlrpc.webserver.WebServer;

public class Task111 {
    
    public static class Calculator {
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
                throw new IllegalArgumentException("Division by zero");
            }
            return (double) a / b;
        }
        
        public String echo(String message) {
            return "Echo: " + message;
        }
    }
    
    public static void startServer(int port) {
        try {
            WebServer webServer = new WebServer(port);
            XmlRpcServer xmlRpcServer = webServer.getXmlRpcServer();
            
            PropertyHandlerMapping phm = new PropertyHandlerMapping();
            phm.addHandler("Calculator", Calculator.class);
            xmlRpcServer.setHandlerMapping(phm);
            
            XmlRpcServerConfigImpl serverConfig = (XmlRpcServerConfigImpl) xmlRpcServer.getConfig();
            serverConfig.setEnabledForExtensions(true);
            serverConfig.setContentLengthOptional(false);
            
            webServer.start();
            System.out.println("XML-RPC Server started on port " + port);
            
        } catch (Exception e) {
            System.err.println("Error starting server: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Starting XML-RPC Server Demo...");
        System.out.println("Note: This requires Apache XML-RPC library");
        System.out.println("Server would start on port 8080");
        
        // Test cases for Calculator methods
        Calculator calc = new Calculator();
        
        System.out.println("\\nTest Case 1 - Addition:");
        System.out.println("5 + 3 = " + calc.add(5, 3));
        
        System.out.println("\\nTest Case 2 - Subtraction:");
        System.out.println("10 - 4 = " + calc.subtract(10, 4));
        
        System.out.println("\\nTest Case 3 - Multiplication:");
        System.out.println("6 * 7 = " + calc.multiply(6, 7));
        
        System.out.println("\\nTest Case 4 - Division:");
        System.out.println("20 / 4 = " + calc.divide(20, 4));
        
        System.out.println("\\nTest Case 5 - Echo:");
        System.out.println(calc.echo("Hello World"));
        
        // Uncomment to actually start the server
        // startServer(8080);
    }
}
