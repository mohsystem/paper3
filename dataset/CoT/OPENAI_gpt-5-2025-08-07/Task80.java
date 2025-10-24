import javax.net.ssl.*;
import javax.net.ssl.SNIServerName;
import javax.net.ssl.SNIHostName;
import java.net.*;
import java.security.NoSuchAlgorithmException;
import java.util.*;
import java.io.IOException;

/*
 Chain-of-Through process (high-level, non-sensitive) embedded as comments:
 1) Problem understanding: create an IPv6 TCP SSL client socket that connects to a specified host/port and returns the established socket.
 2) Security requirements: enforce certificate validation, hostname verification, SNI, strong TLS versions, and timeouts.
 3) Secure coding generation: implement IPv6 resolution only (AAAA), configure SSL parameters, and perform handshake.
 4) Code review: ensure proper cleanup on failure and robust error handling.
 5) Secure code output: finalized implementation below.
*/

public class Task80 {
    public static SSLSocket createIPv6SslClient(String host, int port) throws Exception {
        if (host == null || host.isEmpty() || port <= 0 || port > 65535) {
            throw new IllegalArgumentException("Invalid host or port");
        }

        // Resolve IPv6 address only
        Inet6Address ipv6Address = null;
        for (InetAddress addr : InetAddress.getAllByName(host)) {
            if (addr instanceof Inet6Address) {
                ipv6Address = (Inet6Address) addr;
                break;
            }
        }
        if (ipv6Address == null) {
            throw new UnknownHostException("No IPv6 address found for host: " + host);
        }

        // SSL context with default trust store and secure defaults
        SSLContext ctx = SSLContext.getInstance("TLS");
        ctx.init(null, null, null);
        SSLSocketFactory factory = ctx.getSocketFactory();

        SSLSocket sslSocket = (SSLSocket) factory.createSocket();
        // Configure preferred protocols (TLSv1.3, TLSv1.2)
        String[] supported = sslSocket.getSupportedProtocols();
        List<String> preferred = new ArrayList<>();
        Set<String> sup = new HashSet<>(Arrays.asList(supported));
        if (sup.contains("TLSv1.3")) preferred.add("TLSv1.3");
        if (sup.contains("TLSv1.2")) preferred.add("TLSv1.2");
        if (!preferred.isEmpty()) {
            sslSocket.setEnabledProtocols(preferred.toArray(new String[0]));
        }

        // Connect with timeout
        int connectTimeoutMs = 10000;
        sslSocket.connect(new InetSocketAddress(ipv6Address, port), connectTimeoutMs);
        sslSocket.setSoTimeout(10000); // read timeout

        // Enforce SNI and hostname verification
        SSLParameters params = sslSocket.getSSLParameters();
        params.setEndpointIdentificationAlgorithm("HTTPS"); // hostname verification
        List<SNIServerName> sniNames = Collections.singletonList(new SNIHostName(host));
        try {
            params.setServerNames(sniNames);
        } catch (IllegalArgumentException ignored) {
            // Some hosts may not conform to SNI requirements; continue with hostname verification.
        }
        sslSocket.setSSLParameters(params);

        // Handshake to validate certifications and establish session
        sslSocket.startHandshake();

        return sslSocket;
    }

    public static void main(String[] args) {
        String[] hosts = new String[] {
            "google.com",
            "cloudflare.com",
            "example.com",
            "ietf.org",
            "wikipedia.org"
        };
        int port = 443;

        for (String host : hosts) {
            System.out.println("Connecting to " + host + " over IPv6 TLS...");
            try (SSLSocket s = createIPv6SslClient(host, port)) {
                SSLSession sess = s.getSession();
                System.out.println("Connected: " + s.getRemoteSocketAddress());
                System.out.println("Protocol: " + sess.getProtocol() + ", Cipher: " + sess.getCipherSuite());
            } catch (Exception e) {
                System.out.println("Failed to connect to " + host + ": " + e.getMessage());
            }
            System.out.println("----");
        }
    }
}