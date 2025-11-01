import java.nio.charset.StandardCharsets;
import java.util.Scanner;

public class Lse14 {
    // Fixed id_sequence array
    private static final int[] ID_SEQUENCE = {101, 203, 304, 405, 506, 607, 708, 809, 910, 1001};

    // Returns the value at the given index; throws IllegalArgumentException if invalid
    public static int getIdAtIndex(int index) {
        if (index < 0 || index >= ID_SEQUENCE.length) {
            throw new IllegalArgumentException("Invalid index.");
        }
        return ID_SEQUENCE[index];
    }

    public static void main(String[] args) {
        // Prompt the user securely for an index
        try (Scanner scanner = new Scanner(System.in, StandardCharsets.UTF_8.name())) {
            System.out.print("Enter an index (0.." + (ID_SEQUENCE.length - 1) + "): ");
            String line = scanner.nextLine();
            try {
                int userIndex = Integer.parseInt(line.trim());
                int value = getIdAtIndex(userIndex);
                System.out.println("Value at index " + userIndex + ": " + value);
            } catch (NumberFormatException e) {
                System.out.println("Invalid input.");
            } catch (IllegalArgumentException e) {
                System.out.println("Invalid index.");
            }
        } catch (Exception e) {
            System.out.println("I/O error.");
        }

        // 5 Test cases
        int[] tests = {0, 3, 9, -1, 10};
        System.out.println("Running test cases:");
        for (int idx : tests) {
            try {
                int val = getIdAtIndex(idx);
                System.out.println("Index " + idx + " => " + val);
            } catch (IllegalArgumentException e) {
                System.out.println("Index " + idx + " => error");
            }
        }
    }
}