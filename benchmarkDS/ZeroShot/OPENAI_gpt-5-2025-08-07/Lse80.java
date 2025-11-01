import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;

public class Lse80 {

    // Secure scan function: reads up to 1023 characters until newline; consumes newline; returns count.
    public static int scan(InputStream in, byte[] buffer) throws IOException {
        if (in == null || buffer == null || buffer.length == 0) {
            return 0;
        }
        int max = Math.min(1023, buffer.length - 1); // ensure space and enforce 1023 char limit
        int count = 0;

        while (true) {
            int b = in.read();
            if (b == -1) {
                break; // EOF
            }
            if (b == '\n') {
                break; // consume newline but do not store
            }
            if (count < max) {
                buffer[count++] = (byte) b;
            } else {
                // buffer full; discard until newline or EOF
                do {
                    b = in.read();
                    if (b == -1 || b == '\n') {
                        break;
                    }
                } while (true);
                break;
            }
        }
        return count;
    }

    private static void runTest(String label, String input) throws IOException {
        byte[] buf = new byte[1024];
        try (InputStream in = new ByteArrayInputStream(input.getBytes(StandardCharsets.UTF_8))) {
            int n = scan(in, buf);
            String out = new String(buf, 0, n, StandardCharsets.UTF_8);
            System.out.println(label + " -> count=" + n + ", out=\"" + out + "\"");
        }
    }

    public static void main(String[] args) throws Exception {
        runTest("Test1", "Hello, World!\nTrailing");
        runTest("Test2", "");
        runTest("Test3", "NoNewline");
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < 1500; i++) sb.append('A');
        sb.append('\n').append("BBBB");
        runTest("Test4", sb.toString());
        runTest("Test5", "Line1\nLine2\n");
    }
}