import java.io.*;

public class Lse127 {
    public static String http_hello_page(InputStream request, OutputStream response) throws IOException {
        ByteArrayOutputStream buf = new ByteArrayOutputStream();
        byte[] tmp = new byte[1024];
        int n;
        while ((n = request.read(tmp)) != -1) {
            buf.write(tmp, 0, n);
        }
        String username = buf.toString("UTF-8").trim();
        String message = "Hello " + username;
        response.write(message.getBytes("UTF-8"));
        return message;
        }
    public static void main(String[] args) throws Exception {
        String[] usernames = {"Alice", "Bob", "Charlie", "", "Eve Adams"};
        for (String name : usernames) {
            InputStream req = new ByteArrayInputStream(name.getBytes("UTF-8"));
            ByteArrayOutputStream resp = new ByteArrayOutputStream();
            String result = http_hello_page(req, resp);
            System.out.println("Returned: " + result);
            System.out.println("Response Stream: " + resp.toString("UTF-8"));
        }
    }
}