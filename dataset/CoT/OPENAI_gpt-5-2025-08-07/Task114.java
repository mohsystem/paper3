// Chain-of-Through Secure Implementation
// 1) Problem understanding: implement safe string operations (copy, concatenate, manipulate).
// 2) Security requirements: handle nulls, bounds, and avoid overflows; no unsafe APIs.
// 3) Secure coding generation: validations, safe computations.
// 4) Code review: ensure no unsafe calls, null/overflow checks.
// 5) Secure code output: hardened functions with tests.

public class Task114 {
    // Utility: normalize null to empty string
    private static String nz(String s) {
        return s == null ? "" : s;
    }

    // Copy string safely
    public static String copyString(String s) {
        String v = nz(s);
        return new String(v);
    }

    // Concatenate two strings safely
    public static String concatStrings(String a, String b) {
        return nz(a) + nz(b);
    }

    // Reverse a string
    public static String reverseString(String s) {
        return new StringBuilder(nz(s)).reverse().toString();
    }

    // Replace all occurrences of target with replacement (treat nulls as empty)
    public static String replaceSubstring(String s, String target, String replacement) {
        String src = nz(s);
        String t = target == null ? "" : target;
        String r = replacement == null ? "" : replacement;
        if (t.isEmpty()) {
            return copyString(src);
        }
        return src.replace(t, r);
    }

    // Safe substring with bounds checks
    public static String safeSubstring(String s, int start, int length) {
        String src = nz(s);
        if (start < 0) start = 0;
        if (length < 0) length = 0;
        int n = src.length();
        if (start >= n || length == 0) return "";
        long endLong = (long) start + (long) length;
        int end = (int) Math.min(n, Math.min(Integer.MAX_VALUE, endLong));
        if (end < start) return ""; // overflow guard
        return src.substring(start, end);
    }

    // Count non-overlapping occurrences
    public static int countOccurrences(String s, String sub) {
        String src = nz(s);
        String needle = sub == null ? "" : sub;
        if (needle.isEmpty()) return 0;
        int count = 0, idx = 0;
        while (true) {
            idx = src.indexOf(needle, idx);
            if (idx < 0) break;
            count++;
            idx += needle.length();
        }
        return count;
    }

    // Repeat string with overflow check
    public static String repeatString(String s, int times) {
        String src = nz(s);
        if (times <= 0 || src.isEmpty()) return "";
        long total = (long) src.length() * (long) times;
        if (total > Integer.MAX_VALUE) {
            throw new IllegalArgumentException("Result too large");
        }
        StringBuilder sb = new StringBuilder((int) total);
        for (int i = 0; i < times; i++) sb.append(src);
        return sb.toString();
    }

    public static String toUpper(String s) {
        return nz(s).toUpperCase();
    }

    public static String toLower(String s) {
        return nz(s).toLowerCase();
    }

    public static String trimSafe(String s) {
        return nz(s).trim();
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        String[][] tests = new String[][]{
            {"Hello", "World", "l", "L"},
            {"", "Test", "Test", "Exam"},
            {"  Spaces  ", "", " ", ""},
            {"abcabcabc", "xyz", "abc", "A"},
            {null, null, null, null}
        };

        for (int i = 0; i < tests.length; i++) {
            String a = tests[i][0];
            String b = tests[i][1];
            String tgt = tests[i][2];
            String rep = tests[i][3];

            System.out.println("=== Test " + (i + 1) + " ===");
            System.out.println("copyString(a): " + copyString(a));
            System.out.println("concatStrings(a,b): " + concatStrings(a, b));
            System.out.println("reverseString(a): " + reverseString(a));
            System.out.println("replaceSubstring(a,tgt,rep): " + replaceSubstring(a, tgt, rep));
            System.out.println("safeSubstring(a,1,3): " + safeSubstring(a, 1, 3));
            System.out.println("countOccurrences(a,tgt): " + countOccurrences(a, tgt));
            System.out.println("repeatString(\"ab\",3): " + repeatString("ab", 3));
            System.out.println("toUpper(a): " + toUpper(a));
            System.out.println("toLower(a): " + toLower(a));
            System.out.println("trimSafe(a): " + trimSafe(a));
        }
    }
}