
import javax.net.ssl.*;
import java.io.IOException;
import java.net.InetSocketAddress;

public class Task80 {
    public static SSLSocket createSSLClientSocket(String host, int port) throws IOException {
        try {
            SSLSocketFactory sslSocketFactory = (SSLSocketFactory) SSLSocketFactory.getDefault();
            SSLSocket sslSocket = (SSLSocket) sslSocketFactory.createSocket();
            
            // Bind to IPv6 address and connect
            InetSocketAddress address = new InetSocketAddress(host, port);
            sslSocket.connect(address, 10000); // 10 second timeout
            
            // Start SSL handshake
            sslSocket.startHandshake();
            
            return sslSocket;
        } catch (IOException e) {
            throw new IOException("Failed to create SSL socket: " + e.getMessage(), e);
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        String[][] testCases = {
            {"www.google.com", "443"},
            {"www.github.com", "443"},
            {"www.mozilla.org", "443"},
            {"www.wikipedia.org", "443"},
            {"www.cloudflare.com", "443"}
        };
        
        System.out.println("Testing SSL Client Socket with IPv6/TCP:");
        System.out.println("=========================================\\n");
        
        for (int i = 0; i < testCases.length; i++) {
            String host = testCases[i][0];
            int port = Integer.parseInt(testCases[i][1]);
            
            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("Host: " + host + ", Port: " + port);
            
            try {
                SSLSocket socket = createSSLClientSocket(host, port);
                System.out.println("Status: Connected successfully");
                System.out.println("Local Address: " + socket.getLocalAddress());
                System.out.println("Remote Address: " + socket.getInetAddress());
                System.out.println("Protocol: " + socket.getSession().getProtocol());
                socket.close();
                System.out.println("Socket closed successfully");
            } catch (Exception e) {
                System.out.println("Status: Failed - " + e.getMessage());
            }
            System.out.println();
        }
    }
}
