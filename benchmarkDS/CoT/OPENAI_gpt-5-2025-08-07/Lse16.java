// Chain-of-Through process in code generation:
// 1) Problem understanding: Return value at given index from a predefined array.
// 2) Security requirements: Prevent out-of-bounds access and invalid indices.
// 3) Secure coding generation: Validate index before access; avoid mutable global exposure.
// 4) Code review: Ensure checks are correct, no overflow or negative indices, and no unsafe operations.
// 5) Secure code output: Final code with bounds checks and clear error handling.

public class Lse16 {
    private static final int[] DATA = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

    // Returns the value at the specified index if valid, otherwise throws an exception.
    public static int valueAtIndex(int index) {
        if (index < 0 || index >= DATA.length) {
            throw new IllegalArgumentException("Index out of bounds: " + index);
        }
        return DATA[index];
    }

    // 5 test cases
    public static void main(String[] args) {
        int[] tests = {0, 5, 9, 3, 7};
        for (int idx : tests) {
            try {
                int val = valueAtIndex(idx);
                System.out.println("Index " + idx + " -> " + val);
            } catch (IllegalArgumentException ex) {
                System.out.println("Error for index " + idx + ": " + ex.getMessage());
            }
        }
    }
}