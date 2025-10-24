import java.util.*;

public class Task172 {
    static final long MOD1 = 1_000_000_007L;
    static final long MOD2 = 1_000_000_009L;
    static final long BASE1 = 911_382_323L;
    static final long BASE2 = 972_663_749L;

    private static long getHash(long[] pref, long[] pow, long mod, int l, int r) {
        long res = (pref[r + 1] - (pref[l] * pow[r - l + 1]) % mod);
        if (res < 0) res += mod;
        return res;
    }

    public static int countSquareDistinctSubstrings(String s) {
        int n = s.length();
        long[] pow1 = new long[n + 1];
        long[] pow2 = new long[n + 1];
        long[] pref1 = new long[n + 1];
        long[] pref2 = new long[n + 1];

        pow1[0] = 1;
        pow2[0] = 1;
        for (int i = 1; i <= n; i++) {
            pow1[i] = (pow1[i - 1] * BASE1) % MOD1;
            pow2[i] = (pow2[i - 1] * BASE2) % MOD2;
        }

        for (int i = 0; i < n; i++) {
            int v = s.charAt(i) - 'a' + 1;
            pref1[i + 1] = (pref1[i] * BASE1 + v) % MOD1;
            pref2[i + 1] = (pref2[i] * BASE2 + v) % MOD2;
        }

        HashSet<Long> set = new HashSet<>();
        for (int l = 0; l < n; l++) {
            for (int len = 2; l + len <= n; len += 2) {
                int mid = l + len / 2;
                int r = l + len - 1;
                long h1a = getHash(pref1, pow1, MOD1, l, mid - 1);
                long h1b = getHash(pref1, pow1, MOD1, mid, r);
                if (h1a != h1b) continue;
                long h2a = getHash(pref2, pow2, MOD2, l, mid - 1);
                long h2b = getHash(pref2, pow2, MOD2, mid, r);
                if (h2a != h2b) continue;

                long hs1 = getHash(pref1, pow1, MOD1, l, r);
                long hs2 = getHash(pref2, pow2, MOD2, l, r);
                long key = ((hs1 & 0xffffffffL) << 32) ^ (hs2 & 0xffffffffL);
                set.add(key);
            }
        }
        return set.size();
    }

    public static void main(String[] args) {
        String[] tests = {
            "abcabcabc",
            "leetcodeleetcode",
            "aaaa",
            "abab",
            "xyz"
        };
        for (String t : tests) {
            System.out.println(countSquareDistinctSubstrings(t));
        }
    }
}