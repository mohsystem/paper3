public class Task113 {

    /**
     * Concatenates multiple strings together.
     *
     * @param strings A variable number of strings to concatenate. Null strings in the array are ignored.
     * @return The concatenated string. Returns an empty string if the input array is null or empty.
     */
    public static String concatenateStrings(String... strings) {
        if (strings == null) {
            return "";
        }
        StringBuilder sb = new StringBuilder();
        for (String s : strings) {
            if (s != null) {
                sb.append(s);
            }
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        // Test Case 1: Basic concatenation
        String[] test1 = {"Hello", ", ", "World", "!"};
        System.out.println("Test Case 1: " + concatenateStrings(test1));

        // Test Case 2: Array with null and empty strings
        String[] test2 = {"Java", " ", null, "is", "", " powerful."};
        System.out.println("Test Case 2: " + concatenateStrings(test2));

        // Test Case 3: Empty array
        String[] test3 = {};
        System.out.println("Test Case 3: " + concatenateStrings(test3));

        // Test Case 4: Null input array
        System.out.println("Test Case 4: " + concatenateStrings(null));

        // Test Case 5: Single string
        String[] test5 = {"SingleString"};
        System.out.println("Test Case 5: " + concatenateStrings(test5));
    }
}