import java.util.HashSet;
import java.util.Set;

public class Task173 {

    private int search(String s, int L) {
        int n = s.length();
        if (L == 0) return 0;
        if (L >= n) return -1;

        long base1 = 26;
        long base2 = 31;
        long mod1 = 1_000_000_007;
        long mod2 = 1_000_000_009;

        long h1 = 1;
        long h2 = 1;
        for (int i = 0; i < L - 1; i++) {
            h1 = (h1 * base1) % mod1;
            h2 = (h2 * base2) % mod2;
        }

        long currentHash1 = 0;
        long currentHash2 = 0;
        for (int i = 0; i < L; i++) {
            currentHash1 = (currentHash1 * base1 + (s.charAt(i) - 'a')) % mod1;
            currentHash2 = (currentHash2 * base2 + (s.charAt(i) - 'a')) % mod2;
        }

        Set<Long> seen = new HashSet<>();
        seen.add(currentHash1 * mod2 + currentHash2);

        for (int i = 1; i <= n - L; i++) {
            long prevCharVal = s.charAt(i - 1) - 'a';
            long nextCharVal = s.charAt(i + L - 1) - 'a';

            currentHash1 = (currentHash1 - (prevCharVal * h1) % mod1 + mod1) % mod1;
            currentHash1 = (currentHash1 * base1) % mod1;
            currentHash1 = (currentHash1 + nextCharVal) % mod1;
            
            currentHash2 = (currentHash2 - (prevCharVal * h2) % mod2 + mod2) % mod2;
            currentHash2 = (currentHash2 * base2) % mod2;
            currentHash2 = (currentHash2 + nextCharVal) % mod2;

            long combinedHash = currentHash1 * mod2 + currentHash2;
            if (seen.contains(combinedHash)) {
                return i;
            }
            seen.add(combinedHash);
        }
        return -1;
    }

    public String longestDupSubstring(String s) {
        int n = s.length();
        int low = 1, high = n;
        int bestLen = 0;
        int start = -1;

        while (low <= high) {
            int mid = low + (high - low) / 2;
            if (mid == 0) {
                low = mid + 1;
                continue;
            }
            int foundStart = search(s, mid);
            
            if (foundStart != -1) {
                bestLen = mid;
                start = foundStart;
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }

        if (start != -1) {
            return s.substring(start, start + bestLen);
        }
        return "";
    }

    public static void main(String[] args) {
        Task173 solution = new Task173();
        String[] testCases = {
            "banana",
            "abcd",
            "ababa",
            "mississippi",
            "aaaaaaaaaa"
        };
        String[] expectedResults = {
            "ana",
            "",
            "aba",
            "issi",
            "aaaaaaaaa"
        };

        for (int i = 0; i < testCases.length; i++) {
            String s = testCases[i];
            String result = solution.longestDupSubstring(s);
            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("Input: s = \"" + s + "\"");
            System.out.println("Output: \"" + result + "\"");
            System.out.println("Expected: \"" + expectedResults[i] + "\"");
            System.out.println();
        }
    }
}