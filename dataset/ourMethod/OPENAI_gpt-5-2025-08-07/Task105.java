import javax.net.ssl.*;
import javax.net.ssl.SSLSocket;
import javax.net.ssl.SNIServerName;
import javax.net.ssl.SNIHostName;
import java.io.*;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.security.GeneralSecurityException;
import java.security.SecureRandom;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class Task105 {
    public static String tlsHttpGet(String host, int port, String path, int timeoutMillis, int maxBytes) {
        if (host == null || host.isEmpty()) return "ERROR: invalid host";
        if (port < 1 || port > 65535) return "ERROR: invalid port";
        if (path == null || path.isEmpty() || !path.startsWith("/")) path = "/";

        SSLSocket sslSocket = null;
        try {
            SSLContext ctx = SSLContext.getInstance("TLS");
            ctx.init(null, null, new SecureRandom());
            SSLSocketFactory factory = ctx.getSocketFactory();

            sslSocket = (SSLSocket) factory.createSocket();
            sslSocket.setSoTimeout(Math.max(1000, timeoutMillis));

            // Configure protocols
            String[] desired = new String[] {"TLSv1.3", "TLSv1.2"};
            sslSocket.setEnabledProtocols(filterSupported(sslSocket.getSupportedProtocols(), desired));

            // Hostname verification and SNI
            SSLParameters params = sslSocket.getSSLParameters();
            params.setEndpointIdentificationAlgorithm("HTTPS");
            List<SNIServerName> sni = new ArrayList<>(1);
            sni.add(new SNIHostName(host));
            params.setServerNames(sni);
            sslSocket.setSSLParameters(params);

            // Connect and handshake
            sslSocket.connect(new InetSocketAddress(host, port), Math.max(1000, timeoutMillis));
            sslSocket.startHandshake();

            String request = "GET " + path + " HTTP/1.1\r\n" +
                    "Host: " + host + "\r\n" +
                    "User-Agent: SecureClient/1.0\r\n" +
                    "Accept: */*\r\n" +
                    "Connection: close\r\n\r\n";
            try (OutputStream out = sslSocket.getOutputStream()) {
                out.write(request.getBytes(StandardCharsets.US_ASCII));
                out.flush();
            }

            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            byte[] buf = new byte[4096];
            int remaining = Math.max(1, maxBytes);
            try (InputStream in = sslSocket.getInputStream()) {
                while (remaining > 0) {
                    int toRead = Math.min(buf.length, remaining);
                    int n = in.read(buf, 0, toRead);
                    if (n == -1) break;
                    baos.write(buf, 0, n);
                    remaining -= n;
                }
            }
            return baos.toString(StandardCharsets.UTF_8.name());
        } catch (GeneralSecurityException e) {
            return "ERROR: security exception";
        } catch (IOException e) {
            return "ERROR: I/O exception";
        } finally {
            if (sslSocket != null) {
                try { sslSocket.close(); } catch (IOException ignored) {}
            }
        }
    }

    private static String[] filterSupported(String[] supported, String[] desired) {
        List<String> set = new ArrayList<>();
        for (String d : desired) {
            for (String s : supported) {
                if (s.equals(d)) {
                    set.add(d);
                    break;
                }
            }
        }
        if (set.isEmpty()) return supported; // fallback to defaults
        return set.toArray(new String[0]);
    }

    public static void main(String[] args) {
        // Five test cases
        String[] hosts = new String[] {
                "example.com",
                "www.google.com",
                "www.cloudflare.com",
                "www.github.com",
                "www.wikipedia.org"
        };
        for (String h : hosts) {
            String resp = tlsHttpGet(h, 443, "/", 8000, 2048);
            String firstLine = "ERROR: no response";
            if (resp != null && !resp.startsWith("ERROR")) {
                int idx = resp.indexOf("\r\n");
                if (idx > 0) firstLine = resp.substring(0, idx);
                else firstLine = resp.length() > 120 ? resp.substring(0, 120) : resp;
            } else if (resp != null) {
                firstLine = resp;
            }
            System.out.println(h + " -> " + firstLine);
        }
    }
}