import java.io.*;
import java.util.*;

public class Lse11 {
    public static int getValueFromArray(int[] arr, int size, int index) {
        if (index < 0 || index >= size) return 0;
        return arr[index];
    }

    public static void main(String[] args) throws Exception {
        int[] id_sequence = new int[] {10, 20, 30, 40, 50, 60, 70, 80};
        Scanner sc = new Scanner(System.in);
        if (sc.hasNextInt()) {
            int idx = sc.nextInt();
            int val = getValueFromArray(id_sequence, id_sequence.length, idx);
            System.exit(val & 0xFF);
        } else {
            int[] tests = new int[] {0, 3, id_sequence.length - 1, -1, id_sequence.length};
            int last = 0;
            for (int t : tests) {
                int v = getValueFromArray(id_sequence, id_sequence.length, t);
                System.out.println(v);
                last = v;
            }
            System.exit(last & 0xFF);
        }
    }
}