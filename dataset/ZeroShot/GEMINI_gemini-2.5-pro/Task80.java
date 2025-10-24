import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.UnknownHostException;

public class Task80 {

    /**
     * Creates a secure SSL client socket using IPv6 and connects to the specified host and port.
     *
     * @param host The hostname to connect to.
     * @param port The port number to connect to.
     * @return The connected SSLSocket, or null if the connection fails or no IPv6 address is found.
     */
    public static SSLSocket createIPv6SslSocket(String host, int port) {
        try {
            // 1. Resolve hostname to an IPv6 address
            InetAddress[] addresses = InetAddress.getAllByName(host);
            Inet6Address ipv6Address = null;
            for (InetAddress addr : addresses) {
                if (addr instanceof Inet6Address) {
                    ipv6Address = (Inet6Address) addr;
                    break;
                }
            }

            if (ipv6Address == null) {
                System.err.println("No IPv6 address found for " + host);
                return null;
            }
            System.out.println("Resolved " + host + " to IPv6 address: " + ipv6Address.getHostAddress());

            // 2. Create SSL socket and connect
            SSLSocketFactory factory = (SSLSocketFactory) SSLSocketFactory.getDefault();
            // The factory uses the system's default trust store for certificate validation
            SSLSocket sslSocket = (SSLSocket) factory.createSocket(ipv6Address, port);
            
            // 3. Enable modern TLS protocols
            sslSocket.setEnabledProtocols(new String[]{"TLSv1.2", "TLSv1.3"});
            
            // Start the handshake
            sslSocket.startHandshake();

            System.out.println("SSL handshake successful with " + host + ":" + port);
            return sslSocket;

        } catch (UnknownHostException e) {
            System.err.println("Error: Host not found " + host + ". " + e.getMessage());
        } catch (SocketException e) {
            System.err.println("Error: Could not connect to " + host + ":" + port + ". " + e.getMessage());
        } catch (Exception e) {
            System.err.println("An unexpected error occurred for " + host + ":" + port + ". " + e.getMessage());
        }
        return null;
    }

    public static void main(String[] args) {
        // --- Test Cases ---
        String[] hosts = {"google.com", "facebook.com", "wikipedia.org", "nonexistent.veryunlikelydomain.com", "google.com"};
        int[] ports = {443, 443, 443, 443, 444};

        for (int i = 0; i < hosts.length; i++) {
            String host = hosts[i];
            int port = ports[i];
            System.out.println("\n--- Test Case " + (i + 1) + ": Connecting to " + host + ":" + port + " ---");

            try (SSLSocket socket = createIPv6SslSocket(host, port)) {
                if (socket != null && socket.isConnected()) {
                    System.out.println("Successfully connected to " + host + ":" + port);

                    // Perform a simple HTTP GET to verify connection
                    PrintWriter out = new PrintWriter(socket.getOutputStream());
                    out.print("GET / HTTP/1.1\r\n");
                    out.print("Host: " + host + "\r\n");
                    out.print("Connection: close\r\n\r\n");
                    out.flush();

                    BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                    String line = in.readLine();
                    System.out.println("Response from server: " + line);
                    
                } else {
                    System.out.println("Failed to connect to " + host + ":" + port);
                }
            } catch (Exception e) {
                System.err.println("Error during communication with " + host + ": " + e.getMessage());
            }
        }
    }
}