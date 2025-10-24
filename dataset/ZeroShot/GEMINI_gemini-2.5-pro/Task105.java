import javax.net.ssl.*;
import java.io.IOException;
import java.net.Socket;
import java.net.UnknownHostException;
import java.security.NoSuchAlgorithmException;

public class Task105 {

    /**
     * Establishes a secure SSL/TLS connection to a remote server.
     *
     * @param host The hostname of the server.
     * @param port The port number of the server.
     * @return true if the connection was successful, false otherwise.
     */
    public static boolean establishSecureConnection(String host, int port) {
        SSLSocketFactory factory;
        try {
            // Create an SSLContext that uses a secure protocol.
            // TLSv1.3 is preferred, TLSv1.2 is also secure.
            SSLContext sslContext = SSLContext.getInstance("TLSv1.3");
            sslContext.init(null, null, null); // Use default KeyManager, TrustManager, and SecureRandom
            factory = sslContext.getSocketFactory();
        } catch (NoSuchAlgorithmException e) {
            System.err.println("TLSv1.3 is not available, falling back to default. Error: " + e.getMessage());
            factory = (SSLSocketFactory) SSLSocketFactory.getDefault();
        } catch (Exception e) {
            System.err.println("Could not initialize SSLContext. Error: " + e.getMessage());
            return false;
        }

        try (SSLSocket sslSocket = (SSLSocket) factory.createSocket(host, port)) {

            // Enable hostname verification, which is crucial for security.
            SSLParameters sslParams = sslSocket.getSSLParameters();
            sslParams.setEndpointIdentificationAlgorithm("HTTPS");
            sslSocket.setSSLParameters(sslParams);

            // Start the TLS handshake explicitly.
            sslSocket.startHandshake();

            SSLSession session = sslSocket.getSession();
            System.out.println("Successfully connected to " + host + ":" + port);
            System.out.println("SSL Session Info:");
            System.out.println("\tProtocol: " + session.getProtocol());
            System.out.println("\tCipher Suite: " + session.getCipherSuite());
            System.out.println("\tPeer Principal: " + session.getPeerPrincipal());

            return true;

        } catch (SSLHandshakeException e) {
            // This can happen for expired certs, wrong host, untrusted CAs, etc.
            System.err.println("SSL Handshake Failed for " + host + ":" + port + ". Error: " + e.getMessage());
        } catch (UnknownHostException e) {
            System.err.println("Could not resolve host: " + host);
        } catch (IOException e) {
            System.err.println("I/O error when connecting to " + host + ":" + port + ". Error: " + e.getMessage());
        } catch (Exception e) {
            System.err.println("An unexpected error occurred for " + host + ":" + port + ". Error: " + e.getMessage());
        }

        return false;
    }

    public static void main(String[] args) {
        String[] hosts = {"google.com", "github.com", "expired.badssl.com", "wrong.host.badssl.com", "nonexistent-domain-xyz123.com"};
        int port = 443;

        for (int i = 0; i < hosts.length; i++) {
            System.out.println("\n--- Test Case " + (i + 1) + ": Connecting to " + hosts[i] + ":" + port + " ---");
            boolean success = establishSecureConnection(hosts[i], port);
            System.out.println("Connection status: " + (success ? "SUCCESS" : "FAILURE"));
            System.out.println("----------------------------------------------------");
        }
    }
}