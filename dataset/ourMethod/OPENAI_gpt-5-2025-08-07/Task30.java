import java.util.Arrays;

public class Task30 {

    public static String longest(String s1, String s2) {
        if (s1 == null || s2 == null) {
            throw new IllegalArgumentException("Inputs must be non-null and contain only lowercase a-z letters.");
        }
        final boolean[] seen = new boolean[26];

        for (int i = 0; i < s1.length(); i++) {
            char c = s1.charAt(i);
            if (c < 'a' || c > 'z') {
                throw new IllegalArgumentException("Inputs must contain only lowercase a-z letters.");
            }
            seen[c - 'a'] = true;
        }

        for (int i = 0; i < s2.length(); i++) {
            char c = s2.charAt(i);
            if (c < 'a' || c > 'z') {
                throw new IllegalArgumentException("Inputs must contain only lowercase a-z letters.");
            }
            seen[c - 'a'] = true;
        }

        StringBuilder sb = new StringBuilder(26);
        for (int i = 0; i < 26; i++) {
            if (seen[i]) {
                sb.append((char) ('a' + i));
            }
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        String[][] tests = new String[][] {
            {"xyaabbbccccdefww", "xxxxyyyyabklmopq"},
            {"abcdefghijklmnopqrstuvwxyz", "abcdefghijklmnopqrstuvwxyz"},
            {"abc", "def"},
            {"", "abcxyz"},
            {"Abc", "def"} // invalid test case
        };

        for (int i = 0; i < tests.length; i++) {
            String s1 = tests[i][0];
            String s2 = tests[i][1];
            try {
                String result = longest(s1, s2);
                System.out.println("Test " + (i + 1) + ": longest(\"" + s1 + "\", \"" + s2 + "\") -> \"" + result + "\"");
            } catch (IllegalArgumentException ex) {
                System.out.println("Test " + (i + 1) + ": longest(\"" + s1 + "\", \"" + s2 + "\") -> Error: " + ex.getMessage());
            }
        }
    }
}