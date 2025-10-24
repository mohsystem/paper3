import java.net.*;
import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.Map;

public class Task91 {

    private static final int DNS_PORT = 53535; // Use a high port for testing to avoid needing root
    private static final String LISTEN_ADDR = "127.0.0.1";
    private static final int BUFFER_SIZE = 512;
    private static final Map<String, String> dnsRecords = new HashMap<>();

    static {
        // Static block to populate our simple DNS database
        dnsRecords.put("example.com.", "93.184.216.34");
        dnsRecords.put("test.local.", "192.168.1.100");
        dnsRecords.put("hello.world.", "1.1.1.1");
        dnsRecords.put("another.test.", "8.8.8.8");
        dnsRecords.put("localhost.", "127.0.0.1");
    }
    
    // Main server loop
    public void runServer() {
        System.out.println("Starting Java DNS server on " + LISTEN_ADDR + ":" + DNS_PORT);
        // Use try-with-resources to ensure the socket is closed automatically.
        try (DatagramSocket socket = new DatagramSocket(DNS_PORT, InetAddress.getByName(LISTEN_ADDR))) {
            byte[] buffer = new byte[BUFFER_SIZE];

            while (true) {
                DatagramPacket requestPacket = new DatagramPacket(buffer, buffer.length);
                
                // 1. Receive incoming DNS request
                socket.receive(requestPacket);
                System.out.println("\nReceived request from " + requestPacket.getAddress() + ":" + requestPacket.getPort());

                // Create a response packet
                byte[] response = processDnsRequest(requestPacket.getData(), requestPacket.getLength());

                if (response != null) {
                    // 5. Send the response back to the client
                    DatagramPacket responsePacket = new DatagramPacket(
                        response, response.length, requestPacket.getAddress(), requestPacket.getPort());
                    socket.send(responsePacket);
                    System.out.println("Sent response to " + requestPacket.getAddress() + ":" + requestPacket.getPort());
                }
            }
        } catch (Exception e) {
            System.err.println("Server error: " + e.getMessage());
            e.printStackTrace();
        }
    }

    // Process the DNS request and create a response
    private byte[] processDnsRequest(byte[] requestData, int requestLength) {
        // Wrap request data in ByteBuffer for easier and safer parsing
        ByteBuffer requestBuffer = ByteBuffer.wrap(requestData, 0, requestLength);

        // --- 2. Parse the received data ---
        // DNS Header (12 bytes)
        if (requestLength < 12) return null; // Invalid header
        
        // Extract transaction ID from the first 2 bytes
        short id = requestBuffer.getShort();
        
        // Extract flags and check if it is a standard query
        short flags = requestBuffer.getShort();
        boolean isStandardQuery = ((flags >> 11) & 0xF) == 0; // Check Opcode
        
        short qdcount = requestBuffer.getShort(); // Question Count

        if (!isStandardQuery || qdcount != 1) {
            System.err.println("Unsupported query type or question count != 1. Skipping.");
            return null;
        }
        
        requestBuffer.position(12); // Skip to the start of the question section

        // Parse Question section
        int[] questionLengthHolder = new int[1];
        String domainName = parseDomainName(requestBuffer, questionLengthHolder);
        
        if (domainName == null) {
            System.err.println("Failed to parse domain name.");
            return null;
        }

        // QTYPE and QCLASS (4 bytes)
        if (requestBuffer.remaining() < 4) return null; // Not enough data
        short qtype = requestBuffer.getShort();
        short qclass = requestBuffer.getShort();

        // We only handle A records (QTYPE=1) and IN class (QCLASS=1)
        if (qtype != 1 || qclass != 1) {
            System.err.println("Unsupported QTYPE or QCLASS. Skipping.");
            return null;
        }
        System.out.println("Query for: " + domainName + " (A, IN)");

        // --- 3. Resolve DNS record ---
        String resolvedIp = dnsRecords.get(domainName);

        // --- 4. Construct the DNS response ---
        ByteBuffer responseBuffer = ByteBuffer.allocate(BUFFER_SIZE);
        
        // --- Response Header ---
        responseBuffer.putShort(id); // Transaction ID
        if (resolvedIp != null) {
            responseBuffer.putShort((short) 0x8180); // Flags: Response, No error
        } else {
            responseBuffer.putShort((short) 0x8183); // Flags: Response, Name Error (NXDOMAIN)
        }
        responseBuffer.putShort((short) 1); // QDCOUNT
        responseBuffer.putShort((short) (resolvedIp != null ? 1 : 0)); // ANCOUNT
        responseBuffer.putShort((short) 0); // NSCOUNT
        responseBuffer.putShort((short) 0); // ARCOUNT

        // --- Response Question Section ---
        // Copy original question (domain name + qtype + qclass) from request
        int questionSectionStart = 12;
        int questionSectionLength = questionLengthHolder[0] + 4;
        responseBuffer.put(requestData, questionSectionStart, questionSectionLength);

        // --- Response Answer Section (if IP was found) ---
        if (resolvedIp != null) {
            System.out.println("Resolved " + domainName + " to " + resolvedIp);
            try {
                // Name (pointer to question name at offset 12)
                responseBuffer.putShort((short) 0xc00c); 
                // Type: A (1)
                responseBuffer.putShort((short) 1);
                // Class: IN (1)
                responseBuffer.putShort((short) 1);
                // TTL: 60 seconds
                responseBuffer.putInt(60);
                // RDLENGTH: 4 bytes for IPv4
                responseBuffer.putShort((short) 4);
                // RDATA: the IP address
                responseBuffer.put(InetAddress.getByName(resolvedIp).getAddress());
            } catch (UnknownHostException e) {
                System.err.println("Error parsing IP address from map: " + resolvedIp);
                return null;
            }
        } else {
             System.out.println("Could not resolve: " + domainName);
        }

        byte[] finalResponse = new byte[responseBuffer.position()];
        responseBuffer.flip();
        responseBuffer.get(finalResponse);
        
        return finalResponse;
    }
    
    // Parses a domain name from the DNS QNAME format (e.g., 3www7example3com0)
    private String parseDomainName(ByteBuffer buffer, int[] lengthHolder) {
        StringBuilder nameBuilder = new StringBuilder();
        int originalPosition = buffer.position();
        while (buffer.hasRemaining()) {
            byte length = buffer.get();
            if (length == 0) break; // End of name
            if ((length & 0xC0) == 0xC0) { // Pointer check
                 System.err.println("Pointers in request QNAME not supported.");
                 return null;
            }
            if (buffer.remaining() < length) return null; // Avoid buffer underflow
            
            byte[] label = new byte[length];
            buffer.get(label);
            nameBuilder.append(new String(label));
            nameBuilder.append(".");
        }
        lengthHolder[0] = buffer.position() - originalPosition;
        return nameBuilder.toString();
    }

    public static void main(String[] args) {
        // 5 test cases explained. To test, run this server and use a DNS client like `dig` or `nslookup`.
        // Example commands:
        // 1. dig @127.0.0.1 -p 53535 example.com
        // 2. dig @127.0.0.1 -p 53535 test.local
        // 3. dig @127.0.0.1 -p 53535 hello.world
        // 4. dig @127.0.0.1 -p 53535 not.found
        // 5. dig @127.0.0.1 -p 53535 localhost
        Task91 server = new Task91();
        server.runServer();
    }
}