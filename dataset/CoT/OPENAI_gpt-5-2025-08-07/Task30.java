import java.util.*;

public class Task30 {
    /*
     Chain-of-Through Process:
     1) Problem understanding: Merge letters from two strings, deduplicate, sort alphabetically, and return as a single string.
     2) Security requirements: Avoid assumptions about input; handle both uppercase and lowercase safely; do not use unsafe operations.
     3) Secure coding generation: Use bounds checks, avoid mutable shared state, and validate characters.
     4) Code review: Ensure indices are within 0..25, avoid null dereferences, use StringBuilder safely.
     5) Secure code output: Final function returns a new string with distinct sorted letters from 'a' to 'z'.
    */

    public static String longest(String s1, String s2) {
        boolean[] present = new boolean[26];
        // Aggregate characters from both strings
        addLetters(present, s1);
        addLetters(present, s2);

        StringBuilder sb = new StringBuilder(26);
        for (int i = 0; i < 26; i++) {
            if (present[i]) {
                sb.append((char) ('a' + i));
            }
        }
        return sb.toString();
    }

    private static void addLetters(boolean[] present, String s) {
        if (s == null) return; // Defensive: though problem states a-z letters, handle null safely
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            // Normalize to lowercase and include only [a-z]
            if (c >= 'A' && c <= 'Z') c = (char) (c - 'A' + 'a');
            if (c >= 'a' && c <= 'z') {
                int idx = c - 'a'; // 0..25
                present[idx] = true;
            }
        }
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        String[][] tests = new String[][]{
            {"xyaabbbccccdefww", "xxxxyyyyabklmopq"},
            {"abcdefghijklmnopqrstuvwxyz", "abcdefghijklmnopqrstuvwxyz"},
            {"abc", "def"},
            {"zzz", "zzzz"},
            {"", "abc"}
        };

        for (int i = 0; i < tests.length; i++) {
            String result = longest(tests[i][0], tests[i][1]);
            System.out.println("Test " + (i + 1) + ": " + result);
        }
    }
}