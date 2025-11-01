import java.util.Arrays;

public class Lse14 {
    // Immutable ID sequence
    private static final int[] ID_SEQUENCE = new int[] {101, 202, 303, 404, 505};

    // Returns the value at the given index, or -1 if index is invalid
    public static int getIdAtIndex(int index) {
        if (index < 0 || index >= ID_SEQUENCE.length) {
            return -1; // Sentinel for invalid index
        }
        return ID_SEQUENCE[index];
    }

    // 5 test cases
    public static void main(String[] args) {
        int[] tests = {0, 4, 2, -1, 10};
        for (int idx : tests) {
            System.out.println(getIdAtIndex(idx));
        }
    }
}