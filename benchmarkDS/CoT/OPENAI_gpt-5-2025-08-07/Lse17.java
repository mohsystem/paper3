// Chain-of-Through process:
// 1) Problem understanding: Provide a function that returns the value at a specified index in an array safely.
//    Inputs: int[] arr, int index. Output: OptionalInt containing the value if valid; empty otherwise.
// 2) Security requirements: Avoid out-of-bounds access, handle null arrays, avoid exceptions by validating inputs.
// 3) Secure coding generation: Validate array and index bounds before access; no user input parsing in core logic.
// 4) Code review: Checked for null checks, bounds validation, and safe access patterns.
// 5) Secure code output: Final code ensures no unsafe memory access and clear result signaling.
import java.util.OptionalInt;

public class Lse17 {

    // Secure function: returns OptionalInt.empty() if input invalid or index out of bounds
    public static OptionalInt getValueAtIndex(int[] arr, int index) {
        if (arr == null) {
            return OptionalInt.empty();
        }
        if (index < 0 || index >= arr.length) {
            return OptionalInt.empty();
        }
        return OptionalInt.of(arr[index]);
    }

    // 5 test cases as required
    public static void main(String[] args) {
        int[] data = new int[] {10, 20, 30, 40, 50};

        int[] testIndices = new int[] {0, 2, 4, -1, 10};

        for (int idx : testIndices) {
            OptionalInt result = getValueAtIndex(data, idx);
            if (result.isPresent()) {
                System.out.println("Index " + idx + " -> " + result.getAsInt());
            } else {
                System.out.println("Index " + idx + " -> invalid index");
            }
        }
    }
}