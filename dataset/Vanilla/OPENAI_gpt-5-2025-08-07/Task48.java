import java.util.*;
import java.util.concurrent.*;

public class Task48 {

    // Simple in-memory chat server
    public static class ChatServer {
        private final ConcurrentMap<String, BlockingQueue<String>> clients = new ConcurrentHashMap<>();

        public boolean registerClient(String name, BlockingQueue<String> inbox) {
            if (name == null || name.isEmpty() || inbox == null) return false;
            return clients.putIfAbsent(name, inbox) == null;
        }

        public boolean unregisterClient(String name) {
            return clients.remove(name) != null;
        }

        public void broadcast(String from, String message) {
            String full = from + ": " + message;
            for (BlockingQueue<String> q : clients.values()) {
                q.offer(full);
            }
        }

        public Set<String> getConnectedClients() {
            return new HashSet<>(clients.keySet());
        }
    }

    // Simple in-memory chat client
    public static class ChatClient {
        private final String name;
        private final ChatServer server;
        private final BlockingQueue<String> inbox = new LinkedBlockingQueue<>();

        public ChatClient(String name, ChatServer server) {
            this.name = name;
            this.server = server;
        }

        public boolean connect() {
            return server.registerClient(name, inbox);
        }

        public boolean disconnect() {
            return server.unregisterClient(name);
        }

        public void send(String message) {
            server.broadcast(name, message);
        }

        public List<String> receiveAll() {
            List<String> out = new ArrayList<>();
            inbox.drainTo(out);
            return out;
        }

        public String getName() { return name; }
    }

    // Tests
    private static void test1_singleClient() {
        System.out.println("Test1: Single client broadcast to self");
        ChatServer server = new ChatServer();
        ChatClient a = new ChatClient("Alice", server);
        a.connect();
        a.send("Hello World");
        System.out.println("Alice inbox: " + a.receiveAll());
    }

    private static void test2_twoClients() {
        System.out.println("Test2: Two clients exchange");
        ChatServer server = new ChatServer();
        ChatClient a = new ChatClient("Alice", server);
        ChatClient b = new ChatClient("Bob", server);
        a.connect(); b.connect();
        a.send("Hi Bob");
        b.send("Hi Alice");
        System.out.println("Alice inbox: " + a.receiveAll());
        System.out.println("Bob inbox: " + b.receiveAll());
    }

    private static void test3_threeClientsBroadcast() {
        System.out.println("Test3: Three clients, multi-broadcast");
        ChatServer server = new ChatServer();
        ChatClient a = new ChatClient("A", server);
        ChatClient b = new ChatClient("B", server);
        ChatClient c = new ChatClient("C", server);
        a.connect(); b.connect(); c.connect();
        a.send("One");
        b.send("Two");
        c.send("Three");
        System.out.println("A inbox: " + a.receiveAll());
        System.out.println("B inbox: " + b.receiveAll());
        System.out.println("C inbox: " + c.receiveAll());
    }

    private static void test4_disconnect() {
        System.out.println("Test4: Client disconnects");
        ChatServer server = new ChatServer();
        ChatClient a = new ChatClient("A", server);
        ChatClient b = new ChatClient("B", server);
        a.connect(); b.connect();
        a.send("Before leave");
        b.disconnect();
        a.send("After B left");
        System.out.println("A inbox: " + a.receiveAll());
        // B should still have messages sent before leaving only
        System.out.println("B inbox (should only contain first): " + b.receiveAll());
    }

    private static void test5_fiveClients() {
        System.out.println("Test5: Five clients flood");
        ChatServer server = new ChatServer();
        ChatClient[] clients = new ChatClient[5];
        for (int i = 0; i < 5; i++) {
            clients[i] = new ChatClient("C" + i, server);
            clients[i].connect();
        }
        clients[0].send("Msg0");
        clients[1].send("Msg1");
        clients[2].send("Msg2");
        clients[3].send("Msg3");
        clients[4].send("Msg4");
        for (int i = 0; i < 5; i++) {
            System.out.println(clients[i].getName() + " inbox: " + clients[i].receiveAll());
        }
    }

    public static void main(String[] args) {
        test1_singleClient();
        test2_twoClients();
        test3_threeClientsBroadcast();
        test4_disconnect();
        test5_fiveClients();
    }
}