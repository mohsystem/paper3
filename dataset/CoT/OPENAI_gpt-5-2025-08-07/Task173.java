import java.util.*;

public class Task173 {
    // Returns any duplicated substring with maximum length, or empty string if none.
    public static String longestDupSubstring(String s) {
        int n = s.length();
        if (n < 2) return "";
        final long MOD1 = 1_000_000_007L;
        final long MOD2 = 1_000_000_009L;
        final long BASE = 911382323L; // fixed base < both mods

        // Precompute powers and prefix hashes
        long[] pow1 = new long[n + 1];
        long[] pow2 = new long[n + 1];
        long[] pre1 = new long[n + 1];
        long[] pre2 = new long[n + 1];
        pow1[0] = 1; pow2[0] = 1;
        for (int i = 0; i < n; i++) {
            int v = (s.charAt(i) - 'a' + 1);
            pow1[i + 1] = (pow1[i] * BASE) % MOD1;
            pow2[i + 1] = (pow2[i] * BASE) % MOD2;
            pre1[i + 1] = (pre1[i] * BASE + v) % MOD1;
            pre2[i + 1] = (pre2[i] * BASE + v) % MOD2;
        }

        // Returns combined key for substring hash [l, r)
        final java.util.function.BiFunction<Integer, Integer, Long> getKey = (l, len) -> {
            int r = l + len;
            long h1 = (pre1[r] - (pre1[l] * pow1[len]) % MOD1);
            if (h1 < 0) h1 += MOD1;
            long h2 = (pre2[r] - (pre2[l] * pow2[len]) % MOD2);
            if (h2 < 0) h2 += MOD2;
            return (h1 << 32) ^ h2;
        };

        // Check if any duplicate of length L exists; return start index or -1
        java.util.function.IntUnaryOperator check = (L) -> {
            if (L == 0) return 0;
            HashMap<Long, ArrayList<Integer>> map = new HashMap<>(Math.max(16, n * 2));
            for (int i = 0; i + L <= n; i++) {
                long key = getKey.apply(i, L);
                ArrayList<Integer> list = map.get(key);
                if (list == null) {
                    list = new ArrayList<>(1);
                    list.add(i);
                    map.put(key, list);
                } else {
                    for (int idx : list) {
                        if (s.regionMatches(i, s, idx, L)) {
                            return i; // found duplicate
                        }
                    }
                    list.add(i);
                }
            }
            return -1;
        };

        int lo = 1, hi = n - 1, bestLen = 0, bestStart = -1;
        while (lo <= hi) {
            int mid = lo + ((hi - lo) >>> 1);
            int pos = check.applyAsInt(mid);
            if (pos != -1) {
                bestLen = mid;
                bestStart = pos;
                lo = mid + 1;
            } else {
                hi = mid - 1;
            }
        }
        return bestLen > 0 ? s.substring(bestStart, bestStart + bestLen) : "";
    }

    public static void main(String[] args) {
        String[] tests = {
            "banana",
            "abcd",
            "aaaaa",
            "abcdabc",
            "mississippi"
        };
        for (String t : tests) {
            String res = longestDupSubstring(t);
            System.out.println("Input: " + t + " -> Output: " + res);
        }
    }
}