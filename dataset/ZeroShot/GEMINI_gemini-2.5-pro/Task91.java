import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class Task91 {

    private static final int PORT = 53530;
    private static final String IP_ADDRESS = "127.0.0.1";
    private static final int BUFFER_SIZE = 512;
    private static final Map<String, String> dnsRecords = new HashMap<>();

    static {
        // Pre-configured DNS records for testing
        dnsRecords.put("test1.example.com.", "192.0.2.1");
        dnsRecords.put("test2.example.com.", "198.51.100.2");
        dnsRecords.put("secure.example.com.", "203.0.113.3");
    }

    public static void startServer() {
        System.out.println("Java DNS Server starting on " + IP_ADDRESS + ":" + PORT);
        try (DatagramSocket socket = new DatagramSocket(PORT, InetAddress.getByName(IP_ADDRESS))) {
            while (true) {
                byte[] buffer = new byte[BUFFER_SIZE];
                DatagramPacket requestPacket = new DatagramPacket(buffer, buffer.length);
                socket.receive(requestPacket);

                byte[] responseData = handleDnsRequest(requestPacket.getData(), requestPacket.getLength());

                if (responseData != null) {
                    DatagramPacket responsePacket = new DatagramPacket(
                            responseData, responseData.length, requestPacket.getAddress(), requestPacket.getPort());
                    socket.send(responsePacket);
                }
            }
        } catch (IOException e) {
            System.err.println("Server error: " + e.getMessage());
            e.printStackTrace();
        }
    }

    private static byte[] handleDnsRequest(byte[] requestData, int length) {
        if (length < 12) return null; // DNS header is 12 bytes

        ByteBuffer requestBuffer = ByteBuffer.wrap(requestData, 0, length);
        
        // --- Parse Header ---
        short id = requestBuffer.getShort();
        short flags = requestBuffer.getShort();
        short qdcount = requestBuffer.getShort();
        // Ignore AN, NS, AR counts for a simple request
        requestBuffer.position(12);

        // Security: Only handle standard queries with one question
        if ((flags & 0x7800) != 0 || qdcount != 1) {
            return buildErrorResponse(id, 4); // RCODE 4: Not Implemented
        }
        
        // --- Parse Question ---
        StringBuilder domainNameBuilder = new StringBuilder();
        int[] qnameLen = new int[1];
        String domainName = parseDomainName(requestBuffer, qnameLen);

        if (domainName == null) {
             return buildErrorResponse(id, 1); // RCODE 1: Format Error
        }
        
        short qtype = requestBuffer.getShort();
        short qclass = requestBuffer.getShort();

        // --- Resolve and Build Response ---
        String ipAddress = dnsRecords.get(domainName);

        if (ipAddress != null && qtype == 1 && qclass == 1) { // A record, IN class
            try {
                int questionLength = qnameLen[0] + 4; // QNAME length + QTYPE + QCLASS
                ByteBuffer responseBuffer = ByteBuffer.allocate(BUFFER_SIZE);
                
                // Header
                responseBuffer.putShort(id);
                responseBuffer.putShort((short) 0x8180); // Response, No error
                responseBuffer.putShort((short) 1);     // QDCOUNT
                responseBuffer.putShort((short) 1);     // ANCOUNT
                responseBuffer.putShort((short) 0);     // NSCOUNT
                responseBuffer.putShort((short) 0);     // ARCOUNT

                // Question (copy from request)
                responseBuffer.put(requestData, 12, questionLength);

                // Answer
                responseBuffer.putShort((short) 0xC00C); // Name pointer to offset 12
                responseBuffer.putShort((short) 1);      // TYPE A
                responseBuffer.putShort((short) 1);      // CLASS IN
                responseBuffer.putInt(60);               // TTL 60 seconds
                responseBuffer.putShort((short) 4);      // RDLENGTH 4 bytes for IPv4
                responseBuffer.put(InetAddress.getByName(ipAddress).getAddress());

                byte[] response = new byte[responseBuffer.position()];
                responseBuffer.flip();
                responseBuffer.get(response);
                return response;

            } catch (Exception e) {
                 return buildErrorResponse(id, 2); // RCODE 2: Server Failure
            }
        } else {
            // Name not found or unsupported type/class
            return buildNxDomainResponse(id, requestData, 12, qnameLen[0] + 4);
        }
    }
    
    private static String parseDomainName(ByteBuffer buffer, int[] outLength) {
        StringBuilder name = new StringBuilder();
        int startPos = buffer.position();
        byte len;
        while ((len = buffer.get()) != 0) {
            if ((len & 0xC0) == 0xC0) { // Pointer
                // Not handling pointers in requests for this simple server
                return null;
            }
            if (name.length() > 0) {
                name.append('.');
            }
            byte[] label = new byte[len];
            buffer.get(label);
            name.append(new String(label));
        }
        name.append('.');
        outLength[0] = buffer.position() - startPos;
        return name.toString();
    }

    private static byte[] buildErrorResponse(short id, int rcode) {
        ByteBuffer buffer = ByteBuffer.allocate(12);
        buffer.putShort(id);
        buffer.putShort((short) (0x8180 | rcode)); // Set response flag and RCODE
        buffer.putShort((short) 0); // QDCOUNT
        buffer.putShort((short) 0); // ANCOUNT
        // NSCOUNT, ARCOUNT are 0
        return buffer.array();
    }
    
    private static byte[] buildNxDomainResponse(short id, byte[] request, int qStart, int qLen) {
        ByteBuffer buffer = ByteBuffer.allocate(12 + qLen);
        buffer.putShort(id);
        buffer.putShort((short) 0x8183); // Response, RCODE 3 (NXDOMAIN)
        buffer.putShort((short) 1); // QDCOUNT
        buffer.putShort((short) 0); // ANCOUNT
        buffer.putShort((short) 0); // NSCOUNT
        buffer.putShort((short) 0); // ARCOUNT
        buffer.put(request, qStart, qLen); // Copy original question
        return buffer.array();
    }
    
    // --- Main method with test cases ---
    public static void main(String[] args) {
        // Run server in a separate thread
        ExecutorService executor = Executors.newSingleThreadExecutor();
        executor.submit(Task91::startServer);

        // Allow server to start
        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }

        System.out.println("--- Running 5 Test Cases ---");
        System.out.println("You can also test manually with: dig @127.0.0.1 -p 53530 <domain>");

        String[] testDomains = {
            "test1.example.com",     // 1. Known domain
            "test2.example.com",     // 2. Another known domain
            "secure.example.com",    // 3. Third known domain
            "unknown.example.com",   // 4. Unknown domain (expect NXDOMAIN)
            "test1.example.com"      // 5. Test with MX type (expect NXDOMAIN/no answer)
        };
        int[] testTypes = {1, 1, 1, 1, 15}; // 1=A, 15=MX

        for (int i = 0; i < testDomains.length; i++) {
            try {
                System.out.println("\n--- Test Case " + (i + 1) + ": Querying for " + testDomains[i] + " ---");
                InetAddress result = InetAddress.getByName(testDomains[i]);
                System.out.println("Resolved " + testDomains[i] + " to " + result.getHostAddress());
            } catch (Exception e) {
                System.out.println("Could not resolve " + testDomains[i] + ": " + e.getMessage());
            }
        }
        
        System.setProperty("sun.net.spi.nameservice.provider.1", "dns,sun");
        System.setProperty("sun.net.spi.nameservice.nameservers", IP_ADDRESS);
        System.setProperty("sun.net.spi.nameservice.nameserver.ports", String.valueOf(PORT));

        executor.shutdownNow(); // Stop the server
        System.out.println("\n--- Test cases finished. Server stopped. ---");
    }
}