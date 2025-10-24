import java.io.*;
import java.net.*;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.concurrent.*;

public class Task91 {
    // DNS Types
    private static final int TYPE_A = 1;
    private static final int TYPE_AAAA = 28;

    static class DnsQuery {
        int id;
        int flags;
        int qdCount;
        String qname;
        int qtype;
        int qclass;
        int questionEndOffset;
    }

    // Encode domain name into DNS wire format
    public static byte[] encodeName(String name) throws IOException {
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        if (name == null || name.isEmpty()) {
            out.write(0);
            return out.toByteArray();
        }
        String[] labels = name.split("\\.");
        for (String label : labels) {
            if (label.length() == 0 || label.length() > 63) throw new IOException("Invalid label length");
            byte[] lb = label.getBytes(StandardCharsets.US_ASCII);
            out.write(lb.length);
            out.write(lb);
        }
        out.write(0);
        return out.toByteArray();
    }

    // Decode domain name with compression support
    public static String decodeName(byte[] data, int[] offsetRef) throws IOException {
        int offset = offsetRef[0];
        StringBuilder name = new StringBuilder();
        Set<Integer> seen = new HashSet<>();
        boolean jumped = false;
        int jumpEnd = -1;

        while (true) {
            if (offset >= data.length) throw new IOException("Truncated name");
            int len = data[offset] & 0xFF;
            if ((len & 0xC0) == 0xC0) {
                if (offset + 1 >= data.length) throw new IOException("Bad pointer");
                int ptr = ((len & 0x3F) << 8) | (data[offset + 1] & 0xFF);
                if (seen.contains(ptr)) throw new IOException("Pointer loop");
                seen.add(ptr);
                if (!jumped) {
                    jumpEnd = offset + 2;
                    jumped = true;
                }
                offset = ptr;
                continue;
            } else if (len == 0) {
                offset++;
                break;
            } else {
                offset++;
                if (offset + len > data.length) throw new IOException("Label out of range");
                if (name.length() > 0) name.append('.');
                name.append(new String(data, offset, len, StandardCharsets.US_ASCII));
                offset += len;
            }
        }
        offsetRef[0] = jumped ? jumpEnd : offset;
        return name.toString();
    }

    public static DnsQuery parseQuery(byte[] data) throws IOException {
        if (data.length < 12) throw new IOException("Header too short");
        ByteBuffer bb = ByteBuffer.wrap(data);
        DnsQuery q = new DnsQuery();
        q.id = bb.getShort() & 0xFFFF;
        q.flags = bb.getShort() & 0xFFFF;
        q.qdCount = bb.getShort() & 0xFFFF;
        int an = bb.getShort() & 0xFFFF;
        int ns = bb.getShort() & 0xFFFF;
        int ar = bb.getShort() & 0xFFFF;
        if (q.qdCount < 1) throw new IOException("No questions");

        int[] off = new int[]{12};
        q.qname = decodeName(data, off);
        if (off[0] + 4 > data.length) throw new IOException("Truncated question");
        ByteBuffer qb = ByteBuffer.wrap(data, off[0], 4);
        q.qtype = qb.getShort() & 0xFFFF;
        q.qclass = qb.getShort() & 0xFFFF;
        q.questionEndOffset = off[0] + 4;
        return q;
    }

    public static List<byte[]> resolve(String qname, int qtype) {
        List<byte[]> answers = new ArrayList<>();
        try {
            // Special-case localhost to avoid external DNS dependency
            if ("localhost".equalsIgnoreCase(qname)) {
                if (qtype == TYPE_A) answers.add(InetAddress.getByName("127.0.0.1").getAddress());
                if (qtype == TYPE_AAAA) answers.add(InetAddress.getByName("::1").getAddress());
                return answers;
            }
            InetAddress[] addrs = InetAddress.getAllByName(qname);
            for (InetAddress ia : addrs) {
                if (qtype == TYPE_A && ia instanceof Inet4Address) {
                    answers.add(ia.getAddress());
                } else if (qtype == TYPE_AAAA && ia instanceof Inet6Address) {
                    answers.add(ia.getAddress());
                }
            }
        } catch (Exception ignored) {}
        return answers;
    }

    public static byte[] buildResponse(byte[] request) {
        try {
            DnsQuery q = parseQuery(request);
            boolean supported = (q.qtype == TYPE_A || q.qtype == TYPE_AAAA) && q.qclass == 1;
            List<byte[]> resolved = supported ? resolve(q.qname, q.qtype) : Collections.emptyList();
            int rcode;
            if (!supported) {
                rcode = 4; // Not Implemented
            } else if (resolved.isEmpty()) {
                rcode = 3; // Name Error
            } else {
                rcode = 0; // NoError
            }

            ByteArrayOutputStream out = new ByteArrayOutputStream(512);
            DataOutputStream dos = new DataOutputStream(out);

            int flags = 0x8000; // QR=1
            flags |= (q.flags & 0x7800); // opcode
            flags |= (q.flags & 0x0100); // RD copied
            // RA=0, AA=0, TC=0
            flags |= (rcode & 0xF);

            dos.writeShort(q.id);
            dos.writeShort(flags);
            dos.writeShort(1); // QD
            dos.writeShort((rcode == 0) ? resolved.size() : 0); // AN
            dos.writeShort(0); // NS
            dos.writeShort(0); // AR

            // Write original question
            dos.write(request, 12, q.questionEndOffset - 12);

            if (rcode == 0) {
                // Use compression pointer to 0xC00C (offset 12)
                int namePtr = 0xC00C;
                for (byte[] rd : resolved) {
                    dos.writeShort(namePtr);
                    dos.writeShort(q.qtype);
                    dos.writeShort(1); // IN
                    dos.writeInt(60); // TTL
                    dos.writeShort(rd.length);
                    dos.write(rd);
                }
            }

            byte[] resp = out.toByteArray();
            return resp.length > 512 ? Arrays.copyOf(resp, 512) : resp;
        } catch (IOException e) {
            // Form a SERVFAIL if possible
            try {
                ByteArrayOutputStream out = new ByteArrayOutputStream(512);
                DataOutputStream dos = new DataOutputStream(out);
                int id = 0;
                if (request != null && request.length >= 2) {
                    id = ((request[0] & 0xFF) << 8) | (request[1] & 0xFF);
                }
                dos.writeShort(id);
                dos.writeShort(0x8000 | 2); // QR=1, SERVFAIL
                dos.writeShort(0);
                dos.writeShort(0);
                dos.writeShort(0);
                dos.writeShort(0);
                return out.toByteArray();
            } catch (IOException ex) {
                return new byte[0];
            }
        }
    }

    public static byte[] buildQuery(int id, String name, int qtype) throws IOException {
        ByteArrayOutputStream out = new ByteArrayOutputStream(512);
        DataOutputStream dos = new DataOutputStream(out);
        dos.writeShort(id & 0xFFFF);
        dos.writeShort(0x0100); // RD=1
        dos.writeShort(1);
        dos.writeShort(0);
        dos.writeShort(0);
        dos.writeShort(0);
        dos.write(encodeName(name));
        dos.writeShort(qtype);
        dos.writeShort(1);
        return out.toByteArray();
    }

    public static void runDnsServer(String bindIp, int port, int durationSeconds) throws IOException {
        DatagramSocket socket = new DatagramSocket(null);
        socket.setReuseAddress(true);
        socket.setSoTimeout(500);
        socket.bind(new InetSocketAddress(bindIp, port));
        long end = System.nanoTime() + TimeUnit.SECONDS.toNanos(durationSeconds);
        byte[] buf = new byte[1500];
        try {
            while (System.nanoTime() < end) {
                try {
                    DatagramPacket pkt = new DatagramPacket(buf, buf.length);
                    socket.receive(pkt);
                    byte[] req = Arrays.copyOfRange(pkt.getData(), 0, pkt.getLength());
                    byte[] resp = buildResponse(req);
                    if (resp.length > 0) {
                        DatagramPacket rp = new DatagramPacket(resp, resp.length, pkt.getAddress(), pkt.getPort());
                        socket.send(rp);
                    }
                } catch (SocketTimeoutException ignored) {
                } catch (Exception e) {
                    // Ignore malformed packets
                }
            }
        } finally {
            socket.close();
        }
    }

    private static String rcodeName(int rcode) {
        switch (rcode) {
            case 0: return "NOERROR";
            case 2: return "SERVFAIL";
            case 3: return "NXDOMAIN";
            case 4: return "NOTIMP";
            default: return "RCODE(" + rcode + ")";
        }
    }

    public static void main(String[] args) throws Exception {
        final String ip = "127.0.0.1";
        final int port = 8053;

        Thread server = new Thread(() -> {
            try {
                runDnsServer(ip, port, 5);
            } catch (IOException e) {
                System.err.println("Server error: " + e.getMessage());
            }
        });
        server.setDaemon(true);
        server.start();

        DatagramSocket client = new DatagramSocket();
        client.setSoTimeout(1500);

        String[] names = new String[] {
                "localhost",
                "localhost",
                "example.com",
                "nonexistent.invalid",
                "example.com"
        };
        int[] types = new int[] { TYPE_A, TYPE_AAAA, TYPE_A, TYPE_A, 16 }; // TXT unsupported

        for (int i = 0; i < 5; i++) {
            int id = 0x1234 + i;
            byte[] q = buildQuery(id, names[i], types[i]);
            DatagramPacket dp = new DatagramPacket(q, q.length, InetAddress.getByName(ip), port);
            client.send(dp);

            try {
                byte[] buf = new byte[1500];
                DatagramPacket rp = new DatagramPacket(buf, buf.length);
                client.receive(rp);
                byte[] resp = Arrays.copyOfRange(rp.getData(), 0, rp.getLength());
                if (resp.length >= 12) {
                    int rid = ((resp[0] & 0xFF) << 8) | (resp[1] & 0xFF);
                    int flags = ((resp[2] & 0xFF) << 8) | (resp[3] & 0xFF);
                    int an = ((resp[6] & 0xFF) << 8) | (resp[7] & 0xFF);
                    int rcode = flags & 0xF;
                    System.out.println("Test " + (i + 1) + ": id=" + rid + " answers=" + an + " rcode=" + rcodeName(rcode));
                } else {
                    System.out.println("Test " + (i + 1) + ": empty response");
                }
            } catch (SocketTimeoutException te) {
                System.out.println("Test " + (i + 1) + ": timeout");
            }
        }

        client.close();
        server.join();
    }
}