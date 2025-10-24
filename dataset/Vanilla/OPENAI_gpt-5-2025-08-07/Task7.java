import java.io.*;
import java.nio.file.*;
import java.util.*;

public class Task7 {

    private static final String USER_DB = "users.db";
    private static final Map<String, Boolean> sessions = new HashMap<>();

    // Client: Build a login request
    public static String clientBuildLoginRequest(String username, String password) {
        return "LOGIN|" + username + "|" + password;
    }

    // Client: Send login (simulate transmission to server)
    public static String clientSendLogin(String username, String password) {
        String req = clientBuildLoginRequest(username, password);
        return serverProcess(req);
    }

    // Server: Ensure a user database exists with sample data
    public static void ensureUserDB(Map<String, String> users) {
        try {
            List<String> lines = new ArrayList<>();
            for (Map.Entry<String, String> e : users.entrySet()) {
                lines.add(e.getKey() + ":" + e.getValue());
            }
            Files.write(Paths.get(USER_DB), lines);
        } catch (IOException e) {
            // Handle silently for this context
        }
    }

    // Server: Authenticate against filesystem
    public static boolean authenticate(String username, String password) {
        try (BufferedReader br = new BufferedReader(new FileReader(USER_DB))) {
            String line;
            while ((line = br.readLine()) != null) {
                int idx = line.indexOf(':');
                if (idx > -1) {
                    String u = line.substring(0, idx);
                    String p = line.substring(idx + 1);
                    if (u.equals(username) && p.equals(password)) {
                        return true;
                    }
                }
            }
        } catch (IOException e) {
            // Ignore for simplicity
        }
        return false;
    }

    // Server: Process requests
    public static String serverProcess(String request) {
        if (request == null) return "ERR|INVALID|EmptyRequest";
        String[] parts = request.split("\\|", -1);
        if (parts.length == 0) return "ERR|INVALID|Malformed";
        String action = parts[0].trim().toUpperCase(Locale.ROOT);

        switch (action) {
            case "LOGIN":
                if (parts.length != 3) return "ERR|LOGIN|InvalidArgs";
                String username = parts[1];
                String password = parts[2];
                if (authenticate(username, password)) {
                    sessions.put(username, true);
                    return "OK|LOGIN|Welcome " + username;
                } else {
                    return "ERR|LOGIN|Unauthorized";
                }
            case "SEND":
                if (parts.length < 4) return "ERR|SEND|InvalidArgs";
                String from = parts[1];
                String to = parts[2];
                String message = request.substring(request.indexOf(to, request.indexOf(from) + from.length()) + to.length() + 1); // preserve '|' in message if any
                if (!sessions.getOrDefault(from, false)) {
                    return "ERR|SEND|NotAuthenticated";
                }
                // Append to chat log (simulate delivering)
                try (FileWriter fw = new FileWriter("chat.log", true)) {
                    fw.write("FROM:" + from + " TO:" + to + " MSG:" + message + System.lineSeparator());
                } catch (IOException e) {
                    return "ERR|SEND|StorageError";
                }
                return "OK|SEND|DeliveredTo|" + to;
            default:
                return "ERR|INVALID|UnknownAction";
        }
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        Map<String, String> sampleUsers = new HashMap<>();
        sampleUsers.put("alice", "password123");
        sampleUsers.put("bob", "letmein");
        ensureUserDB(sampleUsers);

        // Test 1: valid login
        String t1 = clientSendLogin("alice", "password123");
        System.out.println("Test1: " + t1);

        // Test 2: invalid password
        String t2 = clientSendLogin("alice", "wrong");
        System.out.println("Test2: " + t2);

        // Test 3: non-existent user
        String t3 = clientSendLogin("charlie", "nopass");
        System.out.println("Test3: " + t3);

        // Test 4: valid login for bob
        String t4 = clientSendLogin("bob", "letmein");
        System.out.println("Test4: " + t4);

        // Test 5: send message after successful login
        String t5 = serverProcess("SEND|alice|bob|Hello Bob!");
        System.out.println("Test5: " + t5);
    }
}