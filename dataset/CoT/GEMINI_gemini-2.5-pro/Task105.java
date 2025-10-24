import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class Task105 {

    /**
     * Establishes a secure SSL/TLS connection to a remote server.
     *
     * @param host The hostname or IP address of the server.
     * @param port The port number of the server.
     * @return true if the connection was successfully established, false otherwise.
     */
    public static boolean establishTlsConnection(String host, int port) {
        // Use the default SSLSocketFactory. This factory is configured with
        // the system's default security properties, including the default
        // truststore, which is the secure and recommended approach.
        SSLSocketFactory factory = (SSLSocketFactory) SSLSocketFactory.getDefault();
        
        // Use try-with-resources to ensure the socket is always closed.
        try (SSLSocket socket = (SSLSocket) factory.createSocket(host, port)) {
            
            // Forcing modern, secure protocols is a good practice.
            // Modern JVMs have good defaults, but being explicit can prevent
            // accidental use of older, insecure protocols on misconfigured systems.
            socket.setEnabledProtocols(new String[]{"TLSv1.3", "TLSv1.2"});

            System.out.println("Starting TLS handshake with " + host + ":" + port + "...");
            
            // The handshake is initiated automatically on first I/O operation,
            // but calling it explicitly helps to separate connection errors
            // from application-level I/O errors. This will throw an SSLException
            // if certificate validation or other handshake steps fail.
            socket.startHandshake();
            
            System.out.println("Handshake successful!");
            System.out.println("Connected to: " + socket.getInetAddress());
            System.out.println("Using protocol: " + socket.getSession().getProtocol());
            System.out.println("Using cipher suite: " + socket.getSession().getCipherSuite());
            
            // Optional: Perform a simple I/O operation to verify the connection is live.
            // For example, send a basic HTTP GET request.
            OutputStream out = socket.getOutputStream();
            InputStream in = socket.getInputStream();
            out.write(("GET / HTTP/1.1\r\nHost: " + host + "\r\nConnection: close\r\n\r\n").getBytes());
            out.flush();
            
            // Read the first byte of the response
            int firstByte = in.read();
            if (firstByte != -1) {
                System.out.println("Successfully received response data.");
            }

            return true;

        } catch (IOException e) {
            // This catches a wide range of errors:
            // - UnknownHostException: DNS resolution failed.
            // - ConnectException: TCP connection failed (e.g., host down, firewall).
            // - SSLException: Handshake failed (e.g., certificate validation error, protocol mismatch).
            System.err.println("Failed to establish TLS connection with " + host + ":" + port);
            System.err.println("Error: " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Running SSL/TLS Connection Tests ---");

        // Test Case 1: Successful connection to a major site
        System.out.println("\n[Test Case 1: www.google.com]");
        boolean result1 = establishTlsConnection("www.google.com", 443);
        System.out.println("Test 1 Result: " + (result1 ? "SUCCESS" : "FAILURE"));

        // Test Case 2: Successful connection to another major site
        System.out.println("\n[Test Case 2: www.github.com]");
        boolean result2 = establishTlsConnection("www.github.com", 443);
        System.out.println("Test 2 Result: " + (result2 ? "SUCCESS" : "FAILURE"));

        // Test Case 3: Connection to a server with an expired certificate
        // This should fail certificate validation.
        System.out.println("\n[Test Case 3: expired.badssl.com]");
        boolean result3 = establishTlsConnection("expired.badssl.com", 443);
        System.out.println("Test 3 Result: " + (!result3 ? "SUCCESS (Correctly failed)" : "FAILURE (Incorrectly succeeded)"));

        // Test Case 4: Connection to a server with a mismatched hostname in the certificate
        // This should fail certificate validation.
        System.out.println("\n[Test Case 4: wrong.host.badssl.com]");
        boolean result4 = establishTlsConnection("wrong.host.badssl.com", 443);
        System.out.println("Test 4 Result: " + (!result4 ? "SUCCESS (Correctly failed)" : "FAILURE (Incorrectly succeeded)"));
        
        // Test Case 5: Connection to a non-existent domain
        // This should fail at the DNS resolution or TCP connection stage.
        System.out.println("\n[Test Case 5: non-existent-domain-for-testing-123.com]");
        boolean result5 = establishTlsConnection("non-existent-domain-for-testing-123.com", 443);
        System.out.println("Test 5 Result: " + (!result5 ? "SUCCESS (Correctly failed)" : "FAILURE (Incorrectly succeeded)"));
        
        System.out.println("\n--- All tests completed. ---");
    }
}