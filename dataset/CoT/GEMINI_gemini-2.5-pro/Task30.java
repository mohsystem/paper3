public class Task30 {

    /**
     * Takes 2 strings s1 and s2 including only letters from a to z.
     * Returns a new sorted string, the longest possible, containing distinct letters.
     *
     * @param s1 The first string.
     * @param s2 The second string.
     * @return The longest sorted string with distinct letters from s1 and s2.
     */
    public static String longest(String s1, String s2) {
        boolean[] seen = new boolean[26]; // for 'a' through 'z'

        // Mark characters from s1 as seen
        for (char c : s1.toCharArray()) {
            if (c >= 'a' && c <= 'z') {
                seen[c - 'a'] = true;
            }
        }

        // Mark characters from s2 as seen
        for (char c : s2.toCharArray()) {
            if (c >= 'a' && c <= 'z') {
                seen[c - 'a'] = true;
            }
        }

        // Build the result string from the seen characters
        StringBuilder result = new StringBuilder();
        for (int i = 0; i < 26; i++) {
            if (seen[i]) {
                result.append((char) ('a' + i));
            }
        }

        return result.toString();
    }

    public static void main(String[] args) {
        // Test Case 1
        String a1 = "xyaabbbccccdefww";
        String b1 = "xxxxyyyyabklmopq";
        System.out.println("Test Case 1:");
        System.out.println("s1: \"" + a1 + "\"");
        System.out.println("s2: \"" + b1 + "\"");
        System.out.println("Result: \"" + longest(a1, b1) + "\""); // Expected: "abcdefklmopqwxy"
        System.out.println();

        // Test Case 2
        String a2 = "abcdefghijklmnopqrstuvwxyz";
        String b2 = "abcdefghijklmnopqrstuvwxyz";
        System.out.println("Test Case 2:");
        System.out.println("s1: \"" + a2 + "\"");
        System.out.println("s2: \"" + b2 + "\"");
        System.out.println("Result: \"" + longest(a2, b2) + "\""); // Expected: "abcdefghijklmnopqrstuvwxyz"
        System.out.println();
        
        // Test Case 3
        String a3 = "aretheyhere";
        String b3 = "yestheyarehere";
        System.out.println("Test Case 3:");
        System.out.println("s1: \"" + a3 + "\"");
        System.out.println("s2: \"" + b3 + "\"");
        System.out.println("Result: \"" + longest(a3, b3) + "\""); // Expected: "aehrsty"
        System.out.println();
        
        // Test Case 4
        String a4 = "loopingisfunbutdangerous";
        String b4 = "lessdangerousthancoding";
        System.out.println("Test Case 4:");
        System.out.println("s1: \"" + a4 + "\"");
        System.out.println("s2: \"" + b4 + "\"");
        System.out.println("Result: \"" + longest(a4, b4) + "\""); // Expected: "abcdefghilnoprstu"
        System.out.println();

        // Test Case 5
        String a5 = "";
        String b5 = "";
        System.out.println("Test Case 5:");
        System.out.println("s1: \"" + a5 + "\"");
        System.out.println("s2: \"" + b5 + "\"");
        System.out.println("Result: \"" + longest(a5, b5) + "\""); // Expected: ""
        System.out.println();
    }
}