import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;

public class Lse13 {
    private static final int[] ID_SEQUENCE = new int[] {7, 13, 21, 34, 55, 89, 144, 233, 377, 610};

    public static int getValueAtIndex(int index) {
        if (index < 0 || index >= ID_SEQUENCE.length) {
            throw new IllegalArgumentException("invalid index");
        }
        return ID_SEQUENCE[index];
    }

    public static void main(String[] args) {
        System.out.print("Enter an index (0.." + (ID_SEQUENCE.length - 1) + "): ");
        try (BufferedReader br = new BufferedReader(new InputStreamReader(System.in, StandardCharsets.UTF_8))) {
            String line = br.readLine();
            if (line == null) {
                System.err.println("Error: no input.");
            } else {
                line = line.trim();
                if (line.length() == 0 || line.length() > 20) {
                    System.err.println("Error: invalid input.");
                } else {
                    int idx;
                    try {
                        idx = Integer.parseInt(line);
                    } catch (NumberFormatException e) {
                        System.err.println("Error: invalid input.");
                        idx = -1; // sentinel to skip call
                    }
                    if (idx >= 0) {
                        if (idx < 0 || idx >= ID_SEQUENCE.length) {
                            System.err.println("Error: invalid index.");
                        } else {
                            int value = getValueAtIndex(idx);
                            System.out.println("Value at index " + idx + " = " + value);
                        }
                    }
                }
            }
        } catch (IOException e) {
            System.err.println("Error: I/O.");
        }

        // 5 test cases
        int[] tests = new int[] {0, ID_SEQUENCE.length - 1, 2, 5, 7};
        for (int t : tests) {
            try {
                int v = getValueAtIndex(t);
                System.out.println("Test index " + t + " -> " + v);
            } catch (IllegalArgumentException e) {
                System.out.println("Test index " + t + " -> error");
            }
        }
    }
}