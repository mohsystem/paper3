import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

public class Lse12 {
    public static int getValueFromArray(int[] arr, int size, int index) {
        if (arr == null) {
            throw new IllegalArgumentException("Array is null");
        }
        if (size < 0 || size > arr.length) {
            throw new IllegalArgumentException("Invalid size");
        }
        if (index < 0 || index >= size) {
            throw new IndexOutOfBoundsException("Index out of bounds");
        }
        return arr[index];
    }

    private static int toExitCode(int value) {
        return value & 0xFF;
    }

    public static void main(String[] args) {
        int[] example = new int[] {10, 20, 30, 40, 50};
        int size = example.length;

        BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
        String line = null;
        try {
            line = br.readLine();
        } catch (IOException e) {
            System.exit(1);
        }

        if (line != null) {
            line = line.trim();
        }
        if (line != null && !line.isEmpty()) {
            int idx;
            try {
                idx = Integer.parseInt(line);
            } catch (NumberFormatException e) {
                System.exit(1);
                return;
            }
            if (idx < 0 || idx >= size) {
                System.exit(1);
                return;
            }
            try {
                int value = getValueFromArray(example, size, idx);
                System.exit(toExitCode(value));
            } catch (RuntimeException e) {
                System.exit(1);
            }
        } else {
            // No input provided: run 5 test cases
            for (int i = 0; i < size; i++) {
                try {
                    int value = getValueFromArray(example, size, i);
                    System.out.println("Test " + (i + 1) + ": " + value);
                } catch (RuntimeException e) {
                    System.out.println("Test " + (i + 1) + ": error");
                }
            }
            System.exit(0);
        }
    }
}