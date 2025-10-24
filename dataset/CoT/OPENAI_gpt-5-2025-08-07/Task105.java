import javax.net.ssl.*;
import javax.net.SocketFactory;
import javax.net.ssl.SNIHostName;
import java.io.*;
import java.net.*;
import java.security.SecureRandom;
import java.util.*;

public class Task105 {

    // Establishes a secure TLS connection to host:port, performs an HTTPS GET for the path, and returns the status line.
    public static String httpsGet(String host, int port, String path, int timeoutMillis) throws Exception {
        if (host == null || host.isEmpty()) throw new IllegalArgumentException("host is required");
        if (path == null || path.isEmpty()) path = "/";
        if (!path.startsWith("/")) path = "/" + path;
        if (port <= 0) port = 443;
        if (timeoutMillis <= 0) timeoutMillis = 10000;

        SSLContext ctx = SSLContext.getInstance("TLS");
        ctx.init(null, null, new SecureRandom());
        SSLSocketFactory factory = ctx.getSocketFactory();

        Socket plain = new Socket();
        plain.setReuseAddress(false);
        plain.connect(new InetSocketAddress(host, port), timeoutMillis);
        plain.setSoTimeout(timeoutMillis);

        SSLSocket ssl = (SSLSocket) factory.createSocket(plain, host, port, true);

        // Limit protocols to TLSv1.2 and TLSv1.3 if available
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
        params.setEndpointIdentificationAlgorithm("HTTPS"); // enable hostname verification
        try {
            params.setServerNames(Collections.singletonList(new SNIHostName(host))); // SNI
        } catch (IllegalArgumentException ignored) {
            // Hostname may be invalid for SNI; ignore, but connection may fail later due to verification
        }
        ssl.setSSLParameters(params);

        ssl.startHandshake(); // perform TLS handshake with verification

        String request = "GET " + path + " HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" +
                "User-Agent: Task105-Java/1.0\r\n" +
                "Accept: */*\r\n" +
                "Connection: close\r\n\r\n";

        BufferedWriter out = new BufferedWriter(new OutputStreamWriter(ssl.getOutputStream(), "UTF-8"));
        out.write(request);
        out.flush();

        BufferedReader in = new BufferedReader(new InputStreamReader(ssl.getInputStream(), "UTF-8"));
        String statusLine = in.readLine();

        try { in.close(); } catch (Exception ignored) {}
        try { out.close(); } catch (Exception ignored) {}
        try { ssl.close(); } catch (Exception ignored) {}

        if (statusLine == null) {
            throw new IOException("No response received");
        }
        return statusLine;
    }

    public static void main(String[] args) {
        String[][] tests = new String[][]{
                {"www.google.com", "443", "/"},
                {"www.github.com", "443", "/"},
                {"www.cloudflare.com", "443", "/"},
                {"www.wikipedia.org", "443", "/"},
                {"example.com", "443", "/"}
        };
        for (String[] t : tests) {
            String host = t[0];
            int port = Integer.parseInt(t[1]);
            String path = t[2];
            try {
                String result = httpsGet(host, port, path, 10000);
                System.out.println(host + ": " + result);
            } catch (Exception e) {
                System.out.println(host + ": ERROR - " + e.getMessage());
            }
        }
    }
}