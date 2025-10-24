
import java.net.*;
import java.nio.ByteBuffer;
import java.util.*;

public class Task91 {
    private static final int DNS_PORT = 5353;
    private static final Map<String, String> DNS_RECORDS = new HashMap<>();
    
    static {
        DNS_RECORDS.put("example.com", "93.184.216.34");
        DNS_RECORDS.put("test.local", "127.0.0.1");
        DNS_RECORDS.put("localhost", "127.0.0.1");
        DNS_RECORDS.put("google.com", "142.250.185.78");
        DNS_RECORDS.put("github.com", "140.82.121.3");
    }
    
    public static void startDNSServer(String ipAddress, int port) {
        try (DatagramSocket socket = new DatagramSocket(port, InetAddress.getByName(ipAddress))) {
            System.out.println("DNS Server listening on " + ipAddress + ":" + port);
            byte[] buffer = new byte[512];
            
            while (true) {
                DatagramPacket request = new DatagramPacket(buffer, buffer.length);
                socket.receive(request);
                
                byte[] response = processDNSRequest(request.getData(), request.getLength());
                
                DatagramPacket responsePacket = new DatagramPacket(
                    response, response.length, 
                    request.getAddress(), request.getPort()
                );
                socket.send(responsePacket);
                
                System.out.println("Response sent to " + request.getAddress() + ":" + request.getPort());
            }
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
    
    public static byte[] processDNSRequest(byte[] data, int length) {
        try {
            ByteBuffer buffer = ByteBuffer.wrap(data, 0, length);
            
            short transactionId = buffer.getShort();
            short flags = buffer.getShort();
            short questions = buffer.getShort();
            short answerRRs = buffer.getShort();
            short authorityRRs = buffer.getShort();
            short additionalRRs = buffer.getShort();
            
            String domainName = parseDomainName(buffer);
            short queryType = buffer.getShort();
            short queryClass = buffer.getShort();
            
            System.out.println("Query for: " + domainName);
            
            return buildDNSResponse(transactionId, domainName, queryType, queryClass);
        } catch (Exception e) {
            return new byte[0];
        }
    }
    
    private static String parseDomainName(ByteBuffer buffer) {
        StringBuilder domain = new StringBuilder();
        int length;
        
        while ((length = buffer.get() & 0xFF) != 0) {
            if (domain.length() > 0) domain.append('.');
            for (int i = 0; i < length; i++) {
                domain.append((char) buffer.get());
            }
        }
        
        return domain.toString();
    }
    
    private static byte[] buildDNSResponse(short transactionId, String domain, short queryType, short queryClass) {
        ByteBuffer response = ByteBuffer.allocate(512);
        
        response.putShort(transactionId);
        response.putShort((short) 0x8180);
        response.putShort((short) 1);
        response.putShort((short) 1);
        response.putShort((short) 0);
        response.putShort((short) 0);
        
        encodeDomainName(response, domain);
        response.putShort(queryType);
        response.putShort(queryClass);
        
        encodeDomainName(response, domain);
        response.putShort(queryType);
        response.putShort(queryClass);
        response.putInt(300);
        
        String ipAddress = DNS_RECORDS.getOrDefault(domain, "0.0.0.0");
        String[] parts = ipAddress.split("\\\\.");
        response.putShort((short) 4);
        for (String part : parts) {
            response.put((byte) Integer.parseInt(part));
        }
        
        byte[] result = new byte[response.position()];
        response.rewind();
        response.get(result);
        return result;
    }
    
    private static void encodeDomainName(ByteBuffer buffer, String domain) {
        String[] labels = domain.split("\\\\.");
        for (String label : labels) {
            buffer.put((byte) label.length());
            buffer.put(label.getBytes());
        }
        buffer.put((byte) 0);
    }
    
    public static void main(String[] args) {
        System.out.println("=== DNS Server Test Cases ===\\n");
        
        Thread serverThread = new Thread(() -> startDNSServer("127.0.0.1", DNS_PORT));
        serverThread.setDaemon(true);
        serverThread.start();
        
        try {
            Thread.sleep(1000);
            
            String[] testDomains = {"example.com", "test.local", "localhost", "google.com", "github.com"};
            
            for (String domain : testDomains) {
                testDNSQuery(domain, DNS_PORT);
                Thread.sleep(500);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    private static void testDNSQuery(String domain, int port) {
        try (DatagramSocket socket = new DatagramSocket()) {
            socket.setSoTimeout(2000);
            
            byte[] query = buildDNSQuery(domain);
            DatagramPacket packet = new DatagramPacket(
                query, query.length,
                InetAddress.getByName("127.0.0.1"), port
            );
            
            System.out.println("Test: Querying " + domain);
            socket.send(packet);
            
            byte[] buffer = new byte[512];
            DatagramPacket response = new DatagramPacket(buffer, buffer.length);
            socket.receive(response);
            
            System.out.println("Success: Received response for " + domain + "\\n");
        } catch (Exception e) {
            System.err.println("Failed: " + e.getMessage() + "\\n");
        }
    }
    
    private static byte[] buildDNSQuery(String domain) {
        ByteBuffer query = ByteBuffer.allocate(512);
        query.putShort((short) 0x1234);
        query.putShort((short) 0x0100);
        query.putShort((short) 1);
        query.putShort((short) 0);
        query.putShort((short) 0);
        query.putShort((short) 0);
        
        encodeDomainName(query, domain);
        query.putShort((short) 1);
        query.putShort((short) 1);
        
        byte[] result = new byte[query.position()];
        query.rewind();
        query.get(result);
        return result;
    }
}
