import java.io.*;
import java.util.*;

public class Task172 {
    private static final long MOD1 = 1_000_000_007L;
    private static final long MOD2 = 1_000_000_009L;
    private static final long BASE1 = 911_382_323L;
    private static final long BASE2 = 972_663_749L;

    private static final class RollingHash {
        private final long[] h1, h2, p1, p2;

        RollingHash(String s) {
            int n = s.length();
            h1 = new long[n + 1];
            h2 = new long[n + 1];
            p1 = new long[n + 1];
            p2 = new long[n + 1];
            p1[0] = 1L;
            p2[0] = 1L;
            for (int i = 0; i < n; i++) {
                int v = s.charAt(i) - 'a' + 1;
                h1[i + 1] = (h1[i] * BASE1 + v) % MOD1;
                h2[i + 1] = (h2[i] * BASE2 + v) % MOD2;
                p1[i + 1] = (p1[i] * BASE1) % MOD1;
                p2[i + 1] = (p2[i] * BASE2) % MOD2;
            }
        }

        long[] get(int l, int r) { // [l, r)
            long x1 = (h1[r] - (h1[l] * p1[r - l]) % MOD1);
            if (x1 < 0) x1 += MOD1;
            long x2 = (h2[r] - (h2[l] * p2[r - l]) % MOD2);
            if (x2 < 0) x2 += MOD2;
            return new long[]{x1, x2};
        }
    }

    private static final class Key {
        final long h1, h2;
        final int len;

        Key(long h1, long h2, int len) {
            this.h1 = h1;
            this.h2 = h2;
            this.len = len;
        }

        @Override
        public boolean equals(Object o) {
            if (this == o) return true;
            if (!(o instanceof Key)) return false;
            Key k = (Key) o;
            return h1 == k.h1 && h2 == k.h2 && len == k.len;
        }

        @Override
        public int hashCode() {
            long a = h1 ^ (h1 >>> 32);
            long b = h2 ^ (h2 >>> 32);
            int res = 17;
            res = res * 31 + (int) a;
            res = res * 31 + (int) b;
            res = res * 31 + len;
            return res;
        }
    }

    public static int countSquareSubstrings(String text) {
        if (text == null) return 0;
        int n = text.length();
        if (n <= 1) return 0;
        RollingHash rh = new RollingHash(text);
        HashSet<Key> set = new HashSet<>();
        for (int len = 2; len <= n; len += 2) {
            for (int i = 0; i + len <= n; i++) {
                int mid = i + len / 2;
                long[] left = rh.get(i, mid);
                long[] right = rh.get(mid, i + len);
                if (left[0] == right[0] && left[1] == right[1]) {
                    long[] full = rh.get(i, i + len);
                    set.add(new Key(full[0], full[1], len));
                }
            }
        }
        return set.size();
    }

    public static void main(String[] args) {
        String[] tests = new String[]{
            "abcabcabc",
            "leetcodeleetcode",
            "aaaaaa",
            "abcd",
            "abbaabba"
        };
        for (String t : tests) {
            System.out.println(countSquareSubstrings(t));
        }
    }
}