import java.io.Reader;
import java.io.StringReader;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.io.IOException;

public class Lse80 {
    private static final int BUFFER_SIZE = 1024; // max store 1023 chars + null-terminator conceptually

    public static final class ScanResult {
        public final String data;
        public final int count;
        public ScanResult(String data, int count) {
            this.data = data;
            this.count = count;
        }
    }

    // Reads from provided Reader until newline or EOF, consumes newline but does not store it.
    // Stores at most 1023 characters. Returns both data and count.
    public static ScanResult scan(Reader reader) {
        if (reader == null) {
            return new ScanResult("", 0);
        }
        StringBuilder sb = new StringBuilder(Math.min(64, BUFFER_SIZE - 1));
        int stored = 0;
        boolean done = false;
        try {
            while (!done) {
                int r = reader.read();
                if (r == -1) {
                    break; // EOF
                }
                char ch = (char) r;
                if (ch == '\n') {
                    // consume newline, do not store
                    break;
                }
                if (stored < BUFFER_SIZE - 1) {
                    sb.append(ch);
                    stored++;
                } else {
                    // buffer full; keep consuming until newline/EOF
                    // do nothing (discard)
                }
            }
            // If buffer full and we didn't hit newline, continue consuming until newline/EOF
            if (stored >= BUFFER_SIZE - 1) {
                int r;
                while ((r = reader.read()) != -1) {
                    if ((char) r == '\n') break;
                }
            }
        } catch (IOException e) {
            // Treat as EOF on error
        }
        return new ScanResult(sb.toString(), stored);
    }

    // Optional convenience wrapper to read from System.in
    public static ScanResult scan() {
        try (InputStreamReader in = new InputStreamReader(System.in, StandardCharsets.UTF_8)) {
            return scan(in);
        } catch (IOException e) {
            return new ScanResult("", 0);
        }
    }

    public static void main(String[] args) {
        // 5 test cases using StringReader to simulate input source
        String t1 = "Hello\n";
        String t2 = "World";
        String t3 = "\n";
        StringBuilder sb4 = new StringBuilder();
        for (int i = 0; i < 1023; i++) sb4.append('A');
        sb4.append('\n');
        String t4 = sb4.toString();
        StringBuilder sb5 = new StringBuilder();
        for (int i = 0; i < 1500; i++) sb5.append('B');
        sb5.append('\n');
        String t5 = sb5.toString();

        ScanResult r1 = scan(new StringReader(t1));
        System.out.println("T1 count=" + r1.count + " data=\"" + r1.data + "\"");

        ScanResult r2 = scan(new StringReader(t2));
        System.out.println("T2 count=" + r2.count + " data=\"" + r2.data + "\"");

        ScanResult r3 = scan(new StringReader(t3));
        System.out.println("T3 count=" + r3.count + " data=\"" + r3.data + "\"");

        ScanResult r4 = scan(new StringReader(t4));
        System.out.println("T4 count=" + r4.count + " data length=" + r4.data.length() + " first5=\"" + r4.data.substring(0, Math.min(5, r4.data.length())) + "\" last5=\"" + r4.data.substring(Math.max(0, r4.data.length()-5)) + "\"");

        ScanResult r5 = scan(new StringReader(t5));
        System.out.println("T5 count=" + r5.count + " data length=" + r5.data.length() + " first5=\"" + r5.data.substring(0, Math.min(5, r5.data.length())) + "\" last5=\"" + r5.data.substring(Math.max(0, r5.data.length()-5)) + "\"");
    }
}