import java.util.ArrayList;
import java.util.List;

public class Task59 {
    private static final int MAX_LEN = 10000;

    public static String accum(String s) {
        if (s == null) {
            throw new IllegalArgumentException("Input must be non-null and alphabetic [A-Za-z].");
        }
        final int n = s.length();
        if (n > MAX_LEN) {
            throw new IllegalArgumentException("Input too long.");
        }
        for (int i = 0; i < n; i++) {
            char ch = s.charAt(i);
            boolean isAsciiLetter = (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
            if (!isAsciiLetter) {
                throw new IllegalArgumentException("Input must contain only ASCII letters [A-Za-z].");
            }
        }
        if (n == 0) {
            return "";
        }
        long letters = ((long) n * (n + 1)) / 2;
        long separators = n > 1 ? (n - 1) : 0;
        int capacity = (int) (letters + separators);
        StringBuilder sb = new StringBuilder(capacity);
        for (int i = 0; i < n; i++) {
            char ch = s.charAt(i);
            char upper = Character.toUpperCase(ch);
            char lower = Character.toLowerCase(ch);
            sb.append(upper);
            for (int k = 0; k < i; k++) {
                sb.append(lower);
            }
            if (i + 1 < n) {
                sb.append('-');
            }
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        List<String> tests = new ArrayList<>();
        tests.add("abcd");
        tests.add("RqaEzty");
        tests.add("cwAt");
        tests.add("Z");
        tests.add("abc1"); // invalid

        for (String t : tests) {
            try {
                String out = accum(t);
                System.out.println(out);
            } catch (IllegalArgumentException ex) {
                System.out.println("ERROR: " + ex.getMessage());
            }
        }
    }
}