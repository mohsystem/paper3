import java.util.ArrayList;
import java.util.List;

public class Lse148 {
    static class Message {
        String username;
        String content;
        Message(String u, String c) { this.username = u; this.content = c; }
    }

    private static final List<Message> db = new ArrayList<>();

    public static boolean insert_user_message_in_db(String username, String message) {
        if (username == null) username = "";
        if (message == null) message = "";
        db.add(new Message(username, message));
        return true;
    }

    public static String post(String username, String message) {
        insert_user_message_in_db(username, message);
        return "REDIRECT:/";
    }

    public static String displayMessages() {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < db.size(); i++) {
            Message m = db.get(i);
            sb.append(i + 1).append(". ").append(m.username).append(": ").append(m.content);
            if (i + 1 < db.size()) sb.append("\n");
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        System.out.println(post("alice", "Hello world"));
        System.out.println(post("bob", "Hi Alice"));
        System.out.println(post("carol", "Good morning"));
        System.out.println(post("dave", "How are you?"));
        System.out.println(post("eve", "All good!"));
        System.out.println("Main page messages:");
        System.out.println(displayMessages());
    }
}