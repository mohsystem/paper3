
import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;
import java.io.IOException;
import java.net.InetSocketAddress;

public class Task80 {
    public static SSLSocket createSSLClientSocket(String host, int port) throws IOException {
        SSLSocketFactory sslSocketFactory = (SSLSocketFactory) SSLSocketFactory.getDefault();
        SSLSocket sslSocket = (SSLSocket) sslSocketFactory.createSocket();
        
        // Create an IPv6 address and connect
        InetSocketAddress address = new InetSocketAddress(host, port);
        sslSocket.connect(address, 5000); // 5 second timeout
        
        return sslSocket;
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testHosts = {
            "www.google.com",
            "www.github.com",
            "www.amazon.com",
            "www.microsoft.com",
            "www.apple.com"
        };
        int[] testPorts = {443, 443, 443, 443, 443};
        
        for (int i = 0; i < testHosts.length; i++) {
            try {
                System.out.println("Test " + (i + 1) + ": Connecting to " + testHosts[i] + ":" + testPorts[i]);
                SSLSocket socket = createSSLClientSocket(testHosts[i], testPorts[i]);
                System.out.println("Successfully connected!");
                System.out.println("Remote Address: " + socket.getInetAddress());
                System.out.println("Local Address: " + socket.getLocalAddress());
                socket.close();
                System.out.println("Connection closed.\\n");
            } catch (Exception e) {
                System.out.println("Failed to connect: " + e.getMessage() + "\\n");
            }
        }
    }
}
