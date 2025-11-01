import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Lse18 {
    public static int getValueAtIndex(int index, int[] arr) {
        if (arr == null) {
            throw new IllegalArgumentException("invalid input");
        }
        if (index < 0 || index >= arr.length) {
            throw new IllegalArgumentException("invalid input");
        }
        return arr[index];
    }

    public static void main(String[] args) {
        int[] arr = new int[] {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
        int[] tests = new int[] {0, 5, 9, 3, 10}; // 10 is invalid to demonstrate error handling

        for (int t : tests) {
            try {
                int value = getValueAtIndex(t, arr);
                System.out.println("index " + t + " -> " + value);
            } catch (IllegalArgumentException e) {
                System.out.println("index " + t + " -> error");
            }
        }
    }
}