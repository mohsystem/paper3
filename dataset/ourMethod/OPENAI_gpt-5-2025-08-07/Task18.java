import java.util.Arrays;

public class Task18 {

    public static int binaryArrayToInt(int[] bits) {
        if (bits == null) {
            throw new IllegalArgumentException("Input array must not be null.");
        }
        long result = 0L;
        for (int i = 0; i < bits.length; i++) {
            int bit = bits[i];
            if (bit != 0 && bit != 1) {
                throw new IllegalArgumentException("Invalid bit at index " + i + ": " + bit + ". Only 0 or 1 allowed.");
            }
            if (result > (Integer.MAX_VALUE - bit) / 2L) {
                throw new IllegalArgumentException("Overflow: binary value exceeds Integer.MAX_VALUE.");
            }
            result = (result << 1) | bit;
        }
        return (int) result;
    }

    private static String arrToString(int[] arr) {
        StringBuilder sb = new StringBuilder();
        sb.append('[');
        for (int i = 0; i < arr.length; i++) {
            sb.append(arr[i]);
            if (i + 1 < arr.length) sb.append(", ");
        }
        sb.append(']');
        return sb.toString();
    }

    public static void main(String[] args) {
        int[][] tests = new int[][] {
            {0, 0, 0, 1},
            {0, 0, 1, 0},
            {0, 1, 0, 1},
            {1, 0, 0, 1},
            {1, 1, 1, 1}
        };

        for (int[] t : tests) {
            try {
                int result = binaryArrayToInt(t);
                System.out.println("Testing: " + arrToString(t) + " ==> " + result);
            } catch (IllegalArgumentException ex) {
                System.out.println("Error for input " + arrToString(t) + ": " + ex.getMessage());
            }
        }
    }
}