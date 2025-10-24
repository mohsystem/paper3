
import javax.net.ssl.*;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.security.KeyManagementException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.X509Certificate;
import java.util.Arrays;

public class Task80 {
    
    public static SSLSocket createSSLClientSocket(String host, int port) throws IOException, NoSuchAlgorithmException, KeyManagementException {
        // Input validation
        if (host == null || host.trim().isEmpty()) {
            throw new IllegalArgumentException("Host cannot be null or empty");
        }
        if (port < 1 || port > 65535) {
            throw new IllegalArgumentException("Port must be between 1 and 65535");
        }
        
        // Sanitize host input - remove any control characters
        host = host.replaceAll("[\\\\p{Cntrl}]", "");
        
        // Create SSL context with TLS 1.3 or TLS 1.2
        SSLContext sslContext = SSLContext.getInstance("TLS");
        sslContext.init(null, null, null); // Use default trust managers
        
        // Create SSL socket factory
        SSLSocketFactory socketFactory = sslContext.getSocketFactory();
        
        // Create SSL socket with IPv6 support
        SSLSocket sslSocket = (SSLSocket) socketFactory.createSocket();
        
        // Enable strong protocols only (TLS 1.2 and TLS 1.3)
        String[] supportedProtocols = sslSocket.getSupportedProtocols();
        String[] enabledProtocols = Arrays.stream(supportedProtocols)
            .filter(p -> p.equals("TLSv1.3") || p.equals("TLSv1.2"))
            .toArray(String[]::new);
        sslSocket.setEnabledProtocols(enabledProtocols);
        
        // Enable strong cipher suites only
        String[] supportedCiphers = sslSocket.getSupportedCipherSuites();
        String[] enabledCiphers = Arrays.stream(supportedCiphers)
            .filter(c -> !c.contains("_NULL_") && 
                        !c.contains("_EXPORT_") && 
                        !c.contains("_anon_") &&
                        !c.contains("_DES_") &&
                        !c.contains("_MD5") &&
                        (c.contains("_GCM_") || c.contains("_CCM_")))
            .toArray(String[]::new);
        sslSocket.setEnabledCipherSuites(enabledCiphers);
        
        // Enable endpoint identification for hostname verification
        SSLParameters sslParams = sslSocket.getSSLParameters();
        sslParams.setEndpointIdentificationAlgorithm("HTTPS");
        sslSocket.setSSLParameters(sslParams);
        
        // Connect to the specified host and port
        sslSocket.connect(new InetSocketAddress(host, port), 10000); // 10 second timeout
        
        // Start the SSL handshake
        sslSocket.startHandshake();
        
        // Verify the certificate chain
        SSLSession session = sslSocket.getSession();
        if (session == null || !session.isValid()) {
            sslSocket.close();
            throw new IOException("Invalid SSL session");
        }
        
        // Verify peer certificates exist
        try {
            java.security.cert.Certificate[] certs = session.getPeerCertificates();
            if (certs == null || certs.length == 0) {
                sslSocket.close();
                throw new IOException("No peer certificates found");
            }
            
            // Verify certificate is X509 and not expired
            if (certs[0] instanceof X509Certificate) {
                X509Certificate x509 = (X509Certificate) certs[0];
                x509.checkValidity();
            }
        } catch (Exception e) {
            sslSocket.close();
            throw new IOException("Certificate validation failed: " + e.getMessage());
        }
        
        return sslSocket;
    }
    
    public static void main(String[] args) {
        // Test cases
        String[][] testCases = {
            {"www.google.com", "443"},
            {"www.github.com", "443"},
            {"www.cloudflare.com", "443"},
            {"www.amazon.com", "443"},
            {"www.microsoft.com", "443"}
        };
        
        for (String[] testCase : testCases) {
            String host = testCase[0];
            int port = Integer.parseInt(testCase[1]);
            
            try (SSLSocket socket = createSSLClientSocket(host, port)) {
                System.out.println("Successfully connected to " + host + ":" + port);
                System.out.println("Protocol: " + socket.getSession().getProtocol());
                System.out.println("Cipher Suite: " + socket.getSession().getCipherSuite());
                System.out.println("---");
            } catch (Exception e) {
                System.err.println("Failed to connect to " + host + ":" + port + " - " + e.getMessage());
            }
        }
    }
}
