import java.util.Objects;

public class Task141 {

    /**
     * Reverses a given string.
     *
     * @param str The string to be reversed.
     * @return The reversed string, or null if the input is null.
     */
    public static String reverseString(String str) {
        if (str == null) {
            return null;
        }
        // Using StringBuilder is efficient for string manipulation in Java.
        // Its reverse() method correctly handles Unicode characters (surrogate pairs).
        return new StringBuilder(str).reverse().toString();
    }

    public static void main(String[] args) {
        String[] testCases = {
            "Hello, World!",
            "madam",
            "12345",
            "",
            "a"
        };

        for (String testCase : testCases) {
            System.out.println("Original: \"" + testCase + "\"");
            String reversed = reverseString(testCase);
            System.out.println("Reversed: \"" + reversed + "\"");
            System.out.println();
        }
    }
}