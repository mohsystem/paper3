public class Task141 {

    /**
     * Reverses a given string.
     * This method is secure as it handles null input and uses the built-in,
     * memory-safe StringBuilder class for the reversal operation.
     *
     * @param str The string to be reversed.
     * @return The reversed string. Returns null if the input is null.
     */
    public static String reverseString(String str) {
        if (str == null) {
            return null;
        }
        // Using StringBuilder is efficient and safe for string manipulation in Java.
        return new StringBuilder(str).reverse().toString();
    }

    public static void main(String[] args) {
        String[] testCases = {
            "hello",
            "Java",
            "12345",
            "",
            "a single char"
        };

        System.out.println("--- Java String Reversal ---");
        for (String test : testCases) {
            String reversed = reverseString(test);
            System.out.println("Original: \"" + test + "\", Reversed: \"" + reversed + "\"");
        }
    }
}