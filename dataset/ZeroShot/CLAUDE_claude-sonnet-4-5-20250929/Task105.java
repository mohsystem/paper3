
import javax.net.ssl.*;
import java.io.*;
import java.security.cert.X509Certificate;

public class Task105 {
    public static SSLSocket establishSecureConnection(String host, int port) throws Exception {
        SSLContext sslContext = SSLContext.getInstance("TLSv1.3");
        sslContext.init(null, null, null);
        
        SSLSocketFactory factory = sslContext.getSocketFactory();
        SSLSocket socket = (SSLSocket) factory.createSocket(host, port);
        
        // Enable only strong protocols
        socket.setEnabledProtocols(new String[]{"TLSv1.3", "TLSv1.2"});
        
        // Enable only strong cipher suites
        String[] supportedCiphers = socket.getSupportedCipherSuites();
        socket.setEnabledCipherSuites(filterStrongCiphers(supportedCiphers));
        
        socket.startHandshake();
        
        return socket;
    }
    
    private static String[] filterStrongCiphers(String[] ciphers) {
        java.util.List<String> strongCiphers = new java.util.ArrayList<>();
        for (String cipher : ciphers) {
            if (cipher.contains("GCM") || cipher.contains("CHACHA20")) {
                if (!cipher.contains("NULL") && !cipher.contains("anon") && 
                    !cipher.contains("EXPORT") && !cipher.contains("DES")) {
                    strongCiphers.add(cipher);
                }
            }
        }
        return strongCiphers.toArray(new String[0]);
    }
    
    public static String sendRequest(String host, int port, String request) throws Exception {
        SSLSocket socket = establishSecureConnection(host, port);
        
        PrintWriter out = new PrintWriter(
            new BufferedWriter(new OutputStreamWriter(socket.getOutputStream())));
        BufferedReader in = new BufferedReader(
            new InputStreamReader(socket.getInputStream()));
        
        out.println(request);
        out.flush();
        
        StringBuilder response = new StringBuilder();
        String line;
        while ((line = in.readLine()) != null && response.length() < 10000) {
            response.append(line).append("\\n");
            if (line.isEmpty()) break;
        }
        
        in.close();
        out.close();
        socket.close();
        
        return response.toString();
    }
    
    public static void main(String[] args) {
        System.out.println("SSL/TLS Secure Connection Test Cases:\\n");
        
        // Test case 1: HTTPS connection to Google
        try {
            System.out.println("Test 1: Connecting to www.google.com:443");
            String response = sendRequest("www.google.com", 443, 
                "GET / HTTP/1.1\\r\\nHost: www.google.com\\r\\n\\r\\n");
            System.out.println("Connection successful! Response length: " + response.length());
        } catch (Exception e) {
            System.out.println("Test 1 failed: " + e.getMessage());
        }
        
        // Test case 2: HTTPS connection to GitHub
        try {
            System.out.println("\\nTest 2: Connecting to github.com:443");
            String response = sendRequest("github.com", 443, 
                "GET / HTTP/1.1\\r\\nHost: github.com\\r\\n\\r\\n");
            System.out.println("Connection successful! Response length: " + response.length());
        } catch (Exception e) {
            System.out.println("Test 2 failed: " + e.getMessage());
        }
        
        // Test case 3: HTTPS connection to Amazon
        try {
            System.out.println("\\nTest 3: Connecting to www.amazon.com:443");
            String response = sendRequest("www.amazon.com", 443, 
                "GET / HTTP/1.1\\r\\nHost: www.amazon.com\\r\\n\\r\\n");
            System.out.println("Connection successful! Response length: " + response.length());
        } catch (Exception e) {
            System.out.println("Test 3 failed: " + e.getMessage());
        }
        
        // Test case 4: HTTPS connection to Microsoft
        try {
            System.out.println("\\nTest 4: Connecting to www.microsoft.com:443");
            String response = sendRequest("www.microsoft.com", 443, 
                "GET / HTTP/1.1\\r\\nHost: www.microsoft.com\\r\\n\\r\\n");
            System.out.println("Connection successful! Response length: " + response.length());
        } catch (Exception e) {
            System.out.println("Test 4 failed: " + e.getMessage());
        }
        
        // Test case 5: Invalid host test
        try {
            System.out.println("\\nTest 5: Connecting to invalid.host.example:443");
            String response = sendRequest("invalid.host.example", 443, 
                "GET / HTTP/1.1\\r\\nHost: invalid.host.example\\r\\n\\r\\n");
            System.out.println("Connection successful! Response length: " + response.length());
        } catch (Exception e) {
            System.out.println("Test 5 failed (expected): " + e.getMessage());
        }
    }
}
