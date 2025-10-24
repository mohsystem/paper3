import java.util.Arrays;

public class Task168 {
    public static String tweakLetters(String s, int[] shifts) {
        if (s == null || shifts == null) {
            throw new IllegalArgumentException("Inputs must not be null.");
        }
        if (s.length() != shifts.length) {
            throw new IllegalArgumentException("String length and shifts length must match.");
        }
        StringBuilder sb = new StringBuilder(s.length());
        for (int i = 0; i < s.length(); i++) {
            char ch = s.charAt(i);
            if (ch < 'a' || ch > 'z') {
                throw new IllegalArgumentException("Only lowercase a-z are supported.");
            }
            int delta = ((shifts[i] % 26) + 26) % 26;
            int pos = ch - 'a';
            char out = (char) ('a' + (pos + delta) % 26);
            sb.append(out);
        }
        return sb.toString();
    }

    private static void runTest(String input, int[] shifts, String expected) {
        try {
            String result = tweakLetters(input, shifts);
            System.out.println("tweakLetters(\"" + input + "\", " + Arrays.toString(shifts) + ") => \"" + result + "\""
                    + (expected != null ? " | expected: \"" + expected + "\"" : ""));
        } catch (IllegalArgumentException ex) {
            System.out.println("Error: " + ex.getMessage());
        }
    }

    public static void main(String[] args) {
        runTest("apple", new int[]{0, 1, -1, 0, -1}, "aqold");
        runTest("many", new int[]{0, 0, 0, -1}, "manx");
        runTest("rhino", new int[]{1, 1, 1, 1, 1}, "sijop");
        runTest("zzz", new int[]{1, 1, 1}, "aaa");
        runTest("abc", new int[]{-1, 0, 1}, "zbd");
    }
}