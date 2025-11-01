import java.io.*;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;

public class Lse128 {
    public static String http_hello_page(InputStream request, OutputStream response) throws IOException {
        String username = readAllToString(request, StandardCharsets.UTF_8);
        username = username.trim();
        String message = "hello " + username;
        response.write(message.getBytes(StandardCharsets.UTF_8));
        response.flush();
        return message;
    }

    private static String readAllToString(InputStream in, Charset cs) throws IOException {
        ByteArrayOutputStream bout = new ByteArrayOutputStream();
        byte[] buf = new byte[4096];
        int n;
        while ((n = in.read(buf)) != -1) {
            bout.write(buf, 0, n);
        }
        return bout.toString(cs.name());
    }

    public static void main(String[] args) throws Exception {
        String[] usernames = new String[] {"", "Alice", "Bob", "  Charlie  ", "Dana_Smith"};
        for (String u : usernames) {
            ByteArrayInputStream req = new ByteArrayInputStream(u.getBytes(StandardCharsets.UTF_8));
            ByteArrayOutputStream resp = new ByteArrayOutputStream();
            String out = http_hello_page(req, resp);
            System.out.println(out);
        }
    }
}