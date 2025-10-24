import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicInteger;

public class Task48 {

    // Secure Client representation
    static final class Client {
        private final int id;
        private final String name;
        private final BlockingQueue<String> inbox = new LinkedBlockingQueue<>();

        Client(int id, String name) {
            this.id = id;
            this.name = name;
        }

        int getId() {
            return id;
        }

        String getName() {
            return name;
        }

        void receive(String msg) {
            if (msg == null) return;
            // Avoid unbounded growth - cap inbox size
            if (inbox.size() > 10000) {
                inbox.clear(); // backpressure strategy: drop all to prevent OOM
            }
            inbox.offer(msg);
        }

        List<String> fetchAll() {
            List<String> res = new ArrayList<>();
            inbox.drainTo(res);
            return res;
        }
    }

    // Secure Chat Server
    static final class ChatServer {
        private final AtomicInteger nextId = new AtomicInteger(1);
        private final ConcurrentHashMap<Integer, Client> clients = new ConcurrentHashMap<>();

        public Client registerClient(String name) {
            String safeName = sanitizeName(name);
            if (safeName.isEmpty()) {
                safeName = "user" + nextId.get();
            }
            int id = nextId.getAndIncrement();
            Client c = new Client(id, safeName);
            clients.put(id, c);
            return c;
        }

        public boolean disconnectClient(int clientId) {
            return clients.remove(clientId) != null;
        }

        public int getClientCount() {
            return clients.size();
        }

        public List<String> getClientMessages(int clientId) {
            Client c = clients.get(clientId);
            if (c == null) return Collections.emptyList();
            return c.fetchAll();
        }

        public int sendFrom(int clientId, String message) {
            Client sender = clients.get(clientId);
            if (sender == null) return 0;
            String sanitized = sanitizeMessage(message);
            if (sanitized.isEmpty()) return 0;
            String payload = sender.getName() + ": " + sanitized;

            // Broadcast to snapshot of clients to avoid race with removals
            int count = 0;
            for (Client c : clients.values()) {
                c.receive(payload);
                count++;
            }
            return count;
        }

        private static String sanitizeMessage(String msg) {
            if (msg == null) return "";
            StringBuilder sb = new StringBuilder();
            int maxLen = 256;
            int count = 0;
            for (int i = 0; i < msg.length() && count < maxLen; i++) {
                char ch = msg.charAt(i);
                if (ch >= 32 && ch != 127) { // printable ASCII excluding DEL
                    sb.append(ch);
                    count++;
                } else if (ch == '\n' || ch == '\r' || ch == '\t') {
                    sb.append(' ');
                    count++;
                }
                // drop other control chars
            }
            return sb.toString().trim();
        }

        private static String sanitizeName(String name) {
            if (name == null) return "";
            StringBuilder sb = new StringBuilder();
            int maxLen = 32;
            int count = 0;
            for (int i = 0; i < name.length() && count < maxLen; i++) {
                char ch = name.charAt(i);
                if (ch >= 33 && ch <= 126 && ch != ':') { // visible ASCII, exclude colon
                    sb.append(ch);
                    count++;
                }
            }
            return sb.toString().trim();
        }
    }

    // Helper to print messages received by a client
    private static void printInbox(String label, List<String> msgs) {
        System.out.println(label + " (" + msgs.size() + "):");
        for (String m : msgs) {
            System.out.println("  " + m);
        }
    }

    public static void main(String[] args) {
        ChatServer server = new ChatServer();

        // Register clients
        Client alice = server.registerClient("Alice");
        Client bob = server.registerClient("Bob");
        Client charlie = server.registerClient("Charlie");

        // Test 1: Single client receives own message
        System.out.println("Test 1: Single client receives own message");
        server.sendFrom(alice.getId(), "Hello everyone!");
        printInbox("Alice inbox after send", server.getClientMessages(alice.getId()));
        printInbox("Bob inbox", server.getClientMessages(bob.getId()));
        printInbox("Charlie inbox", server.getClientMessages(charlie.getId()));
        System.out.println("----");

        // Test 2: Two clients broadcasting, control chars sanitized
        System.out.println("Test 2: Control chars sanitized");
        server.sendFrom(bob.getId(), "Hi\nthere\tall!");
        printInbox("Alice inbox", server.getClientMessages(alice.getId()));
        printInbox("Bob inbox", server.getClientMessages(bob.getId()));
        printInbox("Charlie inbox", server.getClientMessages(charlie.getId()));
        System.out.println("----");

        // Test 3: Long message truncated securely
        System.out.println("Test 3: Long message truncated");
        char[] big = new char[400];
        Arrays.fill(big, 'X');
        String longMsg = new String(big);
        server.sendFrom(charlie.getId(), longMsg);
        printInbox("Alice inbox", server.getClientMessages(alice.getId()));
        System.out.println("----");

        // Test 4: Invalid/empty message rejected
        System.out.println("Test 4: Empty/invalid message rejected");
        int count = server.sendFrom(alice.getId(), "\u0001\u0002\u0003"); // all control chars -> sanitized to empty
        System.out.println("Broadcast count (should be 0): " + count);
        printInbox("Bob inbox", server.getClientMessages(bob.getId()));
        System.out.println("----");

        // Test 5: Disconnect client doesn't receive further messages
        System.out.println("Test 5: Disconnect behavior");
        server.disconnectClient(charlie.getId());
        server.sendFrom(bob.getId(), "After disconnect");
        printInbox("Alice inbox", server.getClientMessages(alice.getId()));
        printInbox("Charlie inbox (should be empty)", server.getClientMessages(charlie.getId()));
        System.out.println("Active clients: " + server.getClientCount());
    }
}