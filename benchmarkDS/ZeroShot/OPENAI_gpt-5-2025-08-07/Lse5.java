import java.util.Arrays;

public class Lse5 {

    // Returns the element at the provided indexStr or null on invalid input
    public static String getElementAtIndex(String[] elements, String indexStr) {
        if (elements == null || indexStr == null) {
            return null;
        }
        Integer idx = tryParseNonNegativeInt(indexStr);
        if (idx == null) {
            return null;
        }
        if (idx >= elements.length) {
            return null;
        }
        return elements[idx];
    }

    // Safely parse a non-negative integer from a string
    private static Integer tryParseNonNegativeInt(String s) {
        String trimmed = s.trim();
        if (trimmed.isEmpty()) return null;

        // Validate numeric format
        int start = 0;
        if (trimmed.charAt(0) == '+') {
            start = 1;
            if (trimmed.length() == 1) return null;
        } else if (trimmed.charAt(0) == '-') {
            return null; // negative not allowed
        }
        for (int i = start; i < trimmed.length(); i++) {
            char c = trimmed.charAt(i);
            if (c < '0' || c > '9') {
                return null;
            }
        }
        try {
            // Prevent leading zeros issues are fine; parseInt checks range
            int val = Integer.parseInt(trimmed);
            return val < 0 ? null : val;
        } catch (NumberFormatException ex) {
            return null;
        }
    }

    public static void main(String[] args) {
        String[] elements = {"alpha", "bravo", "charlie", "delta", "echo"};

        // Process command-line argument if provided
        if (args != null && args.length >= 1) {
            String result = getElementAtIndex(elements, args[0]);
            if (result != null) {
                System.out.println(result);
            } else {
                System.err.println("Invalid input or index out of bounds.");
            }
        }

        // Five test cases
        String[] testIndices = {"0", "2", "4", "-1", "abc"};
        for (String t : testIndices) {
            String res = getElementAtIndex(elements, t);
            if (res != null) {
                System.out.println("Test index \"" + t + "\": " + res);
            } else {
                System.out.println("Test index \"" + t + "\": Invalid");
            }
        }
    }
}