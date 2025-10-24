// Chain-of-Through Step 1: Problem understanding
// Implement accum(String) -> "A-Bb-Ccc-..." per examples; input letters a..z, A..Z

// Chain-of-Through Step 2: Security requirements
// - No external inputs; handle empty strings safely.
// - Avoid unnecessary allocations; use StringBuilder.

// Chain-of-Through Step 3: Secure coding generation
// - Use Character.toUpperCase/LowerCase safely.
// - Bounds-checked loops.

// Chain-of-Through Step 4: Code review
// - Loops are index-bounded; no risky operations.

// Chain-of-Through Step 5: Secure code output
public class Task59 {
    public static String accum(String s) {
        if (s == null) return null;
        int n = s.length();
        if (n == 0) return "";
        // Estimate capacity: letters n + n*(n-1)/2, plus hyphens (n-1)
        long letters = (long)n + ((long)n * (n - 1L)) / 2L;
        long hyphens = n - 1L;
        long total = letters + hyphens;
        int cap = (total > Integer.MAX_VALUE) ? n : (int)total;
        StringBuilder sb = new StringBuilder(Math.max(cap, 16));
        for (int i = 0; i < n; i++) {
            char c = s.charAt(i);
            if (i > 0) sb.append('-');
            sb.append(Character.toUpperCase(c));
            char lower = Character.toLowerCase(c);
            for (int j = 0; j < i; j++) {
                sb.append(lower);
            }
        }
        return sb.toString();
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        String[] tests = {
            "abcd",
            "RqaEzty",
            "cwAt",
            "",
            "ZpglnRxqenU"
        };
        for (String t : tests) {
            System.out.println("accum(\"" + t + "\") -> \"" + accum(t) + "\"");
        }
    }
}