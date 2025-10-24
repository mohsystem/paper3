import java.util.*;

public class Task114 {
    public static String copyString(String s) {
        return s == null ? null : new String(s);
    }

    public static String concatenate(String a, String b) {
        if (a == null && b == null) return null;
        if (a == null) return new String(b);
        if (b == null) return new String(a);
        return a + b;
    }

    public static String reverseString(String s) {
        if (s == null) return null;
        return new StringBuilder(s).reverse().toString();
    }

    public static String toUpperString(String s) {
        if (s == null) return null;
        return s.toUpperCase();
    }

    public static String removeWhitespace(String s) {
        if (s == null) return null;
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (!Character.isWhitespace(c)) sb.append(c);
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        String t1 = "Hello";
        String t2a = "Hello";
        String t2b = " World";
        String t3 = "OpenAI";
        String t4 = "Mixed Case 123";
        String t5 = "  a b\tc\nd  ";

        System.out.println("Test 1 - copyString: " + copyString(t1));
        System.out.println("Test 2 - concatenate: " + concatenate(t2a, t2b));
        System.out.println("Test 3 - reverseString: " + reverseString(t3));
        System.out.println("Test 4 - toUpperString: " + toUpperString(t4));
        System.out.println("Test 5 - removeWhitespace: " + removeWhitespace(t5));
    }
}