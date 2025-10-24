import javax.net.ssl.*;
import javax.net.ssl.SSLSocket;
import java.io.IOException;
import java.net.*;
import java.security.SecureRandom;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class Task80 {

    public static SSLSocket createIPv6TLSClient(String host, int port, int timeoutMillis) throws Exception {
        if (host == null || host.isEmpty()) {
            throw new IllegalArgumentException("host must be non-empty");
        }
        if (port < 1 || port > 65535) {
            throw new IllegalArgumentException("port out of range");
        }
        if (timeoutMillis <= 0 || timeoutMillis > 60000) {
            throw new IllegalArgumentException("timeoutMillis must be 1..60000");
        }

        Inet6Address ipv6 = null;
        InetAddress[] all = InetAddress.getAllByName(host);
        for (InetAddress addr : all) {
            if (addr instanceof Inet6Address) {
                ipv6 = (Inet6Address) addr;
                break;
            }
        }
        if (ipv6 == null) {
            throw new UnknownHostException("No IPv6 address found for host: " + host);
        }

        Socket plain = null;
        SSLSocket ssl = null;
        try {
            plain = new Socket();
            plain.connect(new InetSocketAddress(ipv6, port), timeoutMillis);
            plain.setSoTimeout(timeoutMillis);

            SSLContext ctx = SSLContext.getInstance("TLS");
            ctx.init(null, null, new SecureRandom());

            SSLSocketFactory sf = ctx.getSocketFactory();
            ssl = (SSLSocket) sf.createSocket(plain, host, port, true);

            // Enforce modern protocols only
            List<String> allowed = new ArrayList<>();
            for (String p : ssl.getSupportedProtocols()) {
                if ("TLSv1.3".equals(p) || "TLSv1.2".equals(p)) {
                    allowed.add(p);
                }
            }
            if (!allowed.isEmpty()) {
                ssl.setEnabledProtocols(allowed.toArray(new String[0]));
            }

            SSLParameters params = ssl.getSSLParameters();
            // Strict hostname verification
            params.setEndpointIdentificationAlgorithm("HTTPS");
            // SNI if host looks like a DNS name
            try {
                if (host.matches("(?i)^[a-z0-9](?:[a-z0-9-]{0,61}[a-z0-9])?(?:\\.[a-z0-9](?:[a-z0-9-]{0,61}[a-z0-9])?)*\\.?$") && host.length() <= 253) {
                    params.setServerNames(Arrays.asList(new SNIHostName(host)));
                }
            } catch (IllegalArgumentException ignored) {
                // Ignore invalid SNI hostnames (e.g., IP literals)
            }
            ssl.setSSLParameters(params);

            // Trigger handshake and certificate validation
            ssl.startHandshake();

            return ssl;
        } catch (Exception e) {
            if (ssl != null) {
                try { ssl.close(); } catch (IOException ignored) {}
            } else if (plain != null && !plain.isClosed()) {
                try { plain.close(); } catch (IOException ignored) {}
            }
            throw e;
        }
    }

    private static void closeQuietly(SSLSocket s) {
        if (s != null) {
            try {
                s.close();
            } catch (IOException ignored) {
            }
        }
    }

    public static void main(String[] args) {
        String[] hosts = new String[] {
                "google.com",
                "cloudflare.com",
                "wikipedia.org",
                "ietf.org",
                "facebook.com"
        };
        int port = 443;
        int timeout = 4000;

        for (String h : hosts) {
            SSLSocket s = null;
            try {
                System.out.println("Connecting (IPv6/TLS) to " + h + ":" + port);
                s = createIPv6TLSClient(h, port, timeout);
                System.out.println("Success: " + h + " - Protocol=" + s.getSession().getProtocol() + ", Cipher=" + s.getSession().getCipherSuite());
            } catch (Exception ex) {
                System.out.println("Failed: " + h + " - " + ex.getClass().getSimpleName() + ": " + ex.getMessage());
            } finally {
                closeQuietly(s);
            }
        }
    }
}