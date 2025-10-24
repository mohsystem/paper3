public class Task114 {

    /**
     * Creates a copy of the given string.
     * In Java, strings are immutable, so returning the original string or
     * creating a new String object are both safe ways to "copy".
     * @param source The string to copy.
     * @return A new string with the same content as the source.
     */
    public static String copyString(String source) {
        if (source == null) {
            return null;
        }
        return new String(source);
    }

    /**
     * Concatenates two strings.
     * @param s1 The first string.
     * @param s2 The second string.
     * @return A new string that is the result of s1 + s2.
     */
    public static String concatenateStrings(String s1, String s2) {
        if (s1 == null && s2 == null) {
            return "";
        }
        if (s1 == null) {
            return s2;
        }
        if (s2 == null) {
            return s1;
        }
        return s1.concat(s2);
    }

    /**
     * Gets the length of a string.
     * @param s The input string.
     * @return The length of the string.
     */
    public static int getStringLength(String s) {
        if (s == null) {
            return 0;
        }
        return s.length();
    }

    /**
     * Finds the first occurrence of a substring within a main string.
     * @param mainStr The string to search within.
     * @param subStr The substring to search for.
     * @return The starting index of the substring, or -1 if not found.
     */
    public static int findSubstring(String mainStr, String subStr) {
        if (mainStr == null || subStr == null) {
            return -1;
        }
        return mainStr.indexOf(subStr);
    }

    /**
     * Replaces all occurrences of a substring with a new substring.
     * @param mainStr The original string.
     * @param oldSub The substring to be replaced.
     * @param newSub The substring to replace with.
     * @return A new string with all replacements made.
     */
    public static String replaceSubstring(String mainStr, String oldSub, String newSub) {
        if (mainStr == null || oldSub == null || newSub == null) {
            return mainStr;
        }
        return mainStr.replace(oldSub, newSub);
    }

    public static void main(String[] args) {
        // Test Case 1: Copying a string
        String original1 = "Hello World";
        String copied1 = copyString(original1);
        System.out.println("Test Case 1: Copying a String");
        System.out.println("Original:  \"" + original1 + "\"");
        System.out.println("Copied:    \"" + copied1 + "\"\n");

        // Test Case 2: Concatenating strings
        String s1 = "Hello, ";
        String s2 = "World!";
        String concatenated = concatenateStrings(s1, s2);
        System.out.println("Test Case 2: Concatenating Strings");
        System.out.println("String 1: \"" + s1 + "\"");
        System.out.println("String 2: \"" + s2 + "\"");
        System.out.println("Concatenated: \"" + concatenated + "\"\n");

        // Test Case 3: Getting string length
        String s3 = "Programming";
        int length = getStringLength(s3);
        System.out.println("Test Case 3: Getting String Length");
        System.out.println("String: \"" + s3 + "\"");
        System.out.println("Length: " + length + "\n");

        // Test Case 4: Finding a substring
        String mainStr4 = "This is a test";
        String subStr4 = "is";
        int index = findSubstring(mainStr4, subStr4);
        System.out.println("Test Case 4: Finding a Substring");
        System.out.println("Main String: \"" + mainStr4 + "\"");
        System.out.println("Substring:   \"" + subStr4 + "\"");
        System.out.println("Found at index: " + index + "\n");

        // Test Case 5: Replacing a substring
        String mainStr5 = "The house is blue and the car is blue.";
        String oldSub5 = "blue";
        String newSub5 = "red";
        String replaced = replaceSubstring(mainStr5, oldSub5, newSub5);
        System.out.println("Test Case 5: Replacing a Substring");
        System.out.println("Original: \"" + mainStr5 + "\"");
        System.out.println("Replaced: \"" + replaced + "\"\n");
    }
}