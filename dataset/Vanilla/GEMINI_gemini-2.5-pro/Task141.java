public class Task141 {

    /**
     * Reverses a given string.
     * @param str The string to be reversed.
     * @return The reversed string.
     */
    public static String reverseString(String str) {
        if (str == null) {
            return null;
        }
        return new StringBuilder(str).reverse().toString();
    }

    public static void main(String[] args) {
        String[] testCases = {
            "hello",
            "world",
            "Java",
            "12345",
            ""
        };

        System.out.println("Java Test Cases:");
        for (String test : testCases) {
            String reversed = reverseString(test);
            System.out.println("Original: \"" + test + "\", Reversed: \"" + reversed + "\"");
        }
    }
}