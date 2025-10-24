public class Task141 {

    /**
     * Reverses a given string.
     *
     * @param str The string to be reversed.
     * @return The reversed string, or null if the input is null.
     */
    public static String reverseString(String str) {
        // Check for null input to avoid NullPointerException
        if (str == null) {
            return null;
        }
        // Use StringBuilder's reverse() method for efficiency and simplicity
        return new StringBuilder(str).reverse().toString();
    }

    public static void main(String[] args) {
        // 5 test cases
        String[] testCases = {
            "hello world",
            "12345",
            "a",
            "",
            "Palindrome"
        };

        for (String test : testCases) {
            String reversed = reverseString(test);
            System.out.println("Original: \"" + test + "\", Reversed: \"" + reversed + "\"");
        }
    }
}