import javax.net.ssl.*;
import javax.net.ssl.SSLParameters;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.SNIHostName;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.util.Arrays;

public class Task105 {
    public static String connectTLS(String host, int port, int timeoutMillis) {
        SSLSocket sslSocket = null;
        Socket plain = null;
        try {
            SSLContext ctx = SSLContext.getInstance("TLS");
            ctx.init(null, null, null);
            SSLSocketFactory factory = ctx.getSocketFactory();

            plain = new Socket();
            plain.connect(new InetSocketAddress(host, port), timeoutMillis);
            plain.setSoTimeout(timeoutMillis);

            sslSocket = (SSLSocket) factory.createSocket(plain, host, port, true);

            SSLParameters params = sslSocket.getSSLParameters();
            try {
                params.setServerNames(Arrays.asList(new SNIHostName(host)));
            } catch (IllegalArgumentException ignored) { }
            params.setEndpointIdentificationAlgorithm("HTTPS");
            sslSocket.setSSLParameters(params);

            sslSocket.startHandshake();

            SSLSession session = sslSocket.getSession();
            String protocol = session.getProtocol();
            String cipher = session.getCipherSuite();
            String subject = "";
            String issuer = "";

            try {
                Certificate[] certs = session.getPeerCertificates();
                if (certs != null && certs.length > 0 && certs[0] instanceof X509Certificate) {
                    X509Certificate x = (X509Certificate) certs[0];
                    subject = x.getSubjectX500Principal().getName();
                    issuer = x.getIssuerX500Principal().getName();
                }
            } catch (SSLPeerUnverifiedException e) {
                subject = "unverified";
                issuer = "unverified";
            }

            return "OK " + host + ":" + port + " protocol=" + protocol + " cipher=" + cipher + " subject=" + subject + " issuer=" + issuer;
        } catch (Exception e) {
            return "ERROR: " + e.getClass().getSimpleName() + ": " + e.getMessage();
        } finally {
            try {
                if (sslSocket != null) sslSocket.close();
            } catch (IOException ignored) { }
            try {
                if (plain != null && !plain.isClosed()) plain.close();
            } catch (IOException ignored) { }
        }
    }

    public static void main(String[] args) {
        String[] hosts = {
                "www.google.com",
                "www.cloudflare.com",
                "www.github.com",
                "www.stackoverflow.com",
                "expired.badssl.com"
        };
        int port = 443;
        for (String h : hosts) {
            String res = connectTLS(h, port, 8000);
            System.out.println(res);
        }
    }
}