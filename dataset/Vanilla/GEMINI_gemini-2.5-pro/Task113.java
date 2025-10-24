public class Task113 {

    /**
     * Concatenates multiple strings into a single string.
     *
     * @param strings A variable number of strings to concatenate.
     * @return The single concatenated string.
     */
    public static String concatenateStrings(String... strings) {
        if (strings == null || strings.length == 0) {
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
        // Test Case 1
        String[] test1 = {"Hello", " ", "World", "!"};
        System.out.println("Test Case 1 Input: [\"Hello\", \" \", \"World\", \"!\"]");
        System.out.println("Output: " + concatenateStrings(test1));
        System.out.println();

        // Test Case 2
        String[] test2 = {"Java", ", ", "Python", ", ", "C++", ", ", "C"};
        System.out.println("Test Case 2 Input: [\"Java\", \", \", \"Python\", \", \", \"C++\", \", \", \"C\"]");
        System.out.println("Output: " + concatenateStrings(test2));
        System.out.println();

        // Test Case 3
        String[] test3 = {"One"};
        System.out.println("Test Case 3 Input: [\"One\"]");
        System.out.println("Output: " + concatenateStrings(test3));
        System.out.println();
        
        // Test Case 4: With empty strings
        String[] test4 = {"", "leading", " and ", "trailing", ""};
        System.out.println("Test Case 4 Input: [\"\", \"leading\", \" and \", \"trailing\", \"\"]");
        System.out.println("Output: " + concatenateStrings(test4));
        System.out.println();

        // Test Case 5: Empty input array
        String[] test5 = {};
        System.out.println("Test Case 5 Input: []");
        System.out.println("Output: " + concatenateStrings(test5));
        System.out.println();
    }
}