import java.io.*;
import java.net.*;
import java.util.*;

public class Task91 {

    static class ParsedQuery {
        int id;
        int flags;
        String qname;
        int qtype;
        int qclass;
        int questionEndOffset;
    }

    public static ParsedQuery parseDnsQuery(byte[] data) throws IOException {
        if (data.length < 12) throw new IOException("Invalid DNS packet");
        ParsedQuery pq = new ParsedQuery();
        pq.id = ((data[0] & 0xFF) << 8) | (data[1] & 0xFF);
        pq.flags = ((data[2] & 0xFF) << 8) | (data[3] & 0xFF);
        int qdcount = ((data[4] & 0xFF) << 8) | (data[5] & 0xFF);
        if (qdcount < 1) throw new IOException("No questions");
        int idx = 12;
        StringBuilder sb = new StringBuilder();
        while (idx < data.length) {
            int len = data[idx] & 0xFF;
            idx++;
            if (len == 0) break;
            if (idx + len > data.length) throw new IOException("Bad QNAME");
            if (sb.length() > 0) sb.append('.');
            sb.append(new String(data, idx, len, "UTF-8"));
            idx += len;
        }
        if (idx + 4 > data.length) throw new IOException("Truncated question");
        pq.qname = sb.toString();
        pq.qtype = ((data[idx] & 0xFF) << 8) | (data[idx + 1] & 0xFF);
        pq.qclass = ((data[idx + 2] & 0xFF) << 8) | (data[idx + 3] & 0xFF);
        pq.questionEndOffset = idx + 4;
        return pq;
    }

    public static byte[] buildDnsResponse(byte[] request, Map<String, String> hosts) throws IOException {
        ParsedQuery pq = parseDnsQuery(request);
        boolean rd = (pq.flags & 0x0100) != 0;
        int opcode = pq.flags & 0x7800;
        String qn = pq.qname.toLowerCase(Locale.ROOT);
        String ip = null;
        if (pq.qclass == 1 && pq.qtype == 1) { // IN A
            ip = hosts.get(qn);
        }
        int rcode = (ip == null) ? 3 : 0; // NXDOMAIN if not found

        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        DataOutputStream dos = new DataOutputStream(baos);

        // Header
        dos.writeShort(pq.id & 0xFFFF);
        int flags = 0x8000 | opcode | 0x0400 | (rd ? 0x0100 : 0x0000) | (rcode & 0xF);
        dos.writeShort(flags);
        dos.writeShort(1); // QDCOUNT
        dos.writeShort(ip != null ? 1 : 0); // ANCOUNT
        dos.writeShort(0); // NSCOUNT
        dos.writeShort(0); // ARCOUNT

        // Question: copy from request (from offset 12 to questionEndOffset)
        dos.write(request, 12, pq.questionEndOffset - 12);

        if (ip != null) {
            // Answer
            dos.writeShort(0xC00C); // pointer to QNAME at 12
            dos.writeShort(1); // TYPE A
            dos.writeShort(1); // CLASS IN
            dos.writeInt(60); // TTL
            dos.writeShort(4); // RDLENGTH
            String[] parts = ip.split("\\.");
            if (parts.length != 4) throw new IOException("Bad IP mapping");
            for (String p : parts) dos.writeByte(Integer.parseInt(p) & 0xFF);
        }
        dos.flush();
        return baos.toByteArray();
    }

    public static byte[] buildDnsQuery(String name, int qtype, boolean rd) throws IOException {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        DataOutputStream dos = new DataOutputStream(baos);
        int id = (int) (System.nanoTime() & 0xFFFF);
        dos.writeShort(id);
        int flags = (rd ? 0x0100 : 0);
        dos.writeShort(flags);
        dos.writeShort(1); // QDCOUNT
        dos.writeShort(0); // ANCOUNT
        dos.writeShort(0); // NSCOUNT
        dos.writeShort(0); // ARCOUNT
        for (String label : name.split("\\.")) {
            byte[] lb = label.getBytes("UTF-8");
            dos.writeByte(lb.length);
            dos.write(lb);
        }
        dos.writeByte(0); // end of QNAME
        dos.writeShort(qtype & 0xFFFF);
        dos.writeShort(1); // IN
        dos.flush();
        return baos.toByteArray();
    }

    public static void runUdpDnsServer(String ip, int port, Map<String, String> hosts, int maxRequests) throws IOException {
        DatagramSocket socket = new DatagramSocket(new InetSocketAddress(ip, port));
        byte[] buf = new byte[1500];
        int handled = 0;
        try {
            while (maxRequests <= 0 || handled < maxRequests) {
                DatagramPacket pkt = new DatagramPacket(buf, buf.length);
                socket.receive(pkt);
                byte[] req = Arrays.copyOfRange(pkt.getData(), 0, pkt.getLength());
                byte[] resp;
                try {
                    resp = buildDnsResponse(req, hosts);
                } catch (Exception e) {
                    // Build a SERVFAIL
                    resp = buildServfail(req);
                }
                DatagramPacket out = new DatagramPacket(resp, resp.length, pkt.getSocketAddress());
                socket.send(out);
                handled++;
            }
        } finally {
            socket.close();
        }
    }

    private static byte[] buildServfail(byte[] request) {
        try {
            int id = ((request[0] & 0xFF) << 8) | (request[1] & 0xFF);
            int flags = ((request[2] & 0xFF) << 8) | (request[3] & 0xFF);
            boolean rd = (flags & 0x0100) != 0;
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            DataOutputStream dos = new DataOutputStream(baos);
            dos.writeShort(id);
            int rflags = 0x8000 | (flags & 0x7800) | (rd ? 0x0100 : 0) | 2; // SERVFAIL
            dos.writeShort(rflags);
            dos.writeShort(1);
            dos.writeShort(0);
            dos.writeShort(0);
            dos.writeShort(0);
            // copy question if available
            try {
                ParsedQuery pq = parseDnsQuery(request);
                dos.write(request, 12, pq.questionEndOffset - 12);
            } catch (Exception ignored) {}
            dos.flush();
            return baos.toByteArray();
        } catch (IOException e) {
            return new byte[] {0,0, (byte)0x81, (byte)0x82, 0,0,0,0,0,0};
        }
    }

    public static void main(String[] args) throws Exception {
        Map<String, String> hosts = new HashMap<>();
        hosts.put("example.com", "93.184.216.34");
        hosts.put("localhost", "127.0.0.1");
        hosts.put("github.com", "140.82.113.3");
        hosts.put("test.local", "10.0.0.1");
        hosts.put("service.internal", "192.168.1.100");

        // 5 test cases
        String[] tests = new String[] {
            "example.com",
            "unknown.example",
            "github.com",
            "localhost",
            "test.local"
        };
        for (String t : tests) {
            byte[] q = buildDnsQuery(t, 1, true);
            byte[] r = buildDnsResponse(q, hosts);
            ParsedQuery pq = parseDnsQuery(q);
            System.out.println("Q: " + pq.qname + " type " + pq.qtype + " -> resp bytes " + r.length);
        }

        // Example: start UDP server that handles 5 requests on 127.0.0.1:5353
        // runUdpDnsServer("127.0.0.1", 5353, hosts, 5);
    }
}