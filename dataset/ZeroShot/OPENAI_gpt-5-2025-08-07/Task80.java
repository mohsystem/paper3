import javax.net.ssl.*;
import javax.net.ssl.SNIHostName;
import java.net.*;
import java.security.KeyStore;
import java.util.ArrayList;
import java.util.List;

public class Task80 {
    public static SSLSocket createIPv6SSLSocket(String host, int port, int timeoutMs) throws Exception {
        if (host == null || host.isEmpty() || port <= 0 || port > 65535) {
            throw new IllegalArgumentException("Invalid host or port");
        }
        // Resolve host to IPv6 addresses only
        InetAddress[] all = InetAddress.getAllByName(host);
        List<Inet6Address> v6 = new ArrayList<>();
        for (InetAddress a : all) {
            if (a instanceof Inet6Address) {
                v6.add((Inet6Address) a);
            }
        }
        if (v6.isEmpty()) {
            throw new UnknownHostException("No IPv6 address found for host: " + host);
        }
        // Prepare plain TCP IPv6 socket
        Socket plain = new Socket();
        plain.setTcpNoDelay(true);
        plain.setKeepAlive(true);

        // Try connect to the first IPv6 address
        SocketAddress target = new InetSocketAddress(v6.get(0), port);
        plain.connect(target, Math.max(1, timeoutMs));

        // Build secure SSLContext using default trust store
        TrustManagerFactory tmf = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
        tmf.init((KeyStore) null);
        SSLContext ctx = SSLContext.getInstance("TLS");
        ctx.init(null, tmf.getTrustManagers(), null);

        SSLSocketFactory factory = ctx.getSocketFactory();
        SSLSocket ssl = (SSLSocket) factory.createSocket(plain, host, port, true);

        // Enforce modern protocols
        List<String> enabled = new ArrayList<>();
        for (String p : ssl.getSupportedProtocols()) {
            if ("TLSv1.3".equals(p) || "TLSv1.2".equals(p)) enabled.add(p);
        }
        if (!enabled.isEmpty()) {
            ssl.setEnabledProtocols(enabled.toArray(new String[0]));
        }

        // Hostname verification and SNI
        SSLParameters params = ssl.getSSLParameters();
        params.setEndpointIdentificationAlgorithm("HTTPS");
        try {
            params.setServerNames(java.util.Collections.singletonList(new SNIHostName(host)));
        } catch (IllegalArgumentException ignored) {
            // If host is an IP literal, SNI may not apply
        }
        ssl.setSSLParameters(params);

        ssl.startHandshake();
        return ssl;
    }

    public static void main(String[] args) {
        String[] hosts = new String[] {
                "ipv6.google.com",
                "www.cloudflare.com",
                "www.google.com",
                "www.wikipedia.org",
                "www.youtube.com"
        };
        int port = 443;
        for (String h : hosts) {
            SSLSocket s = null;
            try {
                s = createIPv6SSLSocket(h, port, 8000);
                System.out.println("Connected to " + h + " via IPv6, protocol=" + s.getSession().getProtocol());
            } catch (Exception e) {
                System.out.println("Failed to connect to " + h + ": " + e.getMessage());
            } finally {
                if (s != null) {
                    try { s.close(); } catch (Exception ignored) {}
                }
            }
        }
    }
}