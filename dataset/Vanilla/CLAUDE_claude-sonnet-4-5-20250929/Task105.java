
import javax.net.ssl.*;
import java.io.*;
import java.security.cert.X509Certificate;

public class Task105 {
    public static SSLSocket createSecureConnection(String host, int port) throws Exception {
        SSLSocketFactory factory = (SSLSocketFactory) SSLSocketFactory.getDefault();
        SSLSocket socket = (SSLSocket) factory.createSocket(host, port);
        socket.startHandshake();
        return socket;
    }
    
    public static String sendHttpsRequest(String host, int port, String request) throws Exception {
        SSLSocket socket = createSecureConnection(host, port);
        
        PrintWriter out = new PrintWriter(new OutputStreamWriter(socket.getOutputStream()));
        BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
        
        out.println(request);
        out.println("Host: " + host);
        out.println("Connection: close");
        out.println();
        out.flush();
        
        StringBuilder response = new StringBuilder();
        String line;
        int lineCount = 0;
        while ((line = in.readLine()) != null && lineCount < 20) {
            response.append(line).append("\\n");
            lineCount++;
        }
        
        in.close();
        out.close();
        socket.close();
        
        return response.toString();
    }
    
    public static SSLServerSocket createSecureServer(int port) throws Exception {
        SSLServerSocketFactory factory = (SSLServerSocketFactory) SSLServerSocketFactory.getDefault();
        SSLServerSocket serverSocket = (SSLServerSocket) factory.createServerSocket(port);
        return serverSocket;
    }
    
    public static String getConnectionInfo(String host, int port) throws Exception {
        SSLSocket socket = createSecureConnection(host, port);
        SSLSession session = socket.getSession();
        
        StringBuilder info = new StringBuilder();
        info.append("Protocol: ").append(session.getProtocol()).append("\\n");
        info.append("Cipher Suite: ").append(session.getCipherSuite()).append("\\n");
        
        socket.close();
        return info.toString();
    }
    
    public static boolean verifyConnection(String host, int port) {
        try {
            SSLSocket socket = createSecureConnection(host, port);
            socket.close();
            return true;
        } catch (Exception e) {
            return false;
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Verify secure connection to google.com:443");
        try {
            boolean result = verifyConnection("google.com", 443);
            System.out.println("Connection verified: " + result);
        } catch (Exception e) {
            System.out.println("Test 1 failed: " + e.getMessage());
        }
        
        System.out.println("\\nTest Case 2: Get connection info for google.com:443");
        try {
            String info = getConnectionInfo("google.com", 443);
            System.out.println(info);
        } catch (Exception e) {
            System.out.println("Test 2 failed: " + e.getMessage());
        }
        
        System.out.println("\\nTest Case 3: Send HTTPS request to example.com:443");
        try {
            String response = sendHttpsRequest("example.com", 443, "GET / HTTP/1.1");
            System.out.println("Response received (first 200 chars): " + 
                response.substring(0, Math.min(200, response.length())));
        } catch (Exception e) {
            System.out.println("Test 3 failed: " + e.getMessage());
        }
        
        System.out.println("\\nTest Case 4: Verify connection to httpbin.org:443");
        try {
            boolean result = verifyConnection("httpbin.org", 443);
            System.out.println("Connection verified: " + result);
        } catch (Exception e) {
            System.out.println("Test 4 failed: " + e.getMessage());
        }
        
        System.out.println("\\nTest Case 5: Test invalid connection");
        try {
            boolean result = verifyConnection("invalid-host-12345.com", 443);
            System.out.println("Connection verified: " + result);
        } catch (Exception e) {
            System.out.println("Test 5 - Expected failure: " + e.getMessage());
        }
    }
}
