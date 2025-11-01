// Chain-of-Through process in code generation:
// 1) Problem understanding: Provide a function that returns the value at a given index from id_sequence.
//    Also provide a function that first prompts the user for an index, then returns the value.
// 2) Security requirements: Validate inputs, guard against null arrays, invalid indices, and bad user input.
// 3) Secure coding generation: Implement bounds checks and safe parsing without throwing on user errors.
// 4) Code review: Ensure no unsafe operations (no out-of-bounds access), handle I/O errors gracefully.
// 5) Secure code output: Return -1 for invalid scenarios to avoid exceptions and unsafe behavior.

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

public class Lse14 {

    // Returns the value at the given index in id_sequence; returns -1 if invalid input.
    public static int getValueAtIndex(int[] id_sequence, int index) {
        if (id_sequence == null) {
            return -1;
        }
        if (index < 0 || index >= id_sequence.length) {
            return -1;
        }
        return id_sequence[index];
    }

    // Prompts the user for an index and returns the value at that index; returns -1 on invalid input.
    public static int promptAndGetValue(int[] id_sequence) {
        BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
        System.out.print("Enter index: ");
        String line;
        try {
            line = br.readLine();
        } catch (IOException e) {
            return -1;
        }
        if (line == null) {
            return -1;
        }
        line = line.trim();
        if (line.isEmpty()) {
            return -1;
        }
        int idx;
        try {
            idx = Integer.parseInt(line);
        } catch (NumberFormatException e) {
            return -1;
        }
        return getValueAtIndex(id_sequence, idx);
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        int[] id_sequence = new int[] {10, 20, 30, 40, 50};

        int[] testIndices = new int[] {0, 2, 4, 1, 3};
        for (int idx : testIndices) {
            int result = getValueAtIndex(id_sequence, idx);
            System.out.println("Index " + idx + " -> " + result);
        }

        // Uncomment to test interactive prompt:
        // int userResult = promptAndGetValue(id_sequence);
        // System.out.println("User-selected value -> " + userResult);
    }
}