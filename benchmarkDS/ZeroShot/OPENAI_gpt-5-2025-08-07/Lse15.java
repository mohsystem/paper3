import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

public class Lse15 {

    // Fixed sequence for demonstration
    private static final int[] ID_SEQUENCE = {101, 203, 307, 409, 512, 618, 723, 834, 945, 1056};

    // Function: takes index as parameter and returns the value at that index
    public static int getValueAtIndex(int index) {
        if (index < 0 || index >= ID_SEQUENCE.length) {
            throw new IllegalArgumentException("Index out of bounds.");
        }
        return ID_SEQUENCE[index];
    }

    private static void runSinglePrompt() {
        System.out.print("Enter an index (0-" + (ID_SEQUENCE.length - 1) + "): ");
        try (BufferedReader br = new BufferedReader(new InputStreamReader(System.in))) {
            String line = br.readLine();
            if (line == null) {
                System.out.println("No input provided.");
                return;
            }
            line = line.trim();
            if (line.isEmpty()) {
                System.out.println("Empty input.");
                return;
            }
            long parsed = Long.parseLong(line);
            if (parsed < Integer.MIN_VALUE || parsed > Integer.MAX_VALUE) {
                System.out.println("Input out of integer range.");
                return;
            }
            int idx = (int) parsed;
            try {
                int value = getValueAtIndex(idx);
                System.out.println("Value at index " + idx + " is: " + value);
            } catch (IllegalArgumentException ex) {
                System.out.println("Error: " + ex.getMessage());
            }
        } catch (NumberFormatException nfe) {
            System.out.println("Invalid number format.");
        } catch (IOException ioe) {
            System.out.println("I/O error: " + ioe.getMessage());
        }
    }

    public static void main(String[] args) {
        // Prompt the user for an index
        runSinglePrompt();

        // Five test cases
        int[] testIndices = {0, 3, 9, -1, 100};
        System.out.println("Running 5 test cases:");
        for (int idx : testIndices) {
            try {
                int value = getValueAtIndex(idx);
                System.out.println("Index " + idx + " -> " + value);
            } catch (IllegalArgumentException ex) {
                System.out.println("Index " + idx + " -> Error: " + ex.getMessage());
            }
        }
    }
}