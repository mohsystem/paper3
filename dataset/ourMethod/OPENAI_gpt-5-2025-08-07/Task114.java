import java.util.Locale;

public class Task114 {
    private static final int MAX_LEN = 100_000;

    private static void validateString(String s) {
        if (s == null) {
            throw new IllegalArgumentException("Input string cannot be null.");
        }
        if (s.length() > MAX_LEN) {
            throw new IllegalArgumentException("Input string exceeds maximum allowed length.");
        }
    }

    public static String copyString(String s) {
        validateString(s);
        return new String(s);
    }

    public static String concatStrings(String a, String b) {
        validateString(a);
        validateString(b);
        long combined = (long) a.length() + (long) b.length();
        if (combined > MAX_LEN) {
            throw new IllegalArgumentException("Concatenated string would exceed maximum allowed length.");
        }
        return a + b;
    }

    public static String reverseString(String s) {
        validateString(s);
        return new StringBuilder(s).reverse().toString();
    }

    public static String toUpperCaseSafe(String s) {
        validateString(s);
        return s.toUpperCase(Locale.ROOT);
    }

    public static String safeSubstring(String s, int start, int length) {
        validateString(s);
        if (start < 0 || length < 0) {
            throw new IllegalArgumentException("Start and length must be non-negative.");
        }
        if ((long) start + (long) length > s.length()) {
            throw new IllegalArgumentException("Substring range is out of bounds.");
        }
        String sub = s.substring(start, start + length);
        if (sub.length() > MAX_LEN) {
            throw new IllegalArgumentException("Result exceeds maximum allowed length.");
        }
        return sub;
    }

    public static void main(String[] args) {
        try {
            // Test 1: Copy
            String t1 = copyString("Hello");
            System.out.println("Copy: " + t1);

            // Test 2: Concat
            String t2 = concatStrings("Hello", " World");
            System.out.println("Concat: " + t2);

            // Test 3: Reverse
            String t3 = reverseString("RaceCar");
            System.out.println("Reverse: " + t3);

            // Test 4: To Upper
            String t4 = toUpperCaseSafe("security123");
            System.out.println("Upper: " + t4);

            // Test 5: Safe Substring
            String t5 = safeSubstring("abcdef", 2, 3);
            System.out.println("Substring: " + t5);
        } catch (IllegalArgumentException ex) {
            System.out.println("Error: " + ex.getMessage());
        }
    }
}