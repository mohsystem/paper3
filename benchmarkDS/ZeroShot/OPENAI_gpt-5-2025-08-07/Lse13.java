import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;

public class Lse13 {
    private static final int[] ID_SEQUENCE = new int[] {101, 203, 307, 409, 512, 678, 789, 890, 901, 1024};

    public static int getIdAtIndex(int index) {
        if (index < 0 || index >= ID_SEQUENCE.length) {
            throw new IndexOutOfBoundsException("Index out of bounds");
        }
        return ID_SEQUENCE[index];
    }

    public static void main(String[] args) {
        // Prompt user for an index and display the value
        System.out.print("Enter index (0-" + (ID_SEQUENCE.length - 1) + "): ");
        try (BufferedReader br = new BufferedReader(new InputStreamReader(System.in, StandardCharsets.UTF_8))) {
            String line = br.readLine();
            if (line != null) {
                line = line.trim();
                try {
                    int idx = Integer.parseInt(line);
                    int value = getIdAtIndex(idx);
                    System.out.println("Value at index " + idx + " is: " + value);
                } catch (NumberFormatException e) {
                    System.out.println("Invalid input: not a valid integer.");
                } catch (IndexOutOfBoundsException e) {
                    System.out.println("Invalid index: must be between 0 and " + (ID_SEQUENCE.length - 1) + ".");
                }
            } else {
                System.out.println("No input received.");
            }
        } catch (IOException e) {
            System.out.println("I/O error occurred.");
        }

        // Five test cases
        int[] testIndexes = new int[] {0, 3, 9, -1, 100};
        System.out.println("Running 5 test cases:");
        for (int testIdx : testIndexes) {
            try {
                int result = getIdAtIndex(testIdx);
                System.out.println("Index " + testIdx + " -> " + result);
            } catch (IndexOutOfBoundsException e) {
                System.out.println("Index " + testIdx + " -> error: " + e.getMessage());
            }
        }
    }
}