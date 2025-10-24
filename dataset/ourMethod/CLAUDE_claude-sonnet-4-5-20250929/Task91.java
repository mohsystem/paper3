
import java.net.*;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;

public class Task91 {
    private static final int MAX_PACKET_SIZE = 512;
    private static final int MAX_LABEL_LENGTH = 63;
    private static final int MAX_DOMAIN_LENGTH = 253;
    private static final int DNS_HEADER_SIZE = 12;
    private static final Map<String, String> DNS_RECORDS = new ConcurrentHashMap<>();
    
    static {
        DNS_RECORDS.put("example.com", "93.184.216.34");
        DNS_RECORDS.put("test.com", "192.0.2.1");
        DNS_RECORDS.put("localhost", "127.0.0.1");
    }
    
    public static class DNSRequest {
        private final int id;
        private final String domain;
        private final boolean valid;
        
        public DNSRequest(int id, String domain, boolean valid) {
            this.id = id;
            this.domain = domain;
            this.valid = valid;
        }
        
        public int getId() { return id; }
        public String getDomain() { return domain; }
        public boolean isValid() { return valid; }
    }
    
    public static DNSRequest parseDNSRequest(byte[] data, int length) {
        if (data == null || length < DNS_HEADER_SIZE || length > MAX_PACKET_SIZE) {
            return new DNSRequest(0, "", false);
        }
        
        try {
            ByteBuffer buffer = ByteBuffer.wrap(data, 0, length);
            int id = buffer.getShort() & 0xFFFF;
            int flags = buffer.getShort() & 0xFFFF;
            int qdCount = buffer.getShort() & 0xFFFF;
            buffer.getShort(); // anCount
            buffer.getShort(); // nsCount
            buffer.getShort(); // arCount
            
            if ((flags & 0x8000) != 0 || qdCount != 1) {
                return new DNSRequest(id, "", false);
            }
            
            StringBuilder domain = new StringBuilder();
            int position = DNS_HEADER_SIZE;
            int totalLength = 0;
            
            while (position < length) {
                int labelLen = data[position] & 0xFF;
                if (labelLen == 0) {
                    position++;
                    break;
                }
                
                if (labelLen > MAX_LABEL_LENGTH) {
                    return new DNSRequest(id, "", false);
                }
                
                if ((labelLen & 0xC0) == 0xC0) {
                    return new DNSRequest(id, "", false);
                }
                
                position++;
                if (position + labelLen > length) {
                    return new DNSRequest(id, "", false);
                }
                
                totalLength += labelLen + 1;
                if (totalLength > MAX_DOMAIN_LENGTH) {
                    return new DNSRequest(id, "", false);
                }
                
                if (domain.length() > 0) {
                    domain.append('.');
                }
                
                for (int i = 0; i < labelLen; i++) {
                    char c = (char) data[position++];
                    if (!isValidDomainChar(c)) {
                        return new DNSRequest(id, "", false);
                    }
                    domain.append(c);
                }
            }
            
            if (position + 4 > length) {
                return new DNSRequest(id, "", false);
            }
            
            int qType = ((data[position] & 0xFF) << 8) | (data[position + 1] & 0xFF);
            int qClass = ((data[position + 2] & 0xFF) << 8) | (data[position + 3] & 0xFF);
            
            if (qType != 1 || qClass != 1) {
                return new DNSRequest(id, "", false);
            }
            
            return new DNSRequest(id, domain.toString().toLowerCase(), true);
            
        } catch (Exception e) {
            return new DNSRequest(0, "", false);
        }
    }
    
    private static boolean isValidDomainChar(char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || 
               (c >= '0' && c <= '9') || c == '-' || c == '_';
    }
    
    public static byte[] createDNSResponse(DNSRequest request, String ipAddress) {
        if (!request.isValid() || ipAddress == null || ipAddress.isEmpty()) {
            return createErrorResponse(request.getId());
        }
        
        String[] octets = ipAddress.split("\\\\.");
        if (octets.length != 4) {
            return createErrorResponse(request.getId());
        }
        
        byte[] ipBytes = new byte[4];
        try {
            for (int i = 0; i < 4; i++) {
                int octet = Integer.parseInt(octets[i]);
                if (octet < 0 || octet > 255) {
                    return createErrorResponse(request.getId());
                }
                ipBytes[i] = (byte) octet;
            }
        } catch (NumberFormatException e) {
            return createErrorResponse(request.getId());
        }
        
        ByteBuffer buffer = ByteBuffer.allocate(MAX_PACKET_SIZE);
        buffer.putShort((short) request.getId());
        buffer.putShort((short) 0x8180); // Standard query response, no error
        buffer.putShort((short) 1); // Questions: 1
        buffer.putShort((short) 1); // Answers: 1
        buffer.putShort((short) 0); // Authority RRs: 0
        buffer.putShort((short) 0); // Additional RRs: 0
        
        String[] labels = request.getDomain().split("\\\\.");
        for (String label : labels) {
            if (label.isEmpty() || label.length() > MAX_LABEL_LENGTH) {
                return createErrorResponse(request.getId());
            }
            buffer.put((byte) label.length());
            buffer.put(label.getBytes(StandardCharsets.UTF_8));
        }
        buffer.put((byte) 0);
        
        buffer.putShort((short) 1); // Type A
        buffer.putShort((short) 1); // Class IN
        
        buffer.putShort((short) 0xC00C); // Pointer to domain name
        buffer.putShort((short) 1); // Type A
        buffer.putShort((short) 1); // Class IN
        buffer.putInt(300); // TTL: 5 minutes
        buffer.putShort((short) 4); // Data length
        buffer.put(ipBytes);
        
        byte[] response = new byte[buffer.position()];
        buffer.rewind();
        buffer.get(response);
        return response;
    }
    
    private static byte[] createErrorResponse(int id) {
        ByteBuffer buffer = ByteBuffer.allocate(DNS_HEADER_SIZE);
        buffer.putShort((short) id);
        buffer.putShort((short) 0x8183); // Response with format error
        buffer.putShort((short) 0);
        buffer.putShort((short) 0);
        buffer.putShort((short) 0);
        buffer.putShort((short) 0);
        return buffer.array();
    }
    
    public static String resolveDomain(String domain) {
        if (domain == null || domain.isEmpty() || domain.length() > MAX_DOMAIN_LENGTH) {
            return null;
        }
        return DNS_RECORDS.get(domain.toLowerCase());
    }
    
    public static void runDNSServer(String ipAddress, int port, int timeoutMs) {
        if (port < 1 || port > 65535) {
            throw new IllegalArgumentException("Port must be between 1 and 65535");
        }
        
        if (timeoutMs <= 0) {
            throw new IllegalArgumentException("Timeout must be positive");
        }
        
        DatagramSocket socket = null;
        try {
            InetAddress address = InetAddress.getByName(ipAddress);
            socket = new DatagramSocket(port, address);
            socket.setSoTimeout(timeoutMs);
            
            byte[] receiveBuffer = new byte[MAX_PACKET_SIZE];
            DatagramPacket receivePacket = new DatagramPacket(receiveBuffer, receiveBuffer.length);
            
            socket.receive(receivePacket);
            
            DNSRequest request = parseDNSRequest(receivePacket.getData(), receivePacket.getLength());
            
            if (request.isValid()) {
                String resolvedIP = resolveDomain(request.getDomain());
                byte[] response = createDNSResponse(request, resolvedIP);
                
                DatagramPacket sendPacket = new DatagramPacket(
                    response, response.length,
                    receivePacket.getAddress(), receivePacket.getPort()
                );
                socket.send(sendPacket);
            }
            
        } catch (SocketTimeoutException e) {
            // Timeout is expected for testing
        } catch (Exception e) {
            throw new RuntimeException("DNS server error: " + e.getMessage());
        } finally {
            if (socket != null && !socket.isClosed()) {
                socket.close();
            }
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Parse valid DNS request");
        byte[] validRequest = createTestDNSRequest(0x1234, "example.com");
        DNSRequest req1 = parseDNSRequest(validRequest, validRequest.length);
        System.out.println("Valid: " + req1.isValid() + ", Domain: " + req1.getDomain());
        
        System.out.println("\\nTest Case 2: Parse invalid DNS request (too short)");
        byte[] invalidRequest = new byte[5];
        DNSRequest req2 = parseDNSRequest(invalidRequest, invalidRequest.length);
        System.out.println("Valid: " + req2.isValid());
        
        System.out.println("\\nTest Case 3: Resolve known domain");
        String ip = resolveDomain("example.com");
        System.out.println("Resolved IP: " + ip);
        
        System.out.println("\\nTest Case 4: Resolve unknown domain");
        String unknownIP = resolveDomain("unknown.com");
        System.out.println("Resolved IP: " + unknownIP);
        
        System.out.println("\\nTest Case 5: Create DNS response");
        DNSRequest req5 = new DNSRequest(0x5678, "test.com", true);
        byte[] response = createDNSResponse(req5, "192.0.2.1");
        System.out.println("Response created, length: " + response.length);
    }
    
    private static byte[] createTestDNSRequest(int id, String domain) {
        ByteBuffer buffer = ByteBuffer.allocate(MAX_PACKET_SIZE);
        buffer.putShort((short) id);
        buffer.putShort((short) 0x0100); // Standard query
        buffer.putShort((short) 1); // Questions: 1
        buffer.putShort((short) 0);
        buffer.putShort((short) 0);
        buffer.putShort((short) 0);
        
        String[] labels = domain.split("\\\\.");
        for (String label : labels) {
            buffer.put((byte) label.length());
            buffer.put(label.getBytes(StandardCharsets.UTF_8));
        }
        buffer.put((byte) 0);
        buffer.putShort((short) 1); // Type A
        buffer.putShort((short) 1); // Class IN
        
        byte[] result = new byte[buffer.position()];
        buffer.rewind();
        buffer.get(result);
        return result;
    }
}
