import java.util.*;

public class Task23 {
    public static long findNb(long m) {
        if (m < 0) return -1;
        long n = 0;
        while (m > 0) {
            n++;
            long cube = n * n * n;
            m -= cube;
        }
        return m == 0 ? n : -1;
    }

    public static void main(String[] args) {
        long[] tests = {
            1071225L,          // expected 45
            91716553919377L,   // expected -1
            4183059834009L,    // expected 2022
            0L,                // expected 0
            135440716410000L   // expected 4824
        };
        for (long m : tests) {
            System.out.println(findNb(m));
        }
    }
}