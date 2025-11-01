import java.util.Scanner;

public class Lse15 {
    // Chain-of-Through process:
    // 1) Problem understanding: Function prompts user for an index and returns the value at that index from id_sequence.
    // 2) Security requirements: Validate input type and bounds; handle EOF/invalid input robustly.
    // 3) Secure coding generation: Use safe parsing with try/catch and loops; avoid trust in external input; no unchecked array access.
    // 4) Code review: Ensure no unvalidated index usage; ensure scanner handling and clean returns on failure.
    // 5) Secure code output: Finalized with explicit checks and clear error handling.

    private static final int[] ID_SEQUENCE = new int[] {101, 202, 303, 404, 505, 606, 707, 808, 909, 1001};
    private static final Scanner SC = new Scanner(System.in);

    // Function accepts a parameter but, per requirement, first prompts the user for the index.
    // Returns the value at that index in ID_SEQUENCE if valid; otherwise returns -1.
    public static int getValueAtIndex(int indexParam) {
        while (true) {
            System.out.print("Enter index (0.." + (ID_SEQUENCE.length - 1) + "): ");
            String line;
            try {
                if (!SC.hasNextLine()) {
                    return -1; // EOF or input closed
                }
                line = SC.nextLine().trim();
            } catch (Exception e) {
                return -1;
            }
            if (line.isEmpty()) {
                System.out.println("Invalid input: empty. Please enter an integer index.");
                continue;
            }
            int idx;
            try {
                // Safe parse
                idx = Integer.parseInt(line);
            } catch (NumberFormatException nfe) {
                System.out.println("Invalid input: not an integer. Try again.");
                continue;
            }
            if (idx < 0 || idx >= ID_SEQUENCE.length) {
                System.out.println("Index out of bounds. Valid range is 0.." + (ID_SEQUENCE.length - 1) + ". Try again.");
                continue;
            }
            return ID_SEQUENCE[idx];
        }
    }

    public static void main(String[] args) {
        // 5 test cases; function will prompt for index each time
        for (int t = 1; t <= 5; t++) {
            int value = getValueAtIndex(-1);
            System.out.println("Test " + t + " -> Value: " + value);
        }
    }
}