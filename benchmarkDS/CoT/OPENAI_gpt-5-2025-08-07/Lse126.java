import java.io.ByteArrayOutputStream;
import java.nio.charset.StandardCharsets;

public class Lse126 {

    public static String http_hello_page(String username) {
        String u = (username == null || username.isEmpty()) ? "World" : username;
        String safe = escapeHtml(u);
        String body = "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>Hello</title></head><body><h1>Hello, " + safe + "!</h1></body></html>";
        try {
            ByteArrayOutputStream responseStream = new ByteArrayOutputStream();
            responseStream.write(body.getBytes(StandardCharsets.UTF_8));
            responseStream.close();
            return new String(responseStream.toByteArray(), StandardCharsets.UTF_8);
        } catch (Exception e) {
            return "";
        }
    }

    private static String escapeHtml(String s) {
        StringBuilder sb = new StringBuilder(s.length());
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            switch (c) {
                case '&': sb.append("&amp;"); break;
                case '<': sb.append("&lt;"); break;
                case '>': sb.append("&gt;"); break;
                case '"': sb.append("&quot;"); break;
                case '\'': sb.append("&#39;"); break;
                default: sb.append(c);
            }
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "Alice",
            "",
            "Bob & Co",
            "<script>alert(1)</script>",
            "O'Neil"
        };
        for (String t : tests) {
            String resp = http_hello_page(t);
            System.out.println(resp);
        }
    }
}