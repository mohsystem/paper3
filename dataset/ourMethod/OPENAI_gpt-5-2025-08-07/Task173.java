import java.util.*;

public class Task173 {
    // Returns any longest duplicated substring (overlapping allowed). If none, returns "".
    public static String longestDupSubstring(String s) {
        if (s == null) return "";
        int n = s.length();
        if (n < 2 || n > 30000) return "";
        for (int i = 0; i < n; i++) {
            char c = s.charAt(i);
            if (c < 'a' || c > 'z') return "";
        }

        final long mod1 = 1_000_000_007L;
        final long mod2 = 1_000_000_009L;
        final long base1 = 911_382_323L;
        final long base2 = 972_663_749L;

        long[] pow1 = new long[n + 1];
        long[] pow2 = new long[n + 1];
        long[] pre1 = new long[n + 1];
        long[] pre2 = new long[n + 1];
        pow1[0] = 1; pow2[0] = 1;
        for (int i = 0; i < n; i++) {
            int v = (s.charAt(i) - 'a' + 1);
            pre1[i + 1] = (pre1[i] * base1 + v) % mod1;
            pre2[i + 1] = (pre2[i] * base2 + v) % mod2;
            pow1[i + 1] = (pow1[i] * base1) % mod1;
            pow2[i + 1] = (pow2[i] * base2) % mod2;
        }

        int lo = 1, hi = n - 1;
        int bestStart = -1, bestLen = 0;
        while (lo <= hi) {
            int mid = lo + (hi - lo) / 2;
            int idx = check(s, mid, pre1, pre2, pow1, pow2, mod1, mod2);
            if (idx != -1) {
                bestStart = idx;
                bestLen = mid;
                lo = mid + 1;
            } else {
                hi = mid - 1;
            }
        }
        if (bestStart == -1) return "";
        return s.substring(bestStart, bestStart + bestLen);
    }

    private static int check(String s, int L, long[] pre1, long[] pre2, long[] pow1, long[] pow2, long mod1, long mod2) {
        int n = s.length();
        if (L <= 0) return -1;
        Map<Long, ArrayList<Integer>> map = new HashMap<>(Math.max(16, n / 2));
        for (int i = 0; i + L <= n; i++) {
            long h1 = (pre1[i + L] - (pre1[i] * pow1[L]) % mod1 + mod1) % mod1;
            long h2 = (pre2[i + L] - (pre2[i] * pow2[L]) % mod2 + mod2) % mod2;
            long key = (h1 << 32) ^ h2;
            ArrayList<Integer> list = map.get(key);
            if (list == null) {
                list = new ArrayList<>(1);
                list.add(i);
                map.put(key, list);
            } else {
                // Verify actual equality to avoid rare hash collisions
                for (int prev : list) {
                    if (equalsRange(s, prev, i, L)) {
                        return i;
                    }
                }
                list.add(i);
            }
        }
        return -1;
    }

    private static boolean equalsRange(String s, int i, int j, int L) {
        for (int k = 0; k < L; k++) {
            if (s.charAt(i + k) != s.charAt(j + k)) return false;
        }
        return true;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "banana",
            "abcd",
            "aaaaa",
            "abcabcabc",
            "mississippi"
        };
        for (String t : tests) {
            String res = longestDupSubstring(t);
            System.out.println("Input: " + t + " -> Output: " + res);
        }
    }
}