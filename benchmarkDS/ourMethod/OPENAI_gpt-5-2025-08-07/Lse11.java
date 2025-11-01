import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;

public class Lse11 {
    public static int getValueFromArray(int[] arr, int size, int index) {
        if (arr == null) {
            throw new IllegalArgumentException("Array is null");
        }
        if (size < 0 || size > arr.length) {
            throw new IllegalArgumentException("Invalid size");
        }
        if (index < 0 || index >= size) {
            throw new IndexOutOfBoundsException("Invalid index");
        }
        return arr[index];
    }

    public static void main(String[] args) {
        final int[] id_sequence = new int[] {3, 1, 4, 1, 5, 9, 2, 6, 5, 3};
        final int size = id_sequence.length;

        // 5 test cases
        int[] testIndices = new int[] {0, 3, 9, 5, 7};
        for (int idx : testIndices) {
            try {
                int val = getValueFromArray(id_sequence, size, idx);
                System.out.println("Test index " + idx + " => " + val);
            } catch (Exception ex) {
                System.out.println("Test index " + idx + " => ERROR");
            }
        }

        // Read one integer index from stdin and return value as exit code
        BufferedReader br = new BufferedReader(new InputStreamReader(System.in, StandardCharsets.UTF_8));
        String line;
        try {
            line = br.readLine();
            if (line == null) {
                System.err.println("ERROR");
                System.exit(1);
                return;
            }
            line = line.trim();
            if (line.length() == 0 || line.length() > 32) {
                System.err.println("ERROR");
                System.exit(1);
                return;
            }
            long lidx = Long.parseLong(line);
            if (lidx < Integer.MIN_VALUE || lidx > Integer.MAX_VALUE) {
                System.err.println("ERROR");
                System.exit(1);
                return;
            }
            int index = (int) lidx;
            if (index < 0 || index >= size) {
                System.err.println("ERROR");
                System.exit(1);
                return;
            }
            int result = getValueFromArray(id_sequence, size, index);
            System.exit(result);
        } catch (Exception e) {
            System.err.println("ERROR");
            System.exit(1);
        }
    }
}