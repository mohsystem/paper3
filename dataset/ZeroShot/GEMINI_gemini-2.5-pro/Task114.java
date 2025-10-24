public class Task114 {

    /**
     * Creates a copy of a string. In Java, strings are immutable,
     * so returning the original reference is safe and efficient.
     * This method creates a new String object with the same content for clarity.
     * @param source The string to be copied.
     * @return A new string with the same value as the source.
     */
    public static String stringCopy(String source) {
        if (source == null) {
            return null;
        }
        return new String(source);
    }

    /**
     * Concatenates two strings.
     * @param s1 The first string.
     * @param s2 The second string.
     * @return The concatenated string.
     */
    public static String stringConcat(String s1, String s2) {
        if (s1 == null && s2 == null) {
            return null;
        }
        if (s1 == null) {
            return new String(s2);
        }
        if (s2 == null) {
            return new String(s1);
        }
        return s1 + s2;
    }

    /**
     * Reverses a string using a StringBuilder for efficiency and safety.
     * @param source The string to be reversed.
     * @return The reversed string.
     */
    public static String stringReverse(String source) {
        if (source == null) {
            return null;
        }
        return new StringBuilder(source).reverse().toString();
    }

    public static void main(String[] args) {
        // Test Case 1: Basic operations
        System.out.println("--- Test Case 1: Basic Operations ---");
        String str1 = "hello";
        String str2 = "world";

        String copiedStr = stringCopy(str1);
        System.out.println("Original: " + str1 + ", Copied: " + copiedStr);

        String concatenatedStr = stringConcat(str1, str2);
        System.out.println("Concatenated: \"" + str1 + "\" + \"" + str2 + "\" = \"" + concatenatedStr + "\"");

        String reversedStr = stringReverse("Java");
        System.out.println("Original: Java, Reversed: " + reversedStr);
        System.out.println();

        // Test Case 2: Empty strings
        System.out.println("--- Test Case 2: Empty Strings ---");
        String emptyStr = "";
        String textStr = "test";

        copiedStr = stringCopy(emptyStr);
        System.out.println("Original: \"\", Copied: \"" + copiedStr + "\"");

        concatenatedStr = stringConcat(emptyStr, textStr);
        System.out.println("Concatenated: \"\" + \"" + textStr + "\" = \"" + concatenatedStr + "\"");

        reversedStr = stringReverse(emptyStr);
        System.out.println("Original: \"\", Reversed: \"" + reversedStr + "\"");
        System.out.println();

        // Test Case 3: Null inputs
        System.out.println("--- Test Case 3: Null Inputs ---");
        copiedStr = stringCopy(null);
        System.out.println("Copying null: " + copiedStr);
        concatenatedStr = stringConcat(null, "safe");
        System.out.println("Concatenating null and \"safe\": " + concatenatedStr);
        reversedStr = stringReverse(null);
        System.out.println("Reversing null: " + reversedStr);
        System.out.println();
        
        // Test Case 4: Strings with spaces and special characters
        System.out.println("--- Test Case 4: Special Characters ---");
        String specialStr = " A B C!@#123 ";
        copiedStr = stringCopy(specialStr);
        System.out.println("Original: \"" + specialStr + "\", Copied: \"" + copiedStr + "\"");

        concatenatedStr = stringConcat(specialStr, "end");
        System.out.println("Concatenated: \"" + specialStr + "\" + \"end\" = \"" + concatenatedStr + "\"");

        reversedStr = stringReverse(specialStr);
        System.out.println("Original: \"" + specialStr + "\", Reversed: \"" + reversedStr + "\"");
        System.out.println();

        // Test Case 5: Single character string
        System.out.println("--- Test Case 5: Single Character String ---");
        String singleCharStr = "a";
        copiedStr = stringCopy(singleCharStr);
        System.out.println("Original: " + singleCharStr + ", Copied: " + copiedStr);

        concatenatedStr = stringConcat(singleCharStr, "b");
        System.out.println("Concatenated: \"" + singleCharStr + "\" + \"b\" = \"" + concatenatedStr + "\"");

        reversedStr = stringReverse(singleCharStr);
        System.out.println("Original: " + singleCharStr + ", Reversed: " + reversedStr);
    }
}