import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.SSLSession;
import javax.security.auth.x500.X500Principal;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.UnknownHostException;

public class Task80 {

    /**
     * Creates an SSL client socket using the system's preferred IP protocol (IPv6 if configured)
     * and TCP, then connects to the specified host and port.
     * To explicitly prefer IPv6, set the system property "java.net.preferIPv6Addresses" to "true".
     *
     * @param host The hostname to connect to.
     * @param port The port number to connect to.
     * @return The established and connected SSLSocket.
     * @throws IOException If an I/O error occurs when creating or connecting the socket.
     * @throws UnknownHostException If the IP address of the host could not be determined.
     */
    public static SSLSocket createSslIpv6ClientSocket(String host, int port) throws IOException, UnknownHostException {
        // Get the default SSL socket factory. This factory is configured with
        // default security settings, including trust store, protocol versions, and cipher suites.
        SSLSocketFactory factory = (SSLSocketFactory) SSLSocketFactory.getDefault();

        // The createSocket method handles DNS resolution (including AAAA records for IPv6),
        // TCP connection, and the TLS handshake.
        SSLSocket sslSocket = (SSLSocket) factory.createSocket(host, port);
        
        // Ensure the handshake is complete before returning.
        sslSocket.startHandshake();

        return sslSocket;
    }

    public static void main(String[] args) {
        // To prefer IPv6 over IPv4, set this system property.
        // This is a global setting for the running JVM instance.
        System.setProperty("java.net.preferIPv6Addresses", "true");

        String[] testHosts = {
            "google.com",
            "www.facebook.com",
            "ipv6.google.com",
            "www.ietf.org",
            "badssl.com" // Known good SSL/TLS test site
        };
        int port = 443;

        for (String host : testHosts) {
            System.out.println("--- Testing connection to " + host + ":" + port + " ---");
            // Use try-with-resources to ensure the socket is always closed.
            try (SSLSocket socket = createSslIpv6ClientSocket(host, port)) {
                System.out.println("Successfully connected to " + socket.getInetAddress().getHostAddress());
                System.out.println("Remote address: " + socket.getRemoteSocketAddress());
                
                SSLSession session = socket.getSession();
                System.out.println("SSL/TLS Protocol: " + session.getProtocol());
                System.out.println("Cipher Suite: " + session.getCipherSuite());
                
                X500Principal peer = (X500Principal) session.getPeerPrincipal();
                System.out.println("Peer Principal: " + peer.getName());
                
            } catch (UnknownHostException e) {
                System.err.println("Error: Host not found or no IPv6 address available for " + host + ". " + e.getMessage());
            } catch (IOException e) {
                System.err.println("Error connecting to " + host + ": " + e.getMessage());
            } catch (Exception e) {
                System.err.println("An unexpected error occurred for " + host + ": " + e.toString());
            }
            System.out.println();
        }
    }
}