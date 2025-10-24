public class Task113 {

    /**
     * Securely concatenates multiple strings using StringBuilder.
     * This avoids the inefficiency of creating multiple intermediate String objects
     * and is safe from buffer overflows as StringBuilder handles its own capacity.
     *
     * @param strings A variable number of strings to concatenate.
     * @return The concatenated string. Returns an empty string if the input is null.
     */
    public static String concatenateStrings(String... strings) {
        if (strings == null) {
            return "";
        }
        StringBuilder sb = new StringBuilder();
        for (String s : strings) {
            if (s != null) { // Safely skip null strings in the input array
                sb.append(s);
            }
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        // Test Case 1: Basic concatenation
        String[] input1 = {"Hello", ", ", "World", "!"};
        System.out.println("Test Case 1: " + concatenateStrings(input1));
        System.out.println("Expected: Hello, World!");
        System.out.println("--------------------");

        // Test Case 2: Concatenating with empty strings
        String[] input2 = {"One", "", "Two", "", "Three"};
        System.out.println("Test Case 2: " + concatenateStrings(input2));
        System.out.println("Expected: OneTwoThree");
        System.out.println("--------------------");

        // Test Case 3: Single string
        String[] input3 = {"OnlyOneString"};
        System.out.println("Test Case 3: " + concatenateStrings(input3));
        System.out.println("Expected: OnlyOneString");
        System.out.println("--------------------");

        // Test Case 4: No strings (empty array)
        String[] input4 = {};
        System.out.println("Test Case 4: " + concatenateStrings(input4));
        System.out.println("Expected: ");
        System.out.println("--------------------");

        // Test Case 5: With null values in the array
        String[] input5 = {"Java", " is ", null, "awesome", null, "."};
        System.out.println("Test Case 5: " + concatenateStrings(input5));
        System.out.println("Expected: Java is awesome.");
        System.out.println("--------------------");
    }
}