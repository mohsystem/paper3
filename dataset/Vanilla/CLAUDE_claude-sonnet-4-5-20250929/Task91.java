
import java.net.*;
import java.nio.ByteBuffer;
import java.util.*;

public class Task91 {
    private static final int DNS_PORT = 5353;
    private static final Map<String, String> DNS_RECORDS = new HashMap<>();
    
    static {
        DNS_RECORDS.put("example.com", "93.184.216.34");
        DNS_RECORDS.put("test.com", "192.168.1.100");
        DNS_RECORDS.put("localhost", "127.0.0.1");
        DNS_RECORDS.put("google.com", "142.250.185.46");
        DNS_RECORDS.put("github.com", "140.82.114.4");
    }
    
    public static void startDNSServer(String ipAddress, int port) throws Exception {
        DatagramSocket socket = new DatagramSocket(port, InetAddress.getByName(ipAddress));
        System.out.println("DNS Server listening on " + ipAddress + ":" + port);
        
        byte[] buffer = new byte[512];
        DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
        
        socket.receive(packet);
        byte[] query = Arrays.copyOf(packet.getData(), packet.getLength());
        String domainName = parseDNSQuery(query);
        System.out.println("Received query for: " + domainName);
        
        String ipAddr = resolveDNSRecord(domainName);
        byte[] response = createDNSResponse(query, ipAddr);
        
        DatagramPacket responsePacket = new DatagramPacket(
            response, response.length, packet.getAddress(), packet.getPort()
        );
        socket.send(responsePacket);
        System.out.println("Sent response: " + ipAddr);
        
        socket.close();
    }
    
    public static String parseDNSQuery(byte[] query) {
        StringBuilder domain = new StringBuilder();
        int pos = 12; // Skip DNS header
        
        while (pos < query.length && query[pos] != 0) {
            int len = query[pos] & 0xFF;
            if (len == 0) break;
            
            if (domain.length() > 0) domain.append(".");
            pos++;
            
            for (int i = 0; i < len && pos < query.length; i++, pos++) {
                domain.append((char) query[pos]);
            }
        }
        
        return domain.toString();
    }
    
    public static String resolveDNSRecord(String domainName) {
        return DNS_RECORDS.getOrDefault(domainName, "0.0.0.0");
    }
    
    public static byte[] createDNSResponse(byte[] query, String ipAddress) {
        ByteBuffer response = ByteBuffer.allocate(512);
        
        // Copy transaction ID
        response.put(query[0]);
        response.put(query[1]);
        
        // Flags: Response, Standard Query, No Error
        response.put((byte) 0x81);
        response.put((byte) 0x80);
        
        // Questions count
        response.put(query[4]);
        response.put(query[5]);
        
        // Answers count
        response.put((byte) 0x00);
        response.put((byte) 0x01);
        
        // Authority and Additional RRs
        response.put((byte) 0x00);
        response.put((byte) 0x00);
        response.put((byte) 0x00);
        response.put((byte) 0x00);
        
        // Copy question section
        int pos = 12;
        while (pos < query.length && query[pos] != 0) {
            response.put(query[pos++]);
        }
        response.put((byte) 0x00); // End of domain name
        pos++;
        
        // Copy QTYPE and QCLASS
        for (int i = 0; i < 4 && pos < query.length; i++, pos++) {
            response.put(query[pos]);
        }
        
        // Answer section
        response.put((byte) 0xC0);
        response.put((byte) 0x0C); // Pointer to domain name
        
        // Type A
        response.put((byte) 0x00);
        response.put((byte) 0x01);
        
        // Class IN
        response.put((byte) 0x00);
        response.put((byte) 0x01);
        
        // TTL (300 seconds)
        response.putInt(300);
        
        // Data length
        response.put((byte) 0x00);
        response.put((byte) 0x04);
        
        // IP Address
        String[] octets = ipAddress.split("\\\\.");
        for (String octet : octets) {
            response.put((byte) Integer.parseInt(octet));
        }
        
        byte[] result = new byte[response.position()];
        response.rewind();
        response.get(result);
        return result;
    }
    
    public static void main(String[] args) {
        System.out.println("DNS Server Test Cases:");
        System.out.println("======================\\n");
        
        // Test Case 1: Parse DNS query
        System.out.println("Test 1: Parse DNS Query");
        byte[] testQuery1 = createTestQuery("example.com");
        String parsed1 = parseDNSQuery(testQuery1);
        System.out.println("Parsed domain: " + parsed1 + "\\n");
        
        // Test Case 2: Resolve DNS record
        System.out.println("Test 2: Resolve DNS Record");
        String resolved = resolveDNSRecord("google.com");
        System.out.println("Resolved IP: " + resolved + "\\n");
        
        // Test Case 3: Resolve unknown domain
        System.out.println("Test 3: Resolve Unknown Domain");
        String unknownResolved = resolveDNSRecord("unknown.com");
        System.out.println("Resolved IP: " + unknownResolved + "\\n");
        
        // Test Case 4: Create DNS response
        System.out.println("Test 4: Create DNS Response");
        byte[] response = createDNSResponse(testQuery1, "93.184.216.34");
        System.out.println("Response created with length: " + response.length + "\\n");
        
        // Test Case 5: Multiple domain parsing
        System.out.println("Test 5: Parse Multiple Domains");
        String[] domains = {"localhost", "test.com", "github.com"};
        for (String domain : domains) {
            byte[] query = createTestQuery(domain);
            String parsed = parseDNSQuery(query);
            String ip = resolveDNSRecord(parsed);
            System.out.println(domain + " -> " + parsed + " -> " + ip);
        }
    }
    
    private static byte[] createTestQuery(String domain) {
        ByteBuffer buffer = ByteBuffer.allocate(512);
        buffer.put((byte) 0x12);
        buffer.put((byte) 0x34);
        buffer.put((byte) 0x01);
        buffer.put((byte) 0x00);
        buffer.put((byte) 0x00);
        buffer.put((byte) 0x01);
        buffer.putInt(0);
        buffer.putInt(0);
        
        String[] labels = domain.split("\\\\.");
        for (String label : labels) {
            buffer.put((byte) label.length());
            buffer.put(label.getBytes());
        }
        buffer.put((byte) 0x00);
        buffer.put((byte) 0x00);
        buffer.put((byte) 0x01);
        buffer.put((byte) 0x00);
        buffer.put((byte) 0x01);
        
        byte[] result = new byte[buffer.position()];
        buffer.rewind();
        buffer.get(result);
        return result;
    }
}
