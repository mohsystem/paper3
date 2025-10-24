public class Task30 {

    /**
     * Takes two strings s1 and s2 including only letters from a to z.
     * Returns a new sorted string, the longest possible, containing distinct letters.
     *
     * @param s1 The first string.
     * @param s2 The second string.
     * @return The longest sorted string with distinct letters from s1 and s2.
     */
    public static String longest(String s1, String s2) {
        boolean[] present = new boolean[26];

        // Mark characters from s1 as present
        for (char c : s1.toCharArray()) {
            if (c >= 'a' && c <= 'z') {
                present[c - 'a'] = true;
            }
        }

        // Mark characters from s2 as present
        for (char c : s2.toCharArray()) {
            if (c >= 'a' && c <= 'z') {
                present[c - 'a'] = true;
            }
        }

        StringBuilder result = new StringBuilder();
        // Build the result string from the present characters
        for (int i = 0; i < 26; i++) {
            if (present[i]) {
                result.append((char) ('a' + i));
            }
        }

        return result.toString();
    }

    public static void main(String[] args) {
        System.out.println("Java Test Cases:");

        // Test Case 1
        String s1_1 = "xyaabbbccccdefww";
        String s2_1 = "xxxxyyyyabklmopq";
        System.out.println("Test 1: " + longest(s1_1, s2_1)); // Expected: "abcdefklmopqwxy"

        // Test Case 2
        String s1_2 = "abcdefghijklmnopqrstuvwxyz";
        String s2_2 = "abcdefghijklmnopqrstuvwxyz";
        System.out.println("Test 2: " + longest(s1_2, s2_2)); // Expected: "abcdefghijklmnopqrstuvwxyz"

        // Test Case 3
        String s1_3 = "aretheyhere";
        String s2_3 = "yestheyarehere";
        System.out.println("Test 3: " + longest(s1_3, s2_3)); // Expected: "aehrsty"

        // Test Case 4
        String s1_4 = "loopingisfunbutdangerous";
        String s2_4 = "lessdangerousthancoding";
        System.out.println("Test 4: " + longest(s1_4, s2_4)); // Expected: "abcdefghilnoprstu"

        // Test Case 5
        String s1_5 = "inmanylanguages";
        String s2_5 = "theresapairoffunctions";
        System.out.println("Test 5: " + longest(s1_5, s2_5)); // Expected: "acefghilmnoprstu"
    }
}