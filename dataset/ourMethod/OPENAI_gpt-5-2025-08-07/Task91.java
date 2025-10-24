import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.net.*;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

public class Task91 {

    // DNS constants
    private static final int DNS_HEADER_LEN = 12;
    private static final int MAX_DNS_UDP_SIZE = 512;
    private static final int TYPE_A = 1;
    private static final int CLASS_IN = 1;

    // Security: validate hostname
    private static boolean isValidHostname(final String name) {
        if (name == null) return false;
        String n = name.trim();
        if (n.isEmpty()) return false;
        if (n.length() > 253) return false; // total length limit
        String[] labels = n.split("\\.");
        for (String label : labels) {
            if (label.isEmpty()) return false;
            if (label.length() > 63) return false;
            for (int i = 0; i < label.length(); i++) {
                char c = label.charAt(i);
                boolean ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
                        || (c >= '0' && c <= '9') || (c == '-') || (c == '_');
                if (!ok) return false;
            }
        }
        return true;
    }

    // Encode domain name into DNS label format
    private static byte[] encodeQName(final String hostname) throws IOException {
        if (!isValidHostname(hostname)) throw new IOException("Invalid hostname");
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        String[] labels = hostname.split("\\.");
        for (String label : labels) {
            byte[] lb = label.getBytes(StandardCharsets.US_ASCII);
            if (lb.length == 0 || lb.length > 63) throw new IOException("Invalid label length");
            out.write(lb.length);
            out.write(lb);
        }
        out.write(0); // end
        byte[] qname = out.toByteArray();
        if (qname.length > 255) throw new IOException("QName too long");
        return qname;
    }

    // Decode domain name starting at offset, only plain labels, no compression in question for safety
    private static class NameDecodingResult {
        final String name;
        final int nextOffset;
        NameDecodingResult(String name, int nextOffset) {
            this.name = name;
            this.nextOffset = nextOffset;
        }
    }

    private static NameDecodingResult decodeQName(final byte[] data, final int offset) throws IOException {
        int idx = offset;
        if (idx >= data.length) throw new IOException("Out of bounds");
        StringBuilder sb = new StringBuilder();
        int totalLen = 0;
        boolean first = true;
        while (true) {
            if (idx >= data.length) throw new IOException("Out of bounds");
            int len = data[idx] & 0xFF;
            idx += 1;
            if ((len & 0xC0) != 0) {
                throw new IOException("Compression not allowed in question");
            }
            if (len == 0) break;
            if (len > 63) throw new IOException("Label too long");
            if (idx + len > data.length) throw new IOException("Label exceeds packet");
            if (!first) sb.append('.');
            String label = new String(data, idx, len, StandardCharsets.US_ASCII);
            // Security: validate label
            if (label.isEmpty() || label.length() > 63) throw new IOException("Invalid label");
            for (int i = 0; i < label.length(); i++) {
                char c = label.charAt(i);
                boolean ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
                        || (c >= '0' && c <= '9') || (c == '-') || (c == '_');
                if (!ok) throw new IOException("Invalid char in label");
            }
            sb.append(label);
            idx += len;
            totalLen += len + 1;
            if (totalLen > 255) throw new IOException("QName too long");
            first = false;
        }
        if (sb.length() == 0) throw new IOException("Empty name");
        return new NameDecodingResult(sb.toString(), idx);
    }

    private static int readU16(final byte[] data, final int off) throws IOException {
        if (off + 2 > data.length) throw new IOException("Out of bounds");
        return ((data[off] & 0xFF) << 8) | (data[off + 1] & 0xFF);
    }

    private static void writeU16(ByteArrayOutputStream out, int v) {
        out.write((v >>> 8) & 0xFF);
        out.write(v & 0xFF);
    }

    private static void writeU32(ByteArrayOutputStream out, long v) {
        out.write((int) ((v >>> 24) & 0xFF));
        out.write((int) ((v >>> 16) & 0xFF));
        out.write((int) ((v >>> 8) & 0xFF));
        out.write((int) (v & 0xFF));
    }

    // Build DNS query bytes for tests
    private static byte[] buildDnsQuery(final String hostname, final int qtype, final int id, final boolean recursionDesired) throws IOException {
        if (!isValidHostname(hostname)) throw new IOException("Invalid hostname");
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        writeU16(out, id & 0xFFFF);
        int flags = 0;
        // QR=0 query, RD per flag
        if (recursionDesired) flags |= 0x0100;
        writeU16(out, flags);
        writeU16(out, 1); // QDCOUNT
        writeU16(out, 0); // ANCOUNT
        writeU16(out, 0); // NSCOUNT
        writeU16(out, 0); // ARCOUNT
        out.write(encodeQName(hostname));
        writeU16(out, qtype & 0xFFFF);
        writeU16(out, CLASS_IN);
        byte[] b = out.toByteArray();
        if (b.length > MAX_DNS_UDP_SIZE) throw new IOException("Query too large");
        return b;
    }

    // Parse minimal response for A record and rcode
    private static class ParsedResponse {
        final int id;
        final int rcode;
        final int ancount;
        final String answerIPv4; // nullable
        ParsedResponse(int id, int rcode, int ancount, String answerIPv4) {
            this.id = id;
            this.rcode = rcode;
            this.ancount = ancount;
            this.answerIPv4 = answerIPv4;
        }
    }

    private static ParsedResponse parseDnsResponse(final byte[] data) throws IOException {
        if (data == null || data.length < DNS_HEADER_LEN) throw new IOException("Short packet");
        int id = readU16(data, 0);
        int flags = readU16(data, 2);
        int rcode = flags & 0x000F;
        int qd = readU16(data, 4);
        int an = readU16(data, 6);
        int off = DNS_HEADER_LEN;
        // Skip questions
        for (int i = 0; i < qd; i++) {
            NameDecodingResult nd = decodeQName(data, off);
            off = nd.nextOffset;
            off += 4; // QTYPE + QCLASS
            if (off > data.length) throw new IOException("Truncated question");
        }
        String ipAnswer = null;
        // Parse first answer if present
        if (an > 0) {
            if (off + 10 > data.length) throw new IOException("Truncated answer");
            // name can be pointer; skip it
            int nameLenByte = data[off] & 0xFF;
            if ((nameLenByte & 0xC0) == 0xC0) {
                off += 2;
            } else {
                // Not expected here; skip labels safely
                while (true) {
                    if (off >= data.length) throw new IOException("Truncated name");
                    int len = data[off] & 0xFF;
                    off += 1;
                    if (len == 0) break;
                    if ((len & 0xC0) != 0) throw new IOException("Invalid compression in answer");
                    if (off + len > data.length) throw new IOException("Truncated label");
                    off += len;
                }
            }
            int type = readU16(data, off); off += 2;
            int clazz = readU16(data, off); off += 2;
            off += 4; // TTL
            int rdlen = readU16(data, off); off += 2;
            if (off + rdlen > data.length) throw new IOException("Truncated rdata");
            if (type == TYPE_A && clazz == CLASS_IN && rdlen == 4) {
                byte[] addr = Arrays.copyOfRange(data, off, off + 4);
                ipAnswer = (addr[0] & 0xFF) + "." + (addr[1] & 0xFF) + "." + (addr[2] & 0xFF) + "." + (addr[3] & 0xFF);
            }
        }
        return new ParsedResponse(id, rcode, an, ipAnswer);
    }

    // DNS server
    public static final class DnsServer implements Runnable {
        private final InetAddress bindAddress;
        private final int requestedPort;
        private final Map<String, InetAddress> aRecords; // immutable
        private volatile boolean running = false;
        private DatagramSocket socket;
        private Thread thread;

        public DnsServer(final String ip, final int port, final Map<String, String> records) throws IOException {
            if (ip == null) throw new IllegalArgumentException("IP is null");
            if (port < 0 || port > 65535) throw new IllegalArgumentException("Port out of range");
            this.bindAddress = InetAddress.getByName(ip);
            this.requestedPort = port;
            Map<String, InetAddress> tmp = new HashMap<>();
            if (records != null) {
                for (Map.Entry<String, String> e : records.entrySet()) {
                    String host = e.getKey() == null ? "" : e.getKey().trim().toLowerCase();
                    String val = e.getValue() == null ? "" : e.getValue().trim();
                    if (!isValidHostname(host)) continue;
                    InetAddress addr = InetAddress.getByName(val);
                    // Only accept IPv4 for A
                    if (addr instanceof Inet4Address) {
                        tmp.put(host, addr);
                    }
                }
            }
            this.aRecords = Collections.unmodifiableMap(tmp);
        }

        public synchronized void start() throws SocketException {
            if (running) return;
            socket = new DatagramSocket(null);
            SocketAddress sa = new InetSocketAddress(bindAddress, requestedPort);
            socket.bind(sa);
            socket.setSoTimeout(500);
            running = true;
            thread = new Thread(this, "DnsServerThread");
            thread.setDaemon(true);
            thread.start();
        }

        public synchronized void stop() {
            running = false;
            if (socket != null && !socket.isClosed()) {
                socket.close();
            }
        }

        public int getPort() {
            if (socket == null) return -1;
            return socket.getLocalPort();
        }

        @Override
        public void run() {
            byte[] buf = new byte[MAX_DNS_UDP_SIZE];
            while (running) {
                try {
                    DatagramPacket req = new DatagramPacket(buf, buf.length);
                    socket.receive(req);
                    byte[] data = Arrays.copyOfRange(req.getData(), req.getOffset(), req.getOffset() + req.getLength());
                    byte[] resp = handleRequest(data);
                    if (resp != null) {
                        DatagramPacket out = new DatagramPacket(resp, resp.length, req.getAddress(), req.getPort());
                        socket.send(out);
                    }
                } catch (SocketTimeoutException ste) {
                    // check running flag
                } catch (SocketException se) {
                    // socket closed or error
                    if (running) {
                        // best-effort continue
                    }
                } catch (Exception ex) {
                    // swallow to keep server alive, but do not expose details
                }
            }
        }

        private byte[] handleRequest(final byte[] data) {
            try {
                if (data == null || data.length < DNS_HEADER_LEN) return null;
                int id = readU16(data, 0);
                int flags = readU16(data, 2);
                int qr = (flags >>> 15) & 0x1;
                int opcode = (flags >>> 11) & 0xF;
                int rd = (flags >>> 8) & 0x1;
                int qd = readU16(data, 4);
                if (qr != 0 || opcode != 0 || qd < 1) {
                    return buildErrorResponse(id, rd, 1, data); // FORMERR
                }
                NameDecodingResult nd = decodeQName(data, DNS_HEADER_LEN);
                int off = nd.nextOffset;
                if (off + 4 > data.length) return buildErrorResponse(id, rd, 1, data);
                int qtype = readU16(data, off); off += 2;
                int qclass = readU16(data, off); off += 2;
                if (qclass != CLASS_IN) {
                    return buildErrorResponse(id, rd, 4, data); // Not Implemented
                }
                // Lookup
                InetAddress addr = aRecords.get(nd.name.toLowerCase());
                if (qtype != TYPE_A) {
                    return buildErrorResponse(id, rd, 4, data); // Not Implemented
                }
                if (addr == null) {
                    return buildErrorResponse(id, rd, 3, data); // NXDOMAIN
                }
                if (!(addr instanceof Inet4Address)) {
                    return buildErrorResponse(id, rd, 3, data); // Not found for A
                }
                // Build success response with one A answer
                return buildAResponse(id, rd, data, nd, (Inet4Address) addr);
            } catch (IOException ioe) {
                // Malformed -> FORMERR
                try {
                    int id = 0;
                    int rd = 0;
                    if (data != null && data.length >= 4) {
                        id = readU16(data, 0);
                        int flags = readU16(data, 2);
                        rd = (flags >>> 8) & 0x1;
                    }
                    return buildErrorResponse(id, rd, 1, data);
                } catch (Exception ignored) {
                    return null;
                }
            }
        }

        private byte[] buildErrorResponse(int id, int rd, int rcode, byte[] request) {
            try {
                ByteArrayOutputStream out = new ByteArrayOutputStream();
                writeU16(out, id & 0xFFFF);
                int flags = 0x8000; // QR=1
                flags |= 0x0400; // AA=1
                if (rd != 0) flags |= 0x0100; // RD copied
                flags |= (rcode & 0xF);
                writeU16(out, flags);
                // If request carries 1 question, copy it back; else zero
                int qd = 0;
                if (request != null && request.length >= DNS_HEADER_LEN) {
                    int rq = ((request[4] & 0xFF) << 8) | (request[5] & 0xFF);
                    if (rq >= 1) {
                        qd = 1;
                    }
                }
                writeU16(out, qd);
                writeU16(out, 0); // ANCOUNT
                writeU16(out, 0); // NSCOUNT
                writeU16(out, 0); // ARCOUNT
                if (qd == 1) {
                    // Copy the first question safely
                    try {
                        NameDecodingResult nd = decodeQName(request, DNS_HEADER_LEN);
                        byte[] qname = Arrays.copyOfRange(request, DNS_HEADER_LEN, nd.nextOffset);
                        out.write(qname);
                        int qtype = readU16(request, nd.nextOffset);
                        int qclass = readU16(request, nd.nextOffset + 2);
                        writeU16(out, qtype);
                        writeU16(out, qclass);
                    } catch (Exception ex) {
                        // ignore question if malformed
                    }
                }
                return out.toByteArray();
            } catch (Exception e) {
                return null;
            }
        }

        private byte[] buildAResponse(int id, int rd, byte[] request, NameDecodingResult nd, Inet4Address addr) throws IOException {
            ByteArrayOutputStream out = new ByteArrayOutputStream();
            writeU16(out, id & 0xFFFF);
            int flags = 0x8000; // QR=1
            flags |= 0x0400;    // AA=1
            if (rd != 0) flags |= 0x0100; // RD copy
            // RCODE=0
            writeU16(out, flags);
            writeU16(out, 1); // QDCOUNT
            writeU16(out, 1); // ANCOUNT
            writeU16(out, 0); // NSCOUNT
            writeU16(out, 0); // ARCOUNT

            // Question: copy from request
            byte[] qnameBytes = Arrays.copyOfRange(request, DNS_HEADER_LEN, nd.nextOffset);
            out.write(qnameBytes);
            int qtype = readU16(request, nd.nextOffset);
            int qclass = readU16(request, nd.nextOffset + 2);
            writeU16(out, qtype);
            writeU16(out, qclass);

            // Answer: use pointer to 0x0C
            out.write(0xC0);
            out.write(0x0C);
            writeU16(out, TYPE_A);
            writeU16(out, CLASS_IN);
            writeU32(out, 60); // TTL 60 seconds
            writeU16(out, 4);  // RDLENGTH
            byte[] ip = addr.getAddress(); // 4 bytes
            out.write(ip);
            return out.toByteArray();
        }
    }

    // Simple UDP client helper for tests
    private static ParsedResponse sendDnsQueryAndReceive(final String serverIp, final int serverPort, final String name, final int qtype, final int id) throws IOException {
        if (!isValidHostname(name)) throw new IOException("Invalid name");
        InetAddress addr = InetAddress.getByName(serverIp);
        if (!(addr instanceof Inet4Address) && !(addr instanceof Inet6Address)) throw new IOException("Invalid server IP");
        if (serverPort <= 0 || serverPort > 65535) throw new IOException("Invalid port");
        byte[] query = buildDnsQuery(name, qtype, id, true);
        try (DatagramSocket sock = new DatagramSocket()) {
            sock.setSoTimeout(1000);
            DatagramPacket p = new DatagramPacket(query, query.length, new InetSocketAddress(addr, serverPort));
            sock.send(p);
            byte[] buf = new byte[MAX_DNS_UDP_SIZE];
            DatagramPacket resp = new DatagramPacket(buf, buf.length);
            sock.receive(resp);
            byte[] data = Arrays.copyOfRange(resp.getData(), resp.getOffset(), resp.getOffset() + resp.getLength());
            return parseDnsResponse(data);
        } catch (SocketTimeoutException ste) {
            throw new IOException("Timeout waiting for response");
        }
    }

    // Test runner
    public static void main(String[] args) throws Exception {
        // Records to serve
        Map<String, String> records = new HashMap<>();
        records.put("example.com", "93.184.216.34");
        records.put("localhost", "127.0.0.1");
        records.put("test.local", "10.0.0.123");
        records.put("a.b.c", "192.0.2.1");

        DnsServer server = new DnsServer("127.0.0.1", 0, records);
        server.start();
        int port = server.getPort();
        System.out.println("DNS server listening on 127.0.0.1:" + port);

        try {
            // Test 1: A example.com
            ParsedResponse r1 = sendDnsQueryAndReceive("127.0.0.1", port, "example.com", TYPE_A, 0x1111);
            System.out.println("Test1: id=" + r1.id + " rcode=" + r1.rcode + " ancount=" + r1.ancount + " A=" + r1.answerIPv4);

            // Test 2: A localhost
            ParsedResponse r2 = sendDnsQueryAndReceive("127.0.0.1", port, "localhost", TYPE_A, 0x2222);
            System.out.println("Test2: id=" + r2.id + " rcode=" + r2.rcode + " ancount=" + r2.ancount + " A=" + r2.answerIPv4);

            // Test 3: A unknown -> NXDOMAIN
            try {
                ParsedResponse r3 = sendDnsQueryAndReceive("127.0.0.1", port, "unknown.example", TYPE_A, 0x3333);
                System.out.println("Test3: id=" + r3.id + " rcode=" + r3.rcode + " ancount=" + r3.ancount + " A=" + r3.answerIPv4);
            } catch (IOException ex) {
                System.out.println("Test3: exception " + ex.getMessage());
            }

            // Test 4: AAAA example.com -> Not Implemented
            final int TYPE_AAAA = 28;
            ParsedResponse r4 = sendDnsQueryAndReceive("127.0.0.1", port, "example.com", TYPE_AAAA, 0x4444);
            System.out.println("Test4: id=" + r4.id + " rcode=" + r4.rcode + " ancount=" + r4.ancount + " A=" + r4.answerIPv4);

            // Test 5: Malformed query (too short) -> expect no response or FORMERR
            try (DatagramSocket sock = new DatagramSocket()) {
                sock.setSoTimeout(750);
                byte[] malformed = new byte[] {0x00, 0x01, 0x02}; // too short
                DatagramPacket p = new DatagramPacket(malformed, malformed.length,
                        new InetSocketAddress(InetAddress.getByName("127.0.0.1"), port));
                sock.send(p);
                byte[] buf = new byte[MAX_DNS_UDP_SIZE];
                DatagramPacket resp = new DatagramPacket(buf, buf.length);
                sock.receive(resp);
                byte[] data = Arrays.copyOfRange(resp.getData(), resp.getOffset(), resp.getOffset() + resp.getLength());
                ParsedResponse r5 = parseDnsResponse(data);
                System.out.println("Test5: id=" + r5.id + " rcode=" + r5.rcode + " ancount=" + r5.ancount + " A=" + r5.answerIPv4);
            } catch (SocketTimeoutException te) {
                System.out.println("Test5: no response (expected for malformed)");
            }
        } finally {
            server.stop();
        }
    }
}