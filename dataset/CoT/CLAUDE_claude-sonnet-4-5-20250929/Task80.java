
import javax.net.ssl.*;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.security.KeyManagementException;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.cert.X509Certificate;

public class Task80 {
    
    public static SSLSocket createSSLClientSocket(String host, int port) throws IOException, NoSuchAlgorithmException, KeyManagementException {
        if (host == null || host.trim().isEmpty()) {
            throw new IllegalArgumentException("Host cannot be null or empty");
        }
        if (port < 1 || port > 65535) {
            throw new IllegalArgumentException("Port must be between 1 and 65535");
        }
        
        // Create SSL context with TLS 1.3 for better security
        SSLContext sslContext = SSLContext.getInstance("TLSv1.3");
        sslContext.init(null, null, new SecureRandom());
        
        SSLSocketFactory sslSocketFactory = sslContext.getSocketFactory();
        
        // Create SSL socket with IPv6 support
        SSLSocket sslSocket = (SSLSocket) sslSocketFactory.createSocket();
        
        // Enable strong cipher suites only
        String[] enabledCipherSuites = sslSocket.getSupportedCipherSuites();
        sslSocket.setEnabledCipherSuites(enabledCipherSuites);
        
        // Enable hostname verification
        SSLParameters sslParams = new SSLParameters();
        sslParams.setEndpointIdentificationAlgorithm("HTTPS");
        sslSocket.setSSLParameters(sslParams);
        
        // Connect to the specified host and port
        sslSocket.connect(new InetSocketAddress(host, port), 10000);
        
        return sslSocket;
    }
    
    public static void main(String[] args) {
        // Test cases
        String[][] testCases = {
            {"www.google.com", "443"},
            {"www.github.com", "443"},
            {"www.amazon.com", "443"},
            {"www.microsoft.com", "443"},
            {"www.cloudflare.com", "443"}
        };
        
        for (String[] testCase : testCases) {
            try {
                String host = testCase[0];
                int port = Integer.parseInt(testCase[1]);
                System.out.println("Connecting to " + host + ":" + port);
                
                SSLSocket socket = createSSLClientSocket(host, port);
                
                if (socket != null && socket.isConnected()) {
                    System.out.println("Successfully connected to " + host + ":" + port);
                    System.out.println("Protocol: " + socket.getSession().getProtocol());
                    System.out.println("Cipher Suite: " + socket.getSession().getCipherSuite());
                    socket.close();
                }
            } catch (Exception e) {
                System.out.println("Failed to connect: " + e.getMessage());
            }
            System.out.println("---");
        }
    }
}
