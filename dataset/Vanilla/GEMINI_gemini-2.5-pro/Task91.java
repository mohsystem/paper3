import java.io.*;
import java.net.*;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.CountDownLatch;

public class Task91 {

    private static final int PORT = 5353;
    private static final String IP = "127.0.0.1";
    private static final Map<String, String> dnsRecords = new HashMap<>();

    static {
        dnsRecords.put("test1.dns.local.", "1.1.1.1");
        dnsRecords.put("test2.dns.local.", "2.2.2.2");
        dnsRecords.put("test3.dns.local.", "3.3.3.3");
    }

    private static String parseDomainName(ByteBuffer buffer) {
        StringBuilder domainName = new StringBuilder();
        int length = buffer.get() & 0xFF;
        while (length != 0) {
            if ((length & 0xC0) == 0xC0) { // Pointer
                // This simple parser doesn't support pointers, but a real one would.
                // For this example, we assume no pointers in the question.
                break;
            }
            byte[] label = new byte[length];
            buffer.get(label);
            domainName.append(new String(label, StandardCharsets.UTF_8));
            domainName.append(".");
            length = buffer.get() & 0xFF;
        }
        return domainName.toString();
    }

    private static void encodeDomainName(ByteBuffer buffer, String domain) {
        for (String label : domain.split("\\.")) {
            buffer.put((byte) label.length());
            buffer.put(label.getBytes(StandardCharsets.UTF_8));
        }
        buffer.put((byte) 0); // End of domain name
    }

    private static byte[] createDnsResponse(byte[] request) {
        ByteBuffer requestBuffer = ByteBuffer.wrap(request);
        ByteBuffer responseBuffer = ByteBuffer.allocate(512);

        // Copy Transaction ID
        short transactionID = requestBuffer.getShort();
        responseBuffer.putShort(transactionID);

        // Flags, QDCOUNT, ANCOUNT, etc.
        requestBuffer.position(2);
        short flags = requestBuffer.getShort();
        short qdcount = requestBuffer.getShort();
        short ancount = requestBuffer.getShort();
        short nscount = requestBuffer.getShort();
        short arcount = requestBuffer.getShort();

        // Find domain name from question section
        requestBuffer.position(12);
        String domainName = parseDomainName(requestBuffer);
        
        // Skip QTYPE and QCLASS
        short qtype = requestBuffer.getShort();
        short qclass = requestBuffer.getShort();

        String ipAddress = dnsRecords.get(domainName);

        if (ipAddress != null && qtype == 1 && qclass == 1) { // A record, IN class
            // Response Flags (QR=1, RCODE=0)
            responseBuffer.putShort((short) 0x8180);
            // Counts
            responseBuffer.putShort((short) 1); // QDCOUNT
            responseBuffer.putShort((short) 1); // ANCOUNT
            responseBuffer.putShort((short) 0); // NSCOUNT
            responseBuffer.putShort((short) 0); // ARCOUNT

            // Question Section
            encodeDomainName(responseBuffer, domainName);
            responseBuffer.putShort((short) 1); // QTYPE A
            responseBuffer.putShort((short) 1); // QCLASS IN

            // Answer Section
            responseBuffer.putShort((short) 0xC00C); // Pointer to name at offset 12
            responseBuffer.putShort((short) 1);      // TYPE A
            responseBuffer.putShort((short) 1);      // CLASS IN
            responseBuffer.putInt(60);               // TTL
            responseBuffer.putShort((short) 4);      // RDLENGTH
            try {
                responseBuffer.put(InetAddress.getByName(ipAddress).getAddress());
            } catch (UnknownHostException e) { /* should not happen */ }
        } else {
            // Not Found (NXDOMAIN)
            responseBuffer.putShort((short) 0x8183);
            responseBuffer.putShort(qdcount); // QDCOUNT
            responseBuffer.putShort((short) 0); // ANCOUNT
            responseBuffer.putShort((short) 0); // NSCOUNT
            responseBuffer.putShort((short) 0); // ARCOUNT
            
            encodeDomainName(responseBuffer, domainName);
            responseBuffer.putShort(qtype);
            responseBuffer.putShort(qclass);
        }

        byte[] response = new byte[responseBuffer.position()];
        responseBuffer.flip();
        responseBuffer.get(response);
        return response;
    }

    public static void runServer(CountDownLatch latch) {
        try (DatagramSocket socket = new DatagramSocket(PORT, InetAddress.getByName(IP))) {
            System.out.println("Java DNS Server listening on " + IP + ":" + PORT);
            latch.countDown(); // Signal that server is up

            byte[] buffer = new byte[512];
            while (!Thread.currentThread().isInterrupted()) {
                DatagramPacket requestPacket = new DatagramPacket(buffer, buffer.length);
                socket.setSoTimeout(1000); // Check for interrupt every second
                try {
                    socket.receive(requestPacket);
                    byte[] requestData = Arrays.copyOf(requestPacket.getData(), requestPacket.getLength());

                    byte[] responseData = createDnsResponse(requestData);

                    DatagramPacket responsePacket = new DatagramPacket(
                            responseData, responseData.length,
                            requestPacket.getAddress(), requestPacket.getPort());
                    socket.send(responsePacket);
                } catch (SocketTimeoutException e) {
                    // Timeout allows checking the interrupted flag
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            System.out.println("Java DNS Server stopped.");
        }
    }

    private static void runClientTest(String hostname) {
        System.out.println("\n--- Testing: " + hostname + " ---");
        try (DatagramSocket clientSocket = new DatagramSocket()) {
            clientSocket.setSoTimeout(2000);
            InetAddress address = InetAddress.getByName(IP);

            // Build DNS Query
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            DataOutputStream dos = new DataOutputStream(baos);
            dos.writeShort(0x1234); // Transaction ID
            dos.writeShort(0x0100); // Flags (Standard Query)
            dos.writeShort(1);      // Questions
            dos.writeShort(0);      // Answers
            dos.writeShort(0);      // Authority RRs
            dos.writeShort(0);      // Additional RRs

            String[] labels = hostname.split("\\.");
            for(String label : labels) {
                dos.writeByte(label.length());
                dos.writeBytes(label);
            }
            dos.writeByte(0); // End of name

            dos.writeShort(1); // Type: A
            dos.writeShort(1); // Class: IN
            dos.flush();

            byte[] query = baos.toByteArray();
            DatagramPacket sendPacket = new DatagramPacket(query, query.length, address, PORT);
            clientSocket.send(sendPacket);

            byte[] responseBuffer = new byte[512];
            DatagramPacket receivePacket = new DatagramPacket(responseBuffer, responseBuffer.length);
            clientSocket.receive(receivePacket);
            
            System.out.println("Query sent for: " + hostname);
            ByteBuffer resp = ByteBuffer.wrap(receivePacket.getData());
            System.out.println("Transaction ID: 0x" + Integer.toHexString(resp.getShort()));
            short flags = resp.getShort();
            System.out.println("Flags: 0x" + Integer.toHexString(flags & 0xFFFF));
            if ((flags & 0x000F) == 3) {
                 System.out.println("Response: NXDOMAIN (Not Found)");
            } else if ((flags & 0x8000) != 0) {
                 System.out.println("Response received (" + receivePacket.getLength() + " bytes).");
                 resp.position(receivePacket.getLength() - 4);
                 byte[] ip = new byte[4];
                 resp.get(ip);
                 System.out.println("Resolved IP: " + InetAddress.getByAddress(ip).getHostAddress());
            }

        } catch (IOException e) {
            System.out.println("Error testing " + hostname + ": " + e.getMessage());
        }
    }

    public static void main(String[] args) throws InterruptedException {
        CountDownLatch serverReadyLatch = new CountDownLatch(1);
        Thread serverThread = new Thread(() -> runServer(serverReadyLatch));
        serverThread.start();
        
        serverReadyLatch.await(); // Wait for server to be ready

        // 5 Test cases
        runClientTest("test1.dns.local");
        runClientTest("test2.dns.local");
        runClientTest("unknown.dns.local");
        runClientTest("test3.dns.local");
        runClientTest("another.unknown.com");
        
        serverThread.interrupt(); // Stop the server
        serverThread.join();
        System.out.println("\nAll tests completed.");
    }
}