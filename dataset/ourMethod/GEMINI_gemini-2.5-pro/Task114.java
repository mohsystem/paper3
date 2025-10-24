import java.nio.charset.StandardCharsets;

public class Task114 {

    /**
     * Creates a copy of a string.
     * In Java, strings are immutable, so returning the original string reference is safe.
     * This method creates an explicit new String object to demonstrate the "copy" concept.
     * @param source The string to copy.
     * @return A new String object with the same content as the source, or null if source is null.
     */
    public static String safeStringCopy(String source) {
        if (source == null) {
            return null;
        }
        // Creates a new String object, ensuring it's a distinct copy.
        return new String(source);
    }

    /**
     * Safely concatenates two strings.
     * Uses StringBuilder for efficient and safe concatenation, handling null inputs.
     * @param s1 The first string.
     * @param s2 The second string.
     * @return A new string that is the result of concatenating s1 and s2.
     */
    public static String safeStringConcat(String s1, String s2) {
        String str1 = (s1 == null) ? "" : s1;
        String str2 = (s2 == null) ? "" : s2;
        
        StringBuilder sb = new StringBuilder(str1.length() + str2.length());
        sb.append(str1);
        sb.append(str2);
        return sb.toString();
    }

    /**
     * Reverses a given string.
     * @param source The string to reverse.
     * @return A new string that is the reverse of the source, or null if source is null.
     */
    public static String reverseString(String source) {
        if (source == null) {
            return null;
        }
        return new StringBuilder(source).reverse().toString();
    }

    public static void main(String[] args) {
        System.out.println("--- Testing safeStringCopy ---");
        String[] copyTests = {"hello", "", "a long string example", "another one", null};
        for (String test : copyTests) {
            String copied = safeStringCopy(test);
            System.out.println("Original: " + (test == null ? "null" : "\"" + test + "\"") 
                             + ", Copied: " + (copied == null ? "null" : "\"" + copied + "\""));
        }

        System.out.println("\n--- Testing safeStringConcat ---");
        String[][] concatTests = {
            {"Hello, ", "World!"},
            {"", "Append this"},
            {"Start with this", ""},
            {"One", null},
            {null, "Two"}
        };
        for (String[] test : concatTests) {
            String s1 = test[0];
            String s2 = test[1];
            String result = safeStringConcat(s1, s2);
            System.out.println("s1: " + (s1 == null ? "null" : "\"" + s1 + "\"") 
                             + ", s2: " + (s2 == null ? "null" : "\"" + s2 + "\"") 
                             + ", Result: \"" + result + "\"");
        }

        System.out.println("\n--- Testing reverseString ---");
        String[] reverseTests = {"abcdef", "12345", "racecar", "a", ""};
        for (String test : reverseTests) {
            String reversed = reverseString(test);
            System.out.println("Original: \"" + test + "\", Reversed: \"" + reversed + "\"");
        }
    }
}