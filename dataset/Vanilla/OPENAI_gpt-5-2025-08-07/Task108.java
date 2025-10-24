import java.io.*;
import java.net.*;
import java.util.*;
import java.util.concurrent.*;

public class Task108 {
    static final int PORT = 5050;

    static class RemoteServer implements Runnable {
        private final int port;
        private volatile boolean running = true;
        private ServerSocket serverSocket;
        private final Map<String, Map<String, String>> store = new ConcurrentHashMap<>();

        RemoteServer(int port) {
            this.port = port;
        }

        @Override
        public void run() {
            try (ServerSocket ss = new ServerSocket(port)) {
                serverSocket = ss;
                while (running) {
                    try {
                        Socket client = ss.accept();
                        handleClient(client);
                    } catch (SocketException se) {
                        if (!running) break;
                    }
                }
            } catch (IOException e) {
                // Server closed or failed to bind
            }
        }

        private void handleClient(Socket client) {
            try (BufferedReader in = new BufferedReader(new InputStreamReader(client.getInputStream()));
                 BufferedWriter out = new BufferedWriter(new OutputStreamWriter(client.getOutputStream()))) {
                String line;
                while ((line = in.readLine()) != null) {
                    String response = process(line);
                    out.write(response);
                    out.write("\n");
                    out.flush();
                    if ("EXIT".equalsIgnoreCase(line.trim())) break;
                    if ("SHUTDOWN".equalsIgnoreCase(line.trim())) break;
                }
            } catch (IOException ignored) {
            }
        }

        private String process(String cmdLine) {
            String line = cmdLine.trim();
            if (line.isEmpty()) return "ERROR Empty command";
            String[] parts = line.split(" ", 4);
            String cmd = parts[0].toUpperCase(Locale.ROOT);
            switch (cmd) {
                case "CREATE": {
                    if (parts.length < 2) return "ERROR Usage: CREATE <id>";
                    String id = parts[1];
                    store.computeIfAbsent(id, k -> new ConcurrentHashMap<>());
                    return "OK";
                }
                case "SET": {
                    if (parts.length < 4) return "ERROR Usage: SET <id> <key> <value>";
                    String id = parts[1], key = parts[2], value = parts[3];
                    Map<String, String> obj = store.get(id);
                    if (obj == null) return "ERROR NotFound";
                    obj.put(key, value);
                    return "OK";
                }
                case "GET": {
                    if (parts.length < 3) return "ERROR Usage: GET <id> <key>";
                    String id = parts[1], key = parts[2];
                    Map<String, String> obj = store.get(id);
                    if (obj == null) return "ERROR NotFound";
                    String val = obj.get(key);
                    if (val == null) return "ERROR NotFound";
                    return "VALUE " + val;
                }
                case "DELETE": {
                    if (parts.length < 2) return "ERROR Usage: DELETE <id>";
                    store.remove(parts[1]);
                    return "OK";
                }
                case "KEYS": {
                    if (parts.length < 2) return "ERROR Usage: KEYS <id>";
                    Map<String, String> obj = store.get(parts[1]);
                    if (obj == null) return "ERROR NotFound";
                    List<String> keys = new ArrayList<>(obj.keySet());
                    Collections.sort(keys);
                    return "KEYS " + String.join(",", keys);
                }
                case "LIST": {
                    List<String> ids = new ArrayList<>(store.keySet());
                    Collections.sort(ids);
                    return "LIST " + String.join(",", ids);
                }
                case "EXIT": {
                    return "OK BYE";
                }
                case "SHUTDOWN": {
                    running = false;
                    try { if (serverSocket != null) serverSocket.close(); } catch (IOException ignored) {}
                    return "OK SHUTDOWN";
                }
                default:
                    return "ERROR UnknownCommand";
            }
        }
    }

    // Client helpers (functions take parameters and return output)
    public static String sendCommand(String cmd) {
        try (Socket s = new Socket("127.0.0.1", PORT);
             BufferedReader in = new BufferedReader(new InputStreamReader(s.getInputStream()));
             BufferedWriter out = new BufferedWriter(new OutputStreamWriter(s.getOutputStream()))) {
            out.write(cmd);
            out.write("\n");
            out.flush();
            String resp = in.readLine();
            return resp == null ? "" : resp;
        } catch (IOException e) {
            return "ERROR " + e.getMessage();
        }
    }
    public static String create(String id) { return sendCommand("CREATE " + id); }
    public static String set(String id, String key, String value) { return sendCommand("SET " + id + " " + key + " " + value); }
    public static String get(String id, String key) { return sendCommand("GET " + id + " " + key); }
    public static String delete(String id) { return sendCommand("DELETE " + id); }
    public static String keys(String id) { return sendCommand("KEYS " + id); }
    public static String list() { return sendCommand("LIST"); }
    public static String shutdown() { return sendCommand("SHUTDOWN"); }

    public static void main(String[] args) throws Exception {
        Thread serverThread = new Thread(new RemoteServer(PORT));
        serverThread.start();
        Thread.sleep(200);

        // Test case 1: Create and Set/Get
        System.out.println(create("foo"));
        System.out.println(set("foo", "a", "1"));
        System.out.println(get("foo", "a"));

        // Test case 2: Create second object and KEYS
        System.out.println(create("bar"));
        System.out.println(set("bar", "x", "hello world"));
        System.out.println(keys("bar"));

        // Test case 3: LIST objects
        System.out.println(list());

        // Test case 4: DELETE and LIST
        System.out.println(delete("foo"));
        System.out.println(list());

        // Test case 5: Error on missing key/object
        System.out.println(get("foo", "a"));
        System.out.println(get("bar", "missing"));

        System.out.println(shutdown());
        serverThread.join();
    }
}