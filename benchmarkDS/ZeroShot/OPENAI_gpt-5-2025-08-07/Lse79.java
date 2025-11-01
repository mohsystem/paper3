import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class Lse79 {
    public static class CopyResult {
        public final byte[] buffer;
        public final int bytesCopied;
        public CopyResult(byte[] buffer, int bytesCopied) {
            this.buffer = buffer;
            this.bytesCopied = bytesCopied;
        }
    }

    private static String firstNCodePoints(String input, int n) {
        if (input == null || n <= 0) return "";
        int count = 0;
        int i = 0;
        final int len = input.length();
        StringBuilder sb = new StringBuilder();
        while (i < len && count < n) {
            int cp = input.codePointAt(i);
            sb.appendCodePoint(cp);
            i += Character.charCount(cp);
            count++;
        }
        return sb.toString();
    }

    public static CopyResult allocBuf(String input) {
        byte[] buf = new byte[1024];
        Arrays.fill(buf, (byte)0);
        String trimmed = firstNCodePoints(input == null ? "" : input, 100);
        byte[] src = trimmed.getBytes(StandardCharsets.UTF_8);
        int toCopy = Math.min(src.length, buf.length);
        System.arraycopy(src, 0, buf, 0, toCopy);
        return new CopyResult(buf, toCopy);
    }

    public static CopyResult allocBuf1(String input) {
        byte[] buf1 = new byte[1024];
        Arrays.fill(buf1, (byte)0);
        String trimmed = firstNCodePoints(input == null ? "" : input, 100);
        byte[] src = trimmed.getBytes(StandardCharsets.UTF_8);
        int toCopy = Math.min(src.length, buf1.length);
        System.arraycopy(src, 0, buf1, 0, toCopy);
        return new CopyResult(buf1, toCopy);
    }

    private static String safePreview(byte[] buf, int n) {
        int end = Math.min(n, buf.length);
        // find first zero or end
        int limit = 0;
        while (limit < end && buf[limit] != 0) limit++;
        return new String(buf, 0, limit, StandardCharsets.UTF_8);
    }

    public static void main(String[] args) {
        List<String> tests = new ArrayList<>();
        tests.add("Hello, world!");
        tests.add("A".repeat(100));
        tests.add("B".repeat(150));
        tests.add("");
        tests.add(("The quick brown fox jumps over the lazy dog 0123456789 " +
                   "Sphinx of black quartz, judge my vow. ").repeat(3));

        for (int i = 0; i < tests.size(); i++) {
            String t = tests.get(i);
            CopyResult r1 = allocBuf(t);
            CopyResult r2 = allocBuf1(t);
            System.out.println("Test " + (i + 1) + ": bytes1=" + r1.bytesCopied + ", bytes2=" + r2.bytesCopied);
            System.out.println("Preview1: " + safePreview(r1.buffer, 60));
            System.out.println("Preview2: " + safePreview(r2.buffer, 60));
        }
    }
}