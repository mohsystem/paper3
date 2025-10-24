import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;
import java.io.IOException;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;

public class Task80 {

    /**
     * Creates an SSL client socket using IPv6 and TCP.
     *
     * @param host The hostname to connect to.
     * @param port The port to connect to.
     * @return The established SSLSocket, or null if an IPv6 address could not be found or connection failed.
     */
    public static SSLSocket createIPv6SslClientSocket(String host, int port) {
        try {
            InetAddress[] addresses = InetAddress.getAllByName(host);
            Inet6Address ipv6Address = null;

            for (InetAddress addr : addresses) {
                if (addr instanceof Inet6Address) {
                    ipv6Address = (Inet6Address) addr;
                    break;
                }
            }

            if (ipv6Address == null) {
                System.err.println("No IPv6 address found for host: " + host);
                return null;
            }

            SSLSocketFactory factory = (SSLSocketFactory) SSLSocketFactory.getDefault();
            SSLSocket sslSocket = (SSLSocket) factory.createSocket(ipv6Address, port);
            
            // Start the handshake to confirm the connection
            sslSocket.startHandshake();
            
            return sslSocket;
        } catch (UnknownHostException e) {
            System.err.println("Host not found: " + e.getMessage());
            return null;
        } catch (IOException e) {
            System.err.println("I/O error during connection: " + e.getMessage());
            return null;
        }
    }

    public static void main(String[] args) {
        String[] hosts = {"google.com", "facebook.com", "ipv6.google.com", "wikipedia.org", "example.com"};
        int port = 443;

        for (String host : hosts) {
            System.out.println("--- Testing connection to " + host + ":" + port + " ---");
            SSLSocket sslSocket = null;
            try {
                sslSocket = createIPv6SslClientSocket(host, port);
                if (sslSocket != null && sslSocket.isConnected()) {
                    System.out.println("Successfully connected to " + host + " over IPv6.");
                    System.out.println("Remote Address: " + sslSocket.getInetAddress());
                    System.out.println("Cipher Suite: " + sslSocket.getSession().getCipherSuite());
                } else {
                    System.out.println("Failed to connect to " + host + " over IPv6.");
                }
            } catch (Exception e) {
                System.err.println("An exception occurred for host " + host + ": " + e.getMessage());
                 e.printStackTrace();
            } finally {
                if (sslSocket != null) {
                    try {
                        sslSocket.close();
                        System.out.println("Socket closed.");
                    } catch (IOException e) {
                        System.err.println("Error closing socket for host " + host + ": " + e.getMessage());
                    }
                }
            }
            System.out.println();
        }
    }
}