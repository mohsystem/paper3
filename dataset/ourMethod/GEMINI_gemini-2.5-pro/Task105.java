import javax.net.ssl.*;
import java.io.*;
import java.net.Socket;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;

public class Task105 {

    /**
     * Establishes a secure SSL/TLS connection to a remote server.
     *
     * @param host The hostname of the server.
     * @param port The port number of the server.
     * @return true if the connection was successful, false otherwise.
     */
    public static boolean establishSecureConnection(String host, int port) {
        if (host == null || host.isEmpty() || port <= 0 || port > 65535) {
            System.err.println("Invalid host or port.");
            return false;
        }

        // Use the default SSLSocketFactory which uses the system's default trust store.
        SSLSocketFactory factory = (SSLSocketFactory) SSLSocketFactory.getDefault();
        
        // Using try-with-resources to ensure the socket is closed automatically.
        try (SSLSocket socket = (SSLSocket) factory.createSocket(host, port)) {

            // Enable hostname verification (endpoint identification).
            // This is crucial to prevent man-in-the-middle attacks.
            SSLParameters params = socket.getSSLParameters();
            params.setEndpointIdentificationAlgorithm("HTTPS");
            socket.setSSLParameters(params);

            // Start the SSL handshake explicitly. This will throw an exception if validation fails.
            socket.startHandshake();

            SSLSession session = socket.getSession();
            System.out.println("Successfully connected to " + host + ":" + port);
            System.out.println("SSL/TLS Protocol: " + session.getProtocol());
            System.out.println("Cipher Suite: " + session.getCipherSuite());

            Certificate[] certs = session.getPeerCertificates();
            System.out.println("Peer Certificate: " + ((X509Certificate) certs[0]).getSubjectX500Principal());

            // Example: Send a simple HTTP GET request
            PrintWriter out = new PrintWriter(new OutputStreamWriter(socket.getOutputStream()));
            out.println("GET / HTTP/1.1");
            out.println("Host: " + host);
            out.println("Connection: Close");
            out.println();
            out.flush();

            // Read the response
            BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            String line;
            System.out.println("\n--- Server Response (first 5 lines) ---");
            for (int i = 0; i < 5 && (line = in.readLine()) != null; i++) {
                System.out.println(line);
            }
            System.out.println("-------------------------------------\n");

            return true;

        } catch (SSLException e) {
            System.err.println("SSL/TLS connection failed for " + host + ":" + port + ". Reason: " + e.getMessage());
            return false;
        } catch (IOException e) {
            System.err.println("I/O error for " + host + ":" + port + ". Reason: " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        String[] hosts = {
            "www.google.com",
            "www.cloudflare.com",
            "expired.badssl.com",
            "wrong.host.badssl.com",
            "self-signed.badssl.com"
        };
        int port = 443;

        for (String host : hosts) {
            System.out.println("--- Testing connection to " + host + " ---");
            boolean success = establishSecureConnection(host, port);
            System.out.println("Connection status: " + (success ? "SUCCESS" : "FAILURE"));
            System.out.println("--------------------------------------\n");
        }
    }
}