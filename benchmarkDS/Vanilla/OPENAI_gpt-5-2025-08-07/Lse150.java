import java.util.ArrayList;
import java.util.List;

public class Lse150 {
    static class Message {
        String username;
        String message;
        Message(String u, String m) { this.username = u; this.message = m; }
    }

    private static final List<Message> DB = new ArrayList<>();

    public static void insert_user_message_in_db(String username, String message) {
        if (username == null) username = "";
        if (message == null) message = "";
        DB.add(new Message(username, message));
    }

    public static String post(String username, String message) {
        insert_user_message_in_db(username, message);
        return "REDIRECT:/";
    }

    public static String main_page() {
        StringBuilder sb = new StringBuilder();
        sb.append("<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>Main</title></head><body>");
        sb.append("<h1>Message Board</h1>");
        sb.append("<form method=\"post\" action=\"/post\">");
        sb.append("Username: <input type=\"text\" name=\"username\"/><br/>");
        sb.append("Message: <textarea name=\"message\"></textarea><br/>");
        sb.append("<button type=\"submit\">Submit</button>");
        sb.append("</form>");
        sb.append("<hr/><ul>");
        for (Message m : DB) {
            sb.append("<li><strong>")
              .append(escapeHtml(m.username))
              .append(":</strong> ")
              .append(escapeHtml(m.message))
              .append("</li>");
        }
        sb.append("</ul></body></html>");
        return sb.toString();
    }

    private static String escapeHtml(String s) {
        StringBuilder out = new StringBuilder();
        for (char c : s.toCharArray()) {
            switch (c) {
                case '&': out.append("&amp;"); break;
                case '<': out.append("&lt;"); break;
                case '>': out.append("&gt;"); break;
                case '"': out.append("&quot;"); break;
                case '\'': out.append("&#39;"); break;
                default: out.append(c);
            }
        }
        return out.toString();
    }

    public static void main(String[] args) {
        System.out.println(main_page());
        System.out.println(post("alice", "Hello!"));
        System.out.println(post("bob", "Hi there"));
        System.out.println(post("carol", "This is a test"));
        System.out.println(post("dave", "Another message"));
        System.out.println(post("eve", "Last one"));
        System.out.println(main_page());
    }
}