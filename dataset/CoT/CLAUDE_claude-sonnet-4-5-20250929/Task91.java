
import java.net.*;
import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.Map;

public class Task91 {
    private static final int DNS_HEADER_SIZE = 12;
    private static final int MAX_BUFFER_SIZE = 512;
    private static Map<String, String> dnsRecords = new HashMap<>();
    
    static {
        // Initialize some DNS records for testing
        dnsRecords.put("example.com", "93.184.216.34");
        dnsRecords.put("test.com", "192.0.2.1");
        dnsRecords.put("localhost", "127.0.0.1");
        dnsRecords.put("google.com", "172.217.164.46");
        dnsRecords.put("github.com", "140.82.121.4");
    }
    
    public static void startDNSServer(String ipAddress, int port) {
        DatagramSocket socket = null;
        try {
            InetAddress address = InetAddress.getByName(ipAddress);
            socket = new DatagramSocket(port, address);
            socket.setSoTimeout(30000); // 30 second timeout
            
            System.out.println("DNS Server started on " + ipAddress + ":" + port);
            
            byte[] buffer = new byte[MAX_BUFFER_SIZE];
            
            while (true) {
                try {
                    DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
                    socket.receive(packet);
                    
                    byte[] requestData = new byte[packet.getLength()];
                    System.arraycopy(packet.getData(), 0, requestData, 0, packet.getLength());
                    
                    byte[] response = processDNSRequest(requestData);
                    
                    if (response != null && response.length > 0) {
                        DatagramPacket responsePacket = new DatagramPacket(
                            response, response.length, 
                            packet.getAddress(), packet.getPort()
                        );
                        socket.send(responsePacket);
                        System.out.println("Response sent to " + packet.getAddress() + ":" + packet.getPort());
                    }
                } catch (SocketTimeoutException e) {
                    System.out.println("Socket timeout, continuing...");
                    break;
                } catch (Exception e) {
                    System.err.println("Error processing request: " + e.getMessage());
                }
            }
        } catch (Exception e) {
            System.err.println("Server error: " + e.getMessage());
        } finally {
            if (socket != null && !socket.isClosed()) {
                socket.close();
            }
        }
    }
    
    private static byte[] processDNSRequest(byte[] requestData) {
        if (requestData == null || requestData.length < DNS_HEADER_SIZE) {
            return null;
        }
        
        try {
            ByteBuffer buffer = ByteBuffer.wrap(requestData);
            
            // Parse DNS header
            short transactionId = buffer.getShort();
            short flags = buffer.getShort();
            short questions = buffer.getShort();
            short answerRRs = buffer.getShort();
            short authorityRRs = buffer.getShort();
            short additionalRRs = buffer.getShort();
            
            if (questions <= 0 || questions > 10) {
                return null;
            }
            
            // Parse question section
            String domain = parseDomainName(buffer, requestData);
            if (domain == null || domain.isEmpty() || domain.length() > 253) {
                return null;
            }
            
            short qType = buffer.getShort();
            short qClass = buffer.getShort();
            
            System.out.println("Query for: " + domain + " (Type: " + qType + ")");
            
            // Create response
            return createDNSResponse(transactionId, domain, qType, qClass, requestData);
            
        } catch (Exception e) {
            System.err.println("Error parsing DNS request: " + e.getMessage());
            return null;
        }
    }
    
    private static String parseDomainName(ByteBuffer buffer, byte[] data) {
        StringBuilder domain = new StringBuilder();
        int position = buffer.position();
        int maxJumps = 10;
        int jumps = 0;
        
        try {
            while (true) {
                if (position >= data.length) break;
                
                int length = data[position] & 0xFF;
                
                if (length == 0) {
                    buffer.position(position + 1);
                    break;
                }
                
                if ((length & 0xC0) == 0xC0) {
                    if (position + 1 >= data.length) break;
                    int pointer = ((length & 0x3F) << 8) | (data[position + 1] & 0xFF);
                    if (pointer >= data.length || jumps++ > maxJumps) break;
                    position = pointer;
                    buffer.position(position + 2);
                    continue;
                }
                
                if (length > 63 || position + length + 1 > data.length) break;
                
                if (domain.length() > 0) {
                    domain.append('.');
                }
                
                for (int i = 0; i < length; i++) {
                    char c = (char) data[position + 1 + i];
                    if (Character.isLetterOrDigit(c) || c == '-' || c == '.') {
                        domain.append(c);
                    }
                }
                
                position += length + 1;
                if (domain.length() > 253) break;
            }
        } catch (Exception e) {
            return null;
        }
        
        return domain.toString();
    }
    
    private static byte[] createDNSResponse(short transactionId, String domain, 
                                           short qType, short qClass, byte[] originalQuery) {
        try {
            ByteBuffer response = ByteBuffer.allocate(MAX_BUFFER_SIZE);
            
            // DNS Header
            response.putShort(transactionId);
            response.putShort((short) 0x8180); // Response, recursion available
            response.putShort((short) 1); // Questions
            
            String ipAddress = dnsRecords.get(domain.toLowerCase());
            response.putShort((short) (ipAddress != null ? 1 : 0)); // Answer RRs
            response.putShort((short) 0); // Authority RRs
            response.putShort((short) 0); // Additional RRs
            
            // Question section (copy from original)
            int questionStart = DNS_HEADER_SIZE;
            int questionEnd = questionStart;
            while (questionEnd < originalQuery.length && originalQuery[questionEnd] != 0) {
                questionEnd++;
            }
            questionEnd += 5; // Include null terminator, type and class
            
            if (questionEnd <= originalQuery.length) {
                response.put(originalQuery, questionStart, questionEnd - questionStart);
            }
            
            // Answer section
            if (ipAddress != null) {
                response.putShort((short) 0xC00C); // Pointer to domain name
                response.putShort(qType); // Type
                response.putShort(qClass); // Class
                response.putInt(300); // TTL
                response.putShort((short) 4); // Data length
                
                String[] parts = ipAddress.split("\\\\.");
                if (parts.length == 4) {
                    for (String part : parts) {
                        int value = Integer.parseInt(part);
                        if (value >= 0 && value <= 255) {
                            response.put((byte) value);
                        }
                    }
                }
            }
            
            byte[] result = new byte[response.position()];
            response.flip();
            response.get(result);
            return result;
            
        } catch (Exception e) {
            System.err.println("Error creating DNS response: " + e.getMessage());
            return null;
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== DNS Server Test Cases ===\\n");
        
        // Test case setup
        Thread serverThread = new Thread(() -> {
            startDNSServer("127.0.0.1", 5353);
        });
        serverThread.start();
        
        try {
            Thread.sleep(1000); // Wait for server to start
            
            // Test cases
            String[] testDomains = {"example.com", "test.com", "localhost", "google.com", "github.com"};
            
            for (int i = 0; i < testDomains.length; i++) {
                System.out.println("\\nTest Case " + (i + 1) + ": Querying " + testDomains[i]);
                sendDNSQuery("127.0.0.1", 5353, testDomains[i]);
                Thread.sleep(500);
            }
            
            Thread.sleep(2000);
            
        } catch (Exception e) {
            System.err.println("Test error: " + e.getMessage());
        }
    }
    
    private static void sendDNSQuery(String serverIp, int serverPort, String domain) {
        try (DatagramSocket socket = new DatagramSocket()) {
            socket.setSoTimeout(5000);
            
            ByteBuffer query = ByteBuffer.allocate(MAX_BUFFER_SIZE);
            query.putShort((short) 0x1234); // Transaction ID
            query.putShort((short) 0x0100); // Standard query
            query.putShort((short) 1); // Questions
            query.putShort((short) 0); // Answer RRs
            query.putShort((short) 0); // Authority RRs
            query.putShort((short) 0); // Additional RRs
            
            String[] labels = domain.split("\\\\.");
            for (String label : labels) {
                query.put((byte) label.length());
                query.put(label.getBytes());
            }
            query.put((byte) 0);
            query.putShort((short) 1); // Type A
            query.putShort((short) 1); // Class IN
            
            byte[] queryData = new byte[query.position()];
            query.flip();
            query.get(queryData);
            
            InetAddress address = InetAddress.getByName(serverIp);
            DatagramPacket packet = new DatagramPacket(queryData, queryData.length, address, serverPort);
            socket.send(packet);
            
            byte[] buffer = new byte[MAX_BUFFER_SIZE];
            DatagramPacket response = new DatagramPacket(buffer, buffer.length);
            socket.receive(response);
            
            System.out.println("Received response for " + domain + " (" + response.getLength() + " bytes)");
            
        } catch (Exception e) {
            System.err.println("Query error: " + e.getMessage());
        }
    }
}
