import java.io.ByteArrayOutputStream;
import java.net.*;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Task91 {

    // Step 1: Problem understanding, Step 2-5: Security-focused implementation with review in comments.

    // Build a standard DNS query for given name and type
    public static byte[] buildDnsQuery(String name, int qtype, int id) {
        if (name == null) throw new IllegalArgumentException("name");
        if (qtype <= 0 || qtype > 65535) throw new IllegalArgumentException("qtype");
        ByteArrayOutputStream out = new ByteArrayOutputStream(512);
        // Header
        out.write((id >> 8) & 0xFF);
        out.write(id & 0xFF);
        // Flags: RD set
        out.write(0x01);
        out.write(0x00);
        // QDCOUNT=1
        out.write(0x00); out.write(0x01);
        // ANCOUNT=0
        out.write(0x00); out.write(0x00);
        // NSCOUNT=0
        out.write(0x00); out.write(0x00);
        // ARCOUNT=0
        out.write(0x00); out.write(0x00);
        // QNAME
        for (String label : name.split("\\.")) {
            byte[] lb = label.getBytes(StandardCharsets.US_ASCII);
            if (lb.length == 0 || lb.length > 63) throw new IllegalArgumentException("Invalid label length");
            out.write(lb.length);
            out.write(lb, 0, lb.length);
        }
        out.write(0x00);
        // QTYPE
        out.write((qtype >> 8) & 0xFF);
        out.write(qtype & 0xFF);
        // QCLASS IN
        out.write(0x00); out.write(0x01);
        return out.toByteArray();
    }

    // Decode QNAME without pointer support for simplicity (test queries we send are uncompressed)
    private static class NameParse {
        String name;
        int nextIndex;
    }

    private static NameParse parseQName(byte[] data, int offset) throws Exception {
        int i = offset;
        StringBuilder sb = new StringBuilder();
        boolean first = true;
        while (true) {
            if (i >= data.length) throw new Exception("Truncated QNAME");
            int len = data[i] & 0xFF;
            if ((len & 0xC0) == 0xC0) { // pointer not supported here
                throw new Exception("Compression in question not supported by parser in this demo");
            }
            i++;
            if (len == 0) break;
            if (len > 63) throw new Exception("Invalid label len");
            if (i + len > data.length) throw new Exception("Truncated label");
            String label = new String(data, i, len, StandardCharsets.US_ASCII);
            i += len;
            if (!first) sb.append('.');
            sb.append(label);
            first = false;
        }
        NameParse np = new NameParse();
        np.name = sb.toString();
        np.nextIndex = i;
        return np;
    }

    // Build a DNS response for a limited set of records: A for example.com, localhost, test.local
    public static byte[] buildDnsResponse(byte[] request) {
        try {
            if (request == null || request.length < 12 || request.length > 512) return null;
            ByteBuffer hdr = ByteBuffer.wrap(request);
            int id = ((hdr.get(0) & 0xFF) << 8) | (hdr.get(1) & 0xFF);
            int flags = ((hdr.get(2) & 0xFF) << 8) | (hdr.get(3) & 0xFF);
            int qd = ((hdr.get(4) & 0xFF) << 8) | (hdr.get(5) & 0xFF);
            if (qd != 1) {
                // Form error
                return buildErrorResponse(id, flags, request, 1);
            }
            NameParse np = parseQName(request, 12);
            if (np.nextIndex + 4 > request.length) return buildErrorResponse(id, flags, request, 1);
            int qtype = ((request[np.nextIndex + 0] & 0xFF) << 8) | (request[np.nextIndex + 1] & 0xFF);
            int qclass = ((request[np.nextIndex + 2] & 0xFF) << 8) | (request[np.nextIndex + 3] & 0xFF);
            if (qclass != 1) {
                return buildErrorResponse(id, flags, request, 4); // Not Implemented for non-IN
            }
            String qname = np.name.toLowerCase();
            ByteArrayOutputStream out = new ByteArrayOutputStream(512);
            // Header
            out.write((id >> 8) & 0xFF); out.write(id & 0xFF);
            int rd = (flags >> 8) & 0x01;
            int rflags = 0x8000 | (rd << 8) | 0x0400; // QR=1, AA=1, RD copied, RA=0
            int rcode = 0;
            byte[] answerRdata = null;
            if (qtype == 1) { // A
                if (qname.equals("example.com")) {
                    answerRdata = new byte[]{(byte)93,(byte)184,(byte)216,(byte)34};
                } else if (qname.equals("localhost")) {
                    answerRdata = new byte[]{127,0,0,1};
                } else if (qname.equals("test.local")) {
                    answerRdata = new byte[]{(byte)192,(byte)168,1,2};
                } else {
                    // NXDOMAIN
                    rcode = 3;
                }
            } else {
                rcode = 4; // Not Implemented
            }
            rflags |= (rcode & 0xF);
            out.write((rflags >> 8) & 0xFF); out.write(rflags & 0xFF);
            // QDCOUNT=1
            out.write(0x00); out.write(0x01);
            // ANCOUNT
            if (answerRdata != null && rcode == 0) { out.write(0x00); out.write(0x01); }
            else { out.write(0x00); out.write(0x00); }
            // NSCOUNT, ARCOUNT
            out.write(0x00); out.write(0x00);
            out.write(0x00); out.write(0x00);
            // Question: copy original
            out.write(request, 12, (np.nextIndex + 4) - 12);
            if (answerRdata != null && rcode == 0) {
                // Answer
                // NAME: pointer to QNAME at 12
                out.write(0xC0); out.write(0x0C);
                // TYPE
                out.write(0x00); out.write(0x01);
                // CLASS IN
                out.write(0x00); out.write(0x01);
                // TTL 60
                out.write(0x00); out.write(0x00); out.write(0x00); out.write(0x3C);
                // RDLENGTH
                out.write(0x00); out.write(0x04);
                // RDATA
                out.write(answerRdata, 0, 4);
            }
            byte[] resp = out.toByteArray();
            if (resp.length > 512) return null;
            return resp;
        } catch (Exception e) {
            return null;
        }
    }

    private static byte[] buildErrorResponse(int id, int flags, byte[] request, int rcode) {
        try {
            ByteArrayOutputStream out = new ByteArrayOutputStream(512);
            out.write((id >> 8) & 0xFF); out.write(id & 0xFF);
            int rd = (flags >> 8) & 0x01;
            int rflags = 0x8000 | (rd << 8) | 0x0400 | (rcode & 0xF);
            out.write((rflags >> 8) & 0xFF); out.write(rflags & 0xFF);
            // QDCOUNT=0
            out.write(0x00); out.write(0x00);
            // ANCOUNT=0, NSCOUNT=0, ARCOUNT=0
            out.write(0x00); out.write(0x00);
            out.write(0x00); out.write(0x00);
            out.write(0x00); out.write(0x00);
            return out.toByteArray();
        } catch (Exception ex) {
            return null;
        }
    }

    public static void runServer(String bindIP, int port, int maxRequests) throws Exception {
        if (port <= 0 || port > 65535) throw new IllegalArgumentException("port");
        InetAddress bindAddr = InetAddress.getByName(bindIP);
        DatagramSocket sock = new DatagramSocket(new InetSocketAddress(bindAddr, port));
        sock.setSoTimeout(0);
        byte[] buf = new byte[512];
        int handled = 0;
        try {
            while (handled < maxRequests) {
                DatagramPacket pkt = new DatagramPacket(buf, buf.length);
                sock.receive(pkt);
                byte[] req = Arrays.copyOfRange(pkt.getData(), pkt.getOffset(), pkt.getOffset() + pkt.getLength());
                byte[] resp = buildDnsResponse(req);
                if (resp != null) {
                    DatagramPacket out = new DatagramPacket(resp, resp.length, pkt.getAddress(), pkt.getPort());
                    sock.send(out);
                }
                handled++;
            }
        } finally {
            sock.close();
        }
    }

    public static String parseAFromResponse(byte[] resp) {
        if (resp == null || resp.length < 12) return null;
        int ancount = ((resp[6] & 0xFF) << 8) | (resp[7] & 0xFF);
        if (ancount < 1) return null;
        // Skip question
        int i = 12;
        while (i < resp.length && resp[i] != 0) {
            int len = resp[i] & 0xFF;
            if ((len & 0xC0) == 0xC0) { if (i + 1 >= resp.length) return null; i += 2; break; }
            i += 1 + len;
        }
        if (i >= resp.length) return null;
        i++; // zero
        i += 4; // qtype+qclass
        // Answer
        if (i + 12 > resp.length) return null;
        // skip NAME (2 if pointer or labels). We expect pointer 2 bytes
        if ((resp[i] & 0xC0) == 0xC0) i += 2;
        else {
            while (i < resp.length && resp[i] != 0) {
                int len = resp[i] & 0xFF; i += 1 + len;
            }
            i++;
        }
        if (i + 10 > resp.length) return null;
        int type = ((resp[i] & 0xFF) << 8) | (resp[i+1] & 0xFF);
        i += 2; // type
        i += 2; // class
        i += 4; // ttl
        int rdlen = ((resp[i] & 0xFF) << 8) | (resp[i+1] & 0xFF); i+=2;
        if (type != 1 || rdlen != 4 || i + 4 > resp.length) return null;
        return (resp[i] & 0xFF) + "." + (resp[i+1] & 0xFF) + "." + (resp[i+2] & 0xFF) + "." + (resp[i+3] & 0xFF);
    }

    public static void main(String[] args) throws Exception {
        final String bindIP = "127.0.0.1";
        final int port = 53535;
        Thread serverThread = new Thread(() -> {
            try { runServer(bindIP, port, 5); } catch (Exception e) { /* swallow in demo */ }
        });
        serverThread.setDaemon(true);
        serverThread.start();
        Thread.sleep(200);

        String[] names = new String[] {
                "example.com",
                "localhost",
                "test.local",
                "unknown.example",
                "example.com"
        };
        int[] types = new int[] {1,1,1,1,28}; // last is AAAA to trigger Not Implemented
        for (int i=0;i<5;i++) {
            byte[] query = buildDnsQuery(names[i], types[i], 0x1234 + i);
            DatagramSocket client = new DatagramSocket();
            client.setSoTimeout(2000);
            DatagramPacket dp = new DatagramPacket(query, query.length, InetAddress.getByName(bindIP), port);
            client.send(dp);
            byte[] buf = new byte[512];
            DatagramPacket rp = new DatagramPacket(buf, buf.length);
            try {
                client.receive(rp);
                byte[] resp = Arrays.copyOfRange(rp.getData(), rp.getOffset(), rp.getOffset()+rp.getLength());
                String a = parseAFromResponse(resp);
                System.out.println("Test " + (i+1) + " for " + names[i] + " type " + types[i] + " -> " + (a==null?"NOANSWER":a));
            } catch (SocketTimeoutException ste) {
                System.out.println("Test " + (i+1) + " timeout");
            } finally {
                client.close();
            }
        }
        serverThread.join(500);
    }
}