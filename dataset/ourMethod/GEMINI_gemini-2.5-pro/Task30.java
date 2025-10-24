import java.util.Arrays;

public class Task30 {

    /**
     * Takes two strings s1 and s2 including only letters from a to z.
     * Returns a new sorted string, the longest possible, containing distinct letters.
     *
     * @param s1 The first string.
     * @param s2 The second string.
     * @return The resulting sorted string with distinct characters.
     */
    public static String longest(String s1, String s2) {
        if (s1 == null && s2 == null) {
            return "";
        }
        
        boolean[] seen = new boolean[26];
        
        if (s1 != null) {
            for (char c : s1.toCharArray()) {
                if (c >= 'a' && c <= 'z') {
                    seen[c - 'a'] = true;
                }
            }
        }

        if (s2 != null) {
            for (char c : s2.toCharArray()) {
                if (c >= 'a' && c <= 'z') {
                    seen[c - 'a'] = true;
                }
            }
        }

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
        String s1 = "xyaabbbccccdefww";
        String s2 = "xxxxyyyyabklmopq";
        System.out.println("Test Case 1:");
        System.out.println("s1: \"" + s1 + "\"");
        System.out.println("s2: \"" + s2 + "\"");
        System.out.println("Result: \"" + longest(s1, s2) + "\"");
        System.out.println("Expected: \"abcdefklmopqwxy\"");
        System.out.println();

        // Test Case 2
        s1 = "abcdefghijklmnopqrstuvwxyz";
        s2 = "abcdefghijklmnopqrstuvwxyz";
        System.out.println("Test Case 2:");
        System.out.println("s1: \"" + s1 + "\"");
        System.out.println("s2: \"" + s2 + "\"");
        System.out.println("Result: \"" + longest(s1, s2) + "\"");
        System.out.println("Expected: \"abcdefghijklmnopqrstuvwxyz\"");
        System.out.println();
        
        // Test Case 3
        s1 = "aretheyhere";
        s2 = "yestheyarehere";
        System.out.println("Test Case 3:");
        System.out.println("s1: \"" + s1 + "\"");
        System.out.println("s2: \"" + s2 + "\"");
        System.out.println("Result: \"" + longest(s1, s2) + "\"");
        System.out.println("Expected: \"aehrsty\"");
        System.out.println();

        // Test Case 4
        s1 = "loopingisfunbutdangerous";
        s2 = "lessdangerousthancoding";
        System.out.println("Test Case 4:");
        System.out.println("s1: \"" + s1 + "\"");
        System.out.println("s2: \"" + s2 + "\"");
        System.out.println("Result: \"" + longest(s1, s2) + "\"");
        System.out.println("Expected: \"abcdefghilnoprstu\"");
        System.out.println();
        
        // Test Case 5
        s1 = "inmanylanguages";
        s2 = "theresapairoffunctions";
        System.out.println("Test Case 5:");
        System.out.println("s1: \"" + s1 + "\"");
        System.out.println("s2: \"" + s2 + "\"");
        System.out.println("Result: \"" + longest(s1, s2) + "\"");
        System.out.println("Expected: \"acefghilmnoprstu\"");
        System.out.println();
    }
}