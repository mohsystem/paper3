import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;

public class Lse127 {
    public static String http_hello_page(InputStream request, OutputStream response) throws IOException {
        byte[] data = request.readAllBytes();
        String username = new String(data, StandardCharsets.UTF_8).trim();
        String out = "Hello " + username;
        response.write(out.getBytes(StandardCharsets.UTF_8));
        response.flush();
        return out;
    }

    public static void main(String[] args) throws Exception {
        String[] tests = new String[] {
            "Alice",
            "  Bob  ",
            "",
            "Bob123",
            "世界"
        };
        for (int i = 0; i < tests.length; i++) {
            ByteArrayInputStream req = new ByteArrayInputStream(tests[i].getBytes(StandardCharsets.UTF_8));
            ByteArrayOutputStream res = new ByteArrayOutputStream();
            String ret = http_hello_page(req, res);
            String written = new String(res.toByteArray(), StandardCharsets.UTF_8);
            System.out.println("Test " + (i + 1) + ": return=" + ret + ", response=" + written);
        }
    }
}