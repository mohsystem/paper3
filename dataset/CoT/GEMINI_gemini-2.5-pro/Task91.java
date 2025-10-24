import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.stream.Collectors;

public class Task91 {

    private static final int PORT = 5353;
    private static final String IP_ADDRESS = "127.0.0.1";
    private static final int BUFFER_SIZE = 512;
    private static final Map<String, String> dnsRecords = new HashMap<>();

    static {
        // 5 Test cases records
        dnsRecords.put("test1.local.", "192.168.1.1");
        dnsRecords.put("test2.local.", "192.168.1.2");
        dnsRecords.put("example.com.", "93.184.216.34");
        dnsRecords.put("my-server.net.", "10.0.0.1");
        dnsRecords.put("localhost.", "127.0.0.1");
    }

    public static void startDnsServer(String ip, int port) {
        try (DatagramSocket socket = new DatagramSocket(port, InetAddress.getByName(ip))) {
            System.out.println("DNS server listening on " + ip + ":" + port);

            while (true) {
                byte[] buffer = new byte[BUFFER_SIZE];
                DatagramPacket requestPacket = new DatagramPacket(buffer, buffer.length);
                socket.receive(requestPacket);

                InetAddress clientAddress = requestPacket.getAddress();
                int clientPort = requestPacket.getPort();

                System.out.println("Received request from " + clientAddress.getHostAddress());

                byte[] responseData = buildDnsResponse(requestPacket.getData(), requestPacket.getLength());
                if (responseData != null) {
                    DatagramPacket responsePacket = new DatagramPacket(responseData, responseData.length, clientAddress, clientPort);
                    socket.send(responsePacket);
                }
            }
        } catch (SocketException e) {
            System.err.println("Socket error: " + e.getMessage());
        } catch (IOException e) {
            System.err.println("I/O error: " + e.getMessage());
        }
    }

    private static String parseDomainName(ByteBuffer buffer) {
        StringBuilder domainName = new StringBuilder();
        int length = buffer.get() & 0xFF;
        while (length > 0) {
            if (domainName.length() > 0) {
                domainName.append(".");
            }
            byte[] label = new byte[length];
            buffer.get(label);
            domainName.append(new String(label));
            length = buffer.get() & 0xFF;
        }
        domainName.append(".");
        return domainName.toString();
    }

    private static byte[] buildDnsResponse(byte[] requestData, int requestLength) {
        if (requestLength < 12) {
            return null; // Invalid request
        }
        ByteBuffer requestBuffer = ByteBuffer.wrap(requestData);

        // --- Parse Request ---
        // Header (first 12 bytes)
        byte[] header = new byte[12];
        requestBuffer.get(header);

        // Move position to start of question section
        requestBuffer.position(12);
        String domainName = parseDomainName(requestBuffer);
        
        // QTYPE and QCLASS (4 bytes)
        short qtype = requestBuffer.getShort();
        short qclass = requestBuffer.getShort();
        
        // Only handle A records for IN class
        if (qtype != 1 || qclass != 1 || !dnsRecords.containsKey(domainName)) {
            System.out.println("Unsupported query or unknown domain: " + domainName);
            // In a real server, we would build a NXDOMAIN response
            return null; 
        }

        System.out.println("Query for: " + domainName);

        // --- Build Response ---
        ByteBuffer responseBuffer = ByteBuffer.allocate(BUFFER_SIZE);

        // Copy header from request
        responseBuffer.put(header);

        // Set response flags (QR=1, Opcode=0, AA=1, RCODE=0)
        responseBuffer.putShort(2, (short) 0x8400); 

        // Set answer count to 1
        responseBuffer.putShort(6, (short) 1);

        // Copy Question section from request
        int questionLength = domainName.length() + 1 + 4; // name + null terminator + qtype/qclass
        responseBuffer.put(requestData, 12, questionLength);
        
        // --- Add Answer Section ---
        // Name (pointer to the name in the question section)
        responseBuffer.putShort((short) 0xC00C); 
        
        // Type (A record)
        responseBuffer.putShort((short) 1);
        
        // Class (IN)
        responseBuffer.putShort((short) 1);
        
        // TTL (e.g., 60 seconds)
        responseBuffer.putInt(60); 

        // RDLENGTH (4 bytes for IPv4)
        responseBuffer.putShort((short) 4);
        
        // RDATA (The IP address)
        String ip = dnsRecords.get(domainName);
        try {
            responseBuffer.put(InetAddress.getByName(ip).getAddress());
        } catch (java.net.UnknownHostException e) {
            return null; // Should not happen with valid IPs
        }

        int responseSize = responseBuffer.position();
        byte[] response = new byte[responseSize];
        responseBuffer.flip();
        responseBuffer.get(response);

        return response;
    }

    public static void main(String[] args) {
        System.out.println("Starting Java DNS Server...");
        System.out.println("You can test this server with commands like:");
        System.out.println("  dig @127.0.0.1 -p 5353 test1.local");
        System.out.println("  nslookup example.com 127.0.0.1");
        System.out.println("----------------------------------------------");
        startDnsServer(IP_ADDRESS, PORT);
    }
}