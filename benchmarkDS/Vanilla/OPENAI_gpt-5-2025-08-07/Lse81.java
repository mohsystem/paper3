import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;

public class Lse81 {
    public static byte[] allocateBuffer() {
        return new byte[1024];
    }

    public static int read10CharsIntoBuf(byte[] buf) throws IOException {
        InputStream in = System.in;
        int offset = 0;
        while (offset < 10) {
            int n = in.read(buf, offset, 10 - offset);
            if (n == -1) break;
            offset += n;
            if (n == 0) break;
        }
        return offset;
    }

    public static void main(String[] args) throws Exception {
        String[] tests = {
            "abcdefghij",
            "1234567890",
            "short",
            "tenchars!!",
            "with newline\nmore"
        };
        for (int i = 0; i < tests.length; i++) {
            byte[] data = tests[i].getBytes(StandardCharsets.UTF_8);
            System.setIn(new ByteArrayInputStream(data));
            byte[] buf = allocateBuffer();
            int n = read10CharsIntoBuf(buf);
            String out = new String(buf, 0, n, StandardCharsets.UTF_8);
            System.out.println("Test" + (i + 1) + ": " + out);
        }
    }
}