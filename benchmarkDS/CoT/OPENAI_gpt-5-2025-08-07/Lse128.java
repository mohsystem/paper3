import java.io.*;

public class Lse128 {

    public static String http_hello_page(InputStream request, OutputStream response) throws IOException {
        String username = readAll(request).trim();
        String message = "hello " + username;
        response.write(message.getBytes("UTF-8"));
        response.flush();
        return message;
    }

    private static String readAll(InputStream in) throws IOException {
        ByteArrayOutputStream buffer = new ByteArrayOutputStream();
        byte[] tmp = new byte[4096];
        int n;
        while ((n = in.read(tmp)) != -1) {
            buffer.write(tmp, 0, n);
        }
        return buffer.toString("UTF-8");
    }

    public static void main(String[] args) throws Exception {
        // Test case 1
        ByteArrayInputStream req1 = new ByteArrayInputStream("Alice".getBytes("UTF-8"));
        ByteArrayOutputStream resp1 = new ByteArrayOutputStream();
        String out1 = http_hello_page(req1, resp1);
        System.out.println(resp1.toString("UTF-8"));

        // Test case 2
        ByteArrayInputStream req2 = new ByteArrayInputStream("Bob".getBytes("UTF-8"));
        ByteArrayOutputStream resp2 = new ByteArrayOutputStream();
        String out2 = http_hello_page(req2, resp2);
        System.out.println(resp2.toString("UTF-8"));

        // Test case 3
        ByteArrayInputStream req3 = new ByteArrayInputStream("   Charlie   ".getBytes("UTF-8"));
        ByteArrayOutputStream resp3 = new ByteArrayOutputStream();
        String out3 = http_hello_page(req3, resp3);
        System.out.println(resp3.toString("UTF-8"));

        // Test case 4 (empty)
        ByteArrayInputStream req4 = new ByteArrayInputStream("".getBytes("UTF-8"));
        ByteArrayOutputStream resp4 = new ByteArrayOutputStream();
        String out4 = http_hello_page(req4, resp4);
        System.out.println(resp4.toString("UTF-8"));

        // Test case 5 (with newline)
        ByteArrayInputStream req5 = new ByteArrayInputStream("Dana\n".getBytes("UTF-8"));
        ByteArrayOutputStream resp5 = new ByteArrayOutputStream();
        String out5 = http_hello_page(req5, resp5);
        System.out.println(resp5.toString("UTF-8"));
    }
}