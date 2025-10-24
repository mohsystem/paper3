import java.util.*;

public class Task173 {
    private static final long MOD1 = 1_000_000_007L;
    private static final long MOD2 = 1_000_000_009L;
    private static final long BASE1 = 911382323L % MOD1;
    private static final long BASE2 = 972663749L % MOD2;

    public static String longestDupSubstring(String s) {
        int n = s.length();
        if (n <= 1) return "";
        int[] vals = new int[n];
        for (int i = 0; i < n; i++) vals[i] = s.charAt(i) - 'a' + 1;

        long[] pow1 = new long[n + 1];
        long[] pow2 = new long[n + 1];
        long[] pref1 = new long[n + 1];
        long[] pref2 = new long[n + 1];
        pow1[0] = 1; pow2[0] = 1;
        for (int i = 0; i < n; i++) {
            pow1[i + 1] = (pow1[i] * BASE1) % MOD1;
            pow2[i + 1] = (pow2[i] * BASE2) % MOD2;
            pref1[i + 1] = (pref1[i] * BASE1 + vals[i]) % MOD1;
            pref2[i + 1] = (pref2[i] * BASE2 + vals[i]) % MOD2;
        }

        int lo = 1, hi = n - 1;
        int bestLen = 0, bestIdx = -1;
        while (lo <= hi) {
            int mid = (lo + hi) >>> 1;
            int idx = check(mid, pref1, pref2, pow1, pow2, n);
            if (idx != -1) {
                bestLen = mid;
                bestIdx = idx;
                lo = mid + 1;
            } else {
                hi = mid - 1;
            }
        }
        return bestIdx == -1 ? "" : s.substring(bestIdx, bestIdx + bestLen);
    }

    private static long getHash(long[] pref, long[] pow, long mod, int i, int L) {
        long res = (pref[i + L] - (pref[i] * pow[L]) % mod + mod) % mod;
        return res;
    }

    private static int check(int L, long[] pref1, long[] pref2, long[] pow1, long[] pow2, int n) {
        if (L == 0) return 0;
        HashMap<Long, Integer> seen = new HashMap<>(n);
        for (int i = 0; i + L <= n; i++) {
            long h1 = getHash(pref1, pow1, MOD1, i, L);
            long h2 = getHash(pref2, pow2, MOD2, i, L);
            long key = (h1 << 32) ^ h2;
            Integer prev = seen.putIfAbsent(key, i);
            if (prev != null) {
                return i;
            }
        }
        return -1;
    }

    private static void runTest(String s) {
        String res = longestDupSubstring(s);
        System.out.println("Input: " + s);
        System.out.println("Output: " + res);
        System.out.println();
    }

    public static void main(String[] args) {
        runTest("banana");       // Expected: "ana"
        runTest("abcd");         // Expected: ""
        runTest("aaaaa");        // Expected: "aaaa"
        runTest("abcabc");       // Expected: "abc"
        runTest("mississippi");  // Expected: "issi" (or another valid longest)
    }
}