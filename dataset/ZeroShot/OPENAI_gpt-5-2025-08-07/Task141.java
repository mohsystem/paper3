import java.nio.charset.StandardCharsets;

public class Task141 {
    public static String reverseString(String input) {
        if (input == null) {
            return "";
        }
        // Reverse by Unicode code points to preserve surrogate pairs
        int[] cps = input.codePoints().toArray();
        StringBuilder sb = new StringBuilder(cps.length);
        for (int i = cps.length - 1; i >= 0; i--) {
            sb.appendCodePoint(cps[i]);
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "hello",
            "",
            "A",
            "racecar",
            "Hello, 世界"
        };
        for (String t : tests) {
            String reversed = reverseString(t);
            System.out.println("Input: \"" + t + "\" -> Reversed: \"" + reversed + "\"");
        }
    }
}