import java.io.*;
import java.util.*;

public class Task172 {
    private static final long MOD1 = 1_000_000_007L;
    private static final long MOD2 = 1_000_000_009L;
    private static final long BASE1 = 911_382_323L;
    private static final long BASE2 = 972_663_749L;

    public static int countEvenSquareDistinct(String text) {
        if (text == null) return 0;
        int n = text.length();
        if (n <= 1) return 0;

        long[] pow1 = new long[n + 1];
        long[] pow2 = new long[n + 1];
        long[] pref1 = new long[n + 1];
        long[] pref2 = new long[n + 1];

        pow1[0] = 1; pow2[0] = 1;
        for (int i = 0; i < n; i++) {
            int v = (text.charAt(i) - 'a' + 1);
            pref1[i + 1] = (pref1[i] * BASE1 + v) % MOD1;
            pref2[i + 1] = (pref2[i] * BASE2 + v) % MOD2;
            pow1[i + 1] = (pow1[i] * BASE1) % MOD1;
            pow2[i + 1] = (pow2[i] * BASE2) % MOD2;
        }

        class Key {
            final long h1, h2;
            final int len;
            Key(long a, long b, int l) { h1 = a; h2 = b; len = l; }
            @Override public boolean equals(Object o) {
                if (this == o) return true;
                if (!(o instanceof Key)) return false;
                Key k = (Key) o;
                return h1 == k.h1 && h2 == k.h2 && len == k.len;
            }
            @Override public int hashCode() {
                long x = h1 * 1315423911L ^ (h2 << 13) ^ len;
                return (int)(x ^ (x >>> 32));
            }
        }

        Set<Key> set = new HashSet<>();

        for (int len = 2; len <= n; len += 2) {
            int half = len / 2;
            for (int i = 0; i + len <= n; i++) {
                int mid = i + half;
                int j = i + len - 1;
                long a1 = subHash(pref1, pow1, i, mid - 1, MOD1);
                long b1 = subHash(pref1, pow1, mid, j, MOD1);
                if (a1 != b1) continue;
                long a2 = subHash(pref2, pow2, i, mid - 1, MOD2);
                long b2 = subHash(pref2, pow2, mid, j, MOD2);
                if (a2 != b2) continue;
                long h1 = subHash(pref1, pow1, i, j, MOD1);
                long h2 = subHash(pref2, pow2, i, j, MOD2);
                set.add(new Key(h1, h2, len));
            }
        }
        return set.size();
    }

    private static long subHash(long[] pref, long[] pow, int l, int r, long mod) {
        long res = (pref[r + 1] - (pref[l] * pow[r - l + 1]) % mod);
        if (res < 0) res += mod;
        return res;
    }

    public static void main(String[] args) {
        String[] tests = {
            "abcabcabc",
            "leetcodeleetcode",
            "aaaa",
            "ababa",
            "z"
        };
        for (String s : tests) {
            System.out.println(countEvenSquareDistinct(s));
        }
    }
}