import java.nio.charset.StandardCharsets;

public class Lse121 {
    public static String simulateMemory(String input) {
        byte[] chunk1 = input.getBytes(StandardCharsets.UTF_8);
        byte[] chunk2 = new byte[chunk1.length];
        System.arraycopy(chunk1, 0, chunk2, 0, chunk1.length);

        String before = new String(chunk2, StandardCharsets.UTF_8);
        System.out.println(before);

        chunk1 = null;

        String after = new String(chunk2, StandardCharsets.UTF_8);
        System.out.println(after);

        chunk2 = null;

        return before + "\n" + after;
    }

    public static void main(String[] args) {
        String[] tests = {
            "Test1",
            "Second",
            "A longer string for memory copy",
            "1234567890",
            "EdgeCase!"
        };
        for (String t : tests) {
            simulateMemory(t);
        }
    }
}