import javax.net.ssl.*;
import java.io.*;
import java.net.*;
import java.security.KeyManagementException;
import java.security.NoSuchAlgorithmException;

public class Task80 {

    /**
     * Creates and connects an SSL client socket over IPv6 to a specified host and port.
     * It enables modern TLS protocols and enforces hostname verification.
     *
     * @param host The hostname to connect to.
     * @param port The port number to connect to.
     * @return The connected and handshaked SSLSocket.
     * @throws IOException if a network error occurs.
     * @throws NoSuchAlgorithmException if the requested TLS protocol is not available.
     * @throws KeyManagementException if initializing the SSLContext fails.
     * @throws UnknownHostException if the host has no IPv6 address.
     * @throws IllegalArgumentException if the port is out of the valid range.
     */
    public static SSLSocket createSslIpv6Socket(String host, int port)
            throws IOException, NoSuchAlgorithmException, KeyManagementException {
        if (port <= 0 || port > 65535) {
            throw new IllegalArgumentException("Port out of valid range (1-65535)");
        }

        // Use a modern and secure TLS version.
        SSLContext sslContext = SSLContext.getInstance("TLSv1.3");
        // Initialize with default trust managers, which use the system's trust store
        // for certificate validation.
        sslContext.init(null, null, null);
        SSLSocketFactory factory = sslContext.getSocketFactory();
        
        // Create an unconnected socket
        SSLSocket sslSocket = (SSLSocket) factory.createSocket();
        
        // Enable hostname verification (prevents MITM attacks).
        SSLParameters sslParams = sslSocket.getSSLParameters();
        sslParams.setEndpointIdentificationAlgorithm("HTTPS");
        sslSocket.setSSLParameters(sslParams);

        // Resolve host to find an IPv6 address.
        InetAddress[] addresses = InetAddress.getAllByName(host);
        InetAddress ipv6Address = null;
        for (InetAddress addr : addresses) {
            if (addr instanceof Inet6Address) {
                ipv6Address = addr;
                break;
            }
        }

        if (ipv6Address == null) {
            throw new UnknownHostException("No IPv6 address found for host: " + host);
        }

        // Connect to the IPv6 address with a timeout.
        InetSocketAddress socketAddress = new InetSocketAddress(ipv6Address, port);
        int timeout = 5000; // 5 seconds
        sslSocket.connect(socketAddress, timeout);

        // The TLS handshake is started implicitly by connect or on the first I/O.
        return sslSocket;
    }

    public static void main(String[] args) {
        String[] testHosts = {"google.com", "ietf.org", "wikipedia.org", "thishostdoesnotexist12345.com", "google.com"};
        int[] testPorts = {443, 443, 443, 443, 99999};

        for (int i = 0; i < testHosts.length; i++) {
            String host = testHosts[i];
            int port = testPorts[i];
            System.out.println("--- Test Case " + (i + 1) + ": Connecting to " + host + ":" + port + " over IPv6 ---");
            // Use try-with-resources to ensure the socket is always closed.
            try (SSLSocket socket = createSslIpv6Socket(host, port)) {
                System.out.println("Successfully connected to " + socket.getRemoteSocketAddress());
                System.out.println("SSL Protocol: " + socket.getSession().getProtocol());
                System.out.println("Cipher Suite: " + socket.getSession().getCipherSuite());
                
                // Example: Send a simple HTTP GET request
                PrintWriter out = new PrintWriter(new OutputStreamWriter(socket.getOutputStream(), "UTF-8"));
                out.print("GET / HTTP/1.1\r\n");
                out.print("Host: " + host + "\r\n");
                out.print("Connection: close\r\n");
                out.print("\r\n");
                out.flush();

                BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream(), "UTF-8"));
                String line = in.readLine();
                if (line != null) {
                    System.out.println("Server response (first line): " + line);
                } else {
                    System.out.println("No response from server.");
                }

            } catch (Exception e) {
                System.err.println("Failed to connect to " + host + ":" + port + ". Reason: " + e.getMessage());
            }
            System.out.println();
        }
    }
}