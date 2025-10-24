public class Task114 {

    /**
     * Creates a copy of a string.
     * @param s The source string.
     * @return A new string with the same content as s.
     */
    public static String copyString(String s) {
        return new String(s);
    }

    /**
     * Concatenates two strings.
     * @param s1 The first string.
     * @param s2 The second string.
     * @return The combined string.
     */
    public static String concatenateStrings(String s1, String s2) {
        return s1 + s2;
    }

    /**
     * Gets the length of a string.
     * @param s The input string.
     * @return The length of the string.
     */
    public static int getStringLength(String s) {
        return s.length();
    }

    /**
     * Finds the index of a substring within a main string.
     * @param mainStr The string to search in.
     * @param subStr The substring to search for.
     * @return The starting index of the substring, or -1 if not found.
     */
    public static int findSubstring(String mainStr, String subStr) {
        return mainStr.indexOf(subStr);
    }

    /**
     * Converts a string to uppercase.
     * @param s The input string.
     * @return The uppercase version of the string.
     */
    public static String toUpperCase(String s) {
        return s.toUpperCase();
    }
    
    /**
     * Converts a string to lowercase.
     * @param s The input string.
     * @return The lowercase version of the string.
     */
    public static String toLowerCase(String s) {
        return s.toLowerCase();
    }

    public static void main(String[] args) {
        String[][] testCases = {
            {"Hello", "World"},
            {"Java", "Programming"},
            {"Test", "Case"},
            {"one two three", "two"},
            {"UPPER", "lower"}
        };
        
        for (int i = 0; i < testCases.length; i++) {
            String s1 = testCases[i][0];
            String s2 = testCases[i][1];

            System.out.println("--- Test Case " + (i + 1) + " ---");
            System.out.println("Original strings: \"" + s1 + "\", \"" + s2 + "\"");

            // Copy
            String copied = copyString(s1);
            System.out.println("Copied s1: " + copied);

            // Concatenate
            String concatenated = concatenateStrings(s1, s2);
            System.out.println("Concatenated: " + concatenated);

            // Length
            System.out.println("Length of s1: " + getStringLength(s1));

            // Find Substring
            int index = findSubstring(s1, s2);
             if (s1.equals("one two three") && s2.equals("two")) { // special case for test 4
                index = findSubstring(s1, s2);
                System.out.println("Index of \"" + s2 + "\" in \"" + s1 + "\": " + index);
            } else {
                 System.out.println("Index of 'o' in \"" + s1 + "\": " + findSubstring(s1, "o"));
            }


            // To Uppercase
            System.out.println("s1 to uppercase: " + toUpperCase(s1));
            
            // To Lowercase
            System.out.println("s2 to lowercase: " + toLowerCase(s2));
            System.out.println();
        }
    }
}