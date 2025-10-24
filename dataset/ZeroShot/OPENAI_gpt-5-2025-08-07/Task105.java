import javax.net.ssl.*;
import javax.net.ssl.SSLSocket;
import java.net.*;
import java.io.*;
import java.security.GeneralSecurityException;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.util.*;

public class Task105 {
    public static String establishSecureConnection(String host, int port, int timeoutMillis) {
        Objects.requireNonNull(host, "host");
        if (port <= 0 || port > 65535) {
            return "ERROR: Invalid port";
        }
        if (timeoutMillis <= 0) {
            return "ERROR: Invalid timeout";
        }
        try {
            SSLContext ctx = SSLContext.getInstance("TLS");
            ctx.init(null, null, null); // Use system defaults (secure)
            SSLSocketFactory factory = ctx.getSocketFactory();

            try (SSLSocket socket = (SSLSocket) factory.createSocket()) {
                socket.setReuseAddress(false);
                socket.setSoTimeout(timeoutMillis);

                // Enforce modern TLS versions only
                String[] supported = socket.getSupportedProtocols();
                List<String> allowed = new ArrayList<>();
                for (String p : supported) {
                    if ("TLSv1.3".equals(p) || "TLSv1.2".equals(p)) {
                        allowed.add(p);
                    }
                }
                if (!allowed.isEmpty()) {
                    socket.setEnabledProtocols(allowed.toArray(new String[0]));
                }

                // Prepare and enforce hostname verification + SNI
                SSLParameters params = socket.getSSLParameters();
                params.setEndpointIdentificationAlgorithm("HTTPS");
                try {
                    params.setServerNames(Collections.singletonList(new SNIHostName(host)));
                } catch (IllegalArgumentException ignored) {
                    // If host is not a valid SNI name, let the handshake fail normally
                }
                socket.setSSLParameters(params);

                SocketAddress remote = new InetSocketAddress(host, port);
                socket.connect(remote, timeoutMillis);
                socket.startHandshake();

                SSLSession session = socket.getSession();
                String protocol = session.getProtocol();
                String cipher = session.getCipherSuite();

                String subject = "";
                try {
                    Certificate[] peerCerts = session.getPeerCertificates();
                    if (peerCerts != null && peerCerts.length > 0 && peerCerts[0] instanceof X509Certificate) {
                        X509Certificate cert = (X509Certificate) peerCerts[0];
                        subject = cert.getSubjectX500Principal().getName();
                    }
                } catch (SSLPeerUnverifiedException e) {
                    subject = "Unverified peer";
                }

                return "OK host=" + host + " port=" + port + " protocol=" + protocol + " cipher=" + cipher + " subject=\"" + subject + "\"";
            }
        } catch (IOException | GeneralSecurityException e) {
            return "ERROR: " + e.getClass().getSimpleName() + ": " + e.getMessage();
        }
    }

    public static void main(String[] args) {
        List<String> hosts = Arrays.asList(
                "www.google.com",
                "www.cloudflare.com",
                "www.github.com",
                "www.wikipedia.org",
                "api.openai.com"
        );
        int port = 443;
        int timeoutMs = 8000;

        for (String h : hosts) {
            String res = establishSecureConnection(h, port, timeoutMs);
            System.out.println(res);
        }
    }
}