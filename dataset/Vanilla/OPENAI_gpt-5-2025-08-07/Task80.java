import javax.net.ssl.SSLParameters;
import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;
import java.net.*;

public class Task80 {
    public static SSLSocket createIPv6SSLSocket(String host, int port) throws Exception {
        InetAddress[] all = InetAddress.getAllByName(host);
        Inet6Address v6 = null;
        for (InetAddress a : all) {
            if (a instanceof Inet6Address) {
                v6 = (Inet6Address) a;
                break;
            }
        }
        if (v6 == null) {
            throw new UnknownHostException("No IPv6 address found for host: " + host);
        }
        Socket underlying = new Socket();
        underlying.connect(new InetSocketAddress(v6, port), 10000);
        underlying.setSoTimeout(10000);

        SSLSocketFactory factory = (SSLSocketFactory) SSLSocketFactory.getDefault();
        SSLSocket ssl = (SSLSocket) factory.createSocket(underlying, host, port, true);

        SSLParameters params = ssl.getSSLParameters();
        params.setEndpointIdentificationAlgorithm("HTTPS");
        ssl.setSSLParameters(params);

        ssl.startHandshake();
        return ssl;
    }

    public static void main(String[] args) {
        String[] hosts = new String[] {
                "google.com",
                "cloudflare.com",
                "example.com",
                "wikipedia.org",
                "ietf.org"
        };
        for (String h : hosts) {
            try {
                SSLSocket s = createIPv6SSLSocket(h, 443);
                System.out.println("Connected to " + h + " via IPv6 using " + s.getSession().getProtocol() + " " + s.getSession().getCipherSuite());
                s.close();
            } catch (Exception e) {
                System.out.println("Failed to connect to " + h + ": " + e);
            }
        }
    }
}