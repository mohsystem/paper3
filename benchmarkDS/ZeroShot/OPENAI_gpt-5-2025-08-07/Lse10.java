import java.io.*;
import java.util.*;

public class Lse10 {
    public static int getValueFromArray(int[] arr, int size, int index) {
        if (arr == null || size <= 0) return 0;
        int bound = Math.min(size, arr.length);
        if (index >= 0 && index < bound) {
            return arr[index];
        }
        return 0;
    }

    public static void main(String[] args) {
        int[] id_sequence = new int[] {7, 13, 42, -1, 255, 1024, -300};
        int size = id_sequence.length;

        // 5 test cases
        int[] testIndices = new int[] {0, 3, 6, -1, 100};
        for (int i = 0; i < testIndices.length; i++) {
            int val = getValueFromArray(id_sequence, size, testIndices[i]);
            System.out.println("Test " + (i + 1) + " index=" + testIndices[i] + " value=" + val);
        }

        int idx = 0;
        try (Scanner sc = new Scanner(System.in)) {
            if (sc.hasNext()) {
                String tok = sc.next();
                try {
                    long lv = Long.parseLong(tok.trim());
                    if (lv > Integer.MAX_VALUE) idx = Integer.MAX_VALUE;
                    else if (lv < Integer.MIN_VALUE) idx = Integer.MIN_VALUE;
                    else idx = (int) lv;
                } catch (NumberFormatException e) {
                    idx = 0;
                }
            }
        }

        int value = getValueFromArray(id_sequence, size, idx);
        int exitCode = value & 0xFF;
        System.exit(exitCode);
    }
}