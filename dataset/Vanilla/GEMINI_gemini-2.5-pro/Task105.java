import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;
import java.io.IOException;

public class Task105 {

    /**
     * Establishes a secure SSL/TLS connection to a remote server.
     * @param host The hostname of the server.
     * @param port The port number of the server.
     * @return true if the connection was successful, false otherwise.
     */
    public static boolean establishSSLConnection(String host, int port) {
        System.out.println("Attempting to connect to " + host + ":" + port + "...");
        try {
            SSLSocketFactory factory = (SSLSocketFactory) SSLSocketFactory.getDefault();
            // The try-with-resources statement ensures the socket is closed automatically.
            // createSocket connects and performs the handshake.
            try (SSLSocket socket = (SSLSocket) factory.createSocket(host, port)) {
                
                socket.startHandshake(); // Can be called explicitly if needed

                System.out.println("Successfully connected to " + host + ":" + port);
                System.out.println("SSL Session Info:");
                System.out.println("\tProtocol: " + socket.getSession().getProtocol());
                System.out.println("\tCipher Suite: " + socket.getSession().getCipherSuite());
                System.out.println("\tPeer Host: " + socket.getSession().getPeerHost());
                
                return true;
            }
        } catch (IOException e) {
            System.err.println("Failed to connect to " + host + ":" + port);
            System.err.println("Error: " + e.getMessage());
            return false;
        } finally {
            System.out.println("----------------------------------------");
        }
    }

    public static void main(String[] args) {
        // 5 Test Cases
        establishSSLConnection("google.com", 443);
        establishSSLConnection("github.com", 443);
        establishSSLConnection("cloudflare.com", 443);
        establishSSLConnection("amazon.com", 443);
        establishSSLConnection("nonexistent-host-for-testing.com", 443); // Failure case
    }
}