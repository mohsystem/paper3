import java.util.Arrays;

public class Task168 {

    /**
     * Tweaks letters in a string based on an array of integers.
     *
     * @param str The input string (all lowercase letters).
     * @param arr An array of integers (0, 1, or -1) of the same length as str.
     * @return The modified string.
     */
    public static String tweakLetters(String str, int[] arr) {
        if (str == null || arr == null || str.length() != arr.length) {
            // As per prompt, we assume valid inputs.
            // In a real-world scenario, you might throw an exception.
            return "";
        }
        
        StringBuilder result = new StringBuilder(str.length());
        for (int i = 0; i < str.length(); i++) {
            char originalChar = str.charAt(i);
            int tweakValue = arr[i];
            char newChar = (char) (originalChar + tweakValue);
            result.append(newChar);
        }
        
        return result.toString();
    }

    public static void main(String[] args) {
        // Test Case 1
        int[] arr1 = {0, 1, -1, 0, -1};
        String str1 = "apple";
        System.out.println("tweakLetters(\"" + str1 + "\", " + Arrays.toString(arr1) + ") -> \"" + tweakLetters(str1, arr1) + "\"");

        // Test Case 2
        int[] arr2 = {0, 0, 0, -1};
        String str2 = "many";
        System.out.println("tweakLetters(\"" + str2 + "\", " + Arrays.toString(arr2) + ") -> \"" + tweakLetters(str2, arr2) + "\"");

        // Test Case 3
        int[] arr3 = {1, 1, 1, 1, 1};
        String str3 = "rhino";
        System.out.println("tweakLetters(\"" + str3 + "\", " + Arrays.toString(arr3) + ") -> \"" + tweakLetters(str3, arr3) + "\"");

        // Test Case 4
        int[] arr4 = {-1, 0, 1};
        String str4 = "abc";
        System.out.println("tweakLetters(\"" + str4 + "\", " + Arrays.toString(arr4) + ") -> \"" + tweakLetters(str4, arr4) + "\"");

        // Test Case 5
        int[] arr5 = {1, -1, 1, -1};
        String str5 = "zulu";
        System.out.println("tweakLetters(\"" + str5 + "\", " + Arrays.toString(arr5) + ") -> \"" + tweakLetters(str5, arr5) + "\"");
    }
}