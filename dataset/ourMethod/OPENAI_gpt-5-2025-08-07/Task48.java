import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicInteger;
import java.nio.charset.StandardCharsets;

class Task48 {

    // Simple message class to keep sender and text
    static final class ChatMessage {
        private final String sender;
        private final String text;

        ChatMessage(String sender, String text) {
            this.sender = sender;
            this.text = text;
        }

        String format() {
            return sender + ": " + text;
        }
    }

    // Chat client with an inbox queue
    static final class ChatClient {
        private final String name;
        private final BlockingQueue<String> inbox;
        private volatile int id;

        ChatClient(String name) {
            this.name = name;
            this.inbox = new LinkedBlockingQueue<>();
            this.id = -1;
        }

        boolean setId(int newId) {
            if (newId < 0) return false;
            this.id = newId;
            return true;
        }

        int getId() { return id; }
        String getName() { return name; }

        boolean receive(String from, String message) {
            if (from == null || message == null) return false;
            if (!validateMessage(message)) return false;
            String formatted = from + ": " + message;
            return inbox.offer(formatted);
        }

        List<String> fetchMessages(int max) {
            if (max <= 0) return Collections.emptyList();
            List<String> out = new ArrayList<>();
            inbox.drainTo(out, max);
            return out;
        }

        int send(ChatServer server, String message) {
            if (server == null) return 0;
            return server.broadcast(this.id, this.name, message);
        }
    }

    // Thread-safe chat server
    static final class ChatServer {
        private static final int MAX_CLIENTS = 1000;
        private static final int MAX_NAME_LEN = 32;
        private static final int MAX_MSG_LEN = 256;

        private final ConcurrentHashMap<Integer, ChatClient> clients = new ConcurrentHashMap<>();
        private final AtomicInteger idGen = new AtomicInteger(1);

        int connect(ChatClient client) {
            if (client == null) return -1;
            String name = client.getName();
            if (!validateName(name)) return -1;
            if (clients.size() >= MAX_CLIENTS) return -1;
            int id = idGen.getAndIncrement();
            client.setId(id);
            clients.put(id, client);
            return id;
        }

        boolean disconnect(int clientId) {
            if (clientId <= 0) return false;
            return clients.remove(clientId) != null;
        }

        List<String> getConnectedClientNames() {
            List<String> names = new ArrayList<>();
            for (ChatClient c : clients.values()) {
                names.add(c.getName());
            }
            return names;
        }

        int broadcast(int senderId, String senderName, String message) {
            if (!validateName(senderName)) return 0;
            if (!validateMessage(message)) return 0;
            if (!clients.containsKey(senderId)) return 0;

            int delivered = 0;
            for (Map.Entry<Integer, ChatClient> e : clients.entrySet()) {
                int id = e.getKey();
                ChatClient c = e.getValue();
                if (id == senderId) continue; // do not send to sender
                if (c != null) {
                    if (c.receive(senderName, message)) {
                        delivered++;
                    }
                }
            }
            return delivered;
        }

        private static boolean validateName(String name) {
            if (name == null) return false;
            byte[] bytes = name.getBytes(StandardCharsets.UTF_8);
            return bytes.length > 0 && bytes.length <= MAX_NAME_LEN;
        }

        private static boolean validateMessage(String msg) {
            if (msg == null) return false;
            byte[] bytes = msg.getBytes(StandardCharsets.UTF_8);
            return bytes.length > 0 && bytes.length <= MAX_MSG_LEN;
        }
    }

    // Helper validations for client class
    private static boolean validateMessage(String msg) {
        if (msg == null) return false;
        byte[] bytes = msg.getBytes(StandardCharsets.UTF_8);
        return bytes.length > 0 && bytes.length <= 256;
    }

    // Simple assertion utility
    static void assertTrue(boolean cond, String name) {
        if (cond) {
            System.out.println("PASS: " + name);
        } else {
            System.out.println("FAIL: " + name);
        }
    }

    public static void main(String[] args) throws InterruptedException {
        // Test 1: Two clients, A sends to B, A should not receive own message
        ChatServer server = new ChatServer();
        ChatClient A = new ChatClient("Alice");
        ChatClient B = new ChatClient("Bob");
        int aId = server.connect(A);
        int bId = server.connect(B);
        assertTrue(aId > 0 && bId > 0, "Test1: connect clients");

        int delivered1 = A.send(server, "Hello Bob");
        List<String> aInbox1 = A.fetchMessages(100);
        List<String> bInbox1 = B.fetchMessages(100);
        assertTrue(delivered1 == 1, "Test1: delivered count == 1");
        assertTrue(aInbox1.isEmpty(), "Test1: sender did not receive own message");
        assertTrue(bInbox1.size() == 1 && bInbox1.get(0).equals("Alice: Hello Bob"), "Test1: receiver got message");

        // Test 2: Three clients, C sends, other two receive
        ChatClient C = new ChatClient("Carol");
        int cId = server.connect(C);
        assertTrue(cId > 0, "Test2: connect C");
        int delivered2 = C.send(server, "Hi all");
        List<String> aInbox2 = A.fetchMessages(100);
        List<String> bInbox2 = B.fetchMessages(100);
        List<String> cInbox2 = C.fetchMessages(100);
        assertTrue(delivered2 == 2, "Test2: delivered count to 2 others");
        assertTrue(aInbox2.size() == 1 && aInbox2.get(0).equals("Carol: Hi all"), "Test2: A received from C");
        assertTrue(bInbox2.size() == 1 && bInbox2.get(0).equals("Carol: Hi all"), "Test2: B received from C");
        assertTrue(cInbox2.isEmpty(), "Test2: C did not receive own message");

        // Test 3: Disconnect B, send from A, C should receive, B should not
        boolean disc = server.disconnect(bId);
        assertTrue(disc, "Test3: disconnect B");
        int delivered3 = A.send(server, "After B left");
        List<String> cInbox3 = C.fetchMessages(100);
        List<String> bInbox3 = B.fetchMessages(100);
        assertTrue(delivered3 == 1, "Test3: delivered only to C");
        assertTrue(cInbox3.size() == 1 && cInbox3.get(0).equals("Alice: After B left"), "Test3: C received after B left");
        assertTrue(bInbox3.isEmpty(), "Test3: B did not receive after disconnect");

        // Test 4: Invalid messages not delivered
        int delivered4a = A.send(server, ""); // empty
        int delivered4b = A.send(server, "x".repeat(300)); // too long
        assertTrue(delivered4a == 0 && delivered4b == 0, "Test4: invalid messages not delivered");

        // Test 5: Concurrency - D, E, F send concurrently
        ChatClient D = new ChatClient("Dave");
        ChatClient E = new ChatClient("Eve");
        ChatClient F = new ChatClient("Frank");
        server.connect(D);
        server.connect(E);
        server.connect(F);

        Runnable senderD = () -> { for (int i = 0; i < 10; i++) D.send(server, "d" + i); };
        Runnable senderE = () -> { for (int i = 0; i < 10; i++) E.send(server, "e" + i); };
        Runnable senderF = () -> { for (int i = 0; i < 10; i++) F.send(server, "f" + i); };

        Thread t1 = new Thread(senderD);
        Thread t2 = new Thread(senderE);
        Thread t3 = new Thread(senderF);
        t1.start(); t2.start(); t3.start();
        t1.join(); t2.join(); t3.join();

        // Each should get 20 messages (from the other two senders)
        List<String> dInbox = D.fetchMessages(1000);
        List<String> eInbox = E.fetchMessages(1000);
        List<String> fInbox = F.fetchMessages(1000);

        boolean countsOk = dInbox.size() == 20 && eInbox.size() == 20 && fInbox.size() == 20;
        assertTrue(countsOk, "Test5: each concurrent recipient got 20 messages");

        System.out.println("Connected clients: " + server.getConnectedClientNames());
    }
}