import java.util.Arrays;

public class Task113 {

    /**
     * Concatenates multiple strings together using a StringBuilder for efficiency.
     *
     * @param strings A variable number of strings to concatenate.
     * @return A single string that is the result of concatenation. Returns an empty
     *         string if the input is null or empty.
     */
    public static String concatenateStrings(String... strings) {
        if (strings == null || strings.length == 0) {
            return "";
        }
        StringBuilder sb = new StringBuilder();
        for (String str : strings) {
            if (str != null) { // Handle potential null strings in the input array
                sb.append(str);
            }
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        // Test Case 1: Basic concatenation
        String[] test1 = {"Hello", " ", "World", "!"};
        System.out.println("Test Case 1: " + Arrays.toString(test1));
        System.out.println("Result: " + concatenateStrings(test1));
        System.out.println();

        // Test Case 2: Another basic case
        String[] test2 = {"Java", " ", "is", " ", "fun."};
        System.out.println("Test Case 2: " + Arrays.toString(test2));
        System.out.println("Result: " + concatenateStrings(test2));
        System.out.println();

        // Test Case 3: Empty input array
        String[] test3 = {};
        System.out.println("Test Case 3: " + Arrays.toString(test3));
        System.out.println("Result: " + concatenateStrings(test3));
        System.out.println();

        // Test Case 4: Array with a single element
        String[] test4 = {"SingleString"};
        System.out.println("Test Case 4: " + Arrays.toString(test4));
        System.out.println("Result: " + concatenateStrings(test4));
        System.out.println();

        // Test Case 5: Array with empty and null strings
        String[] test5 = {"part1", "", "part2", null, "part3"};
        System.out.println("Test Case 5: " + Arrays.toString(test5));
        System.out.println("Result: " + concatenateStrings(test5));
        System.out.println();
    }
}