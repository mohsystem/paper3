import java.io.IOException;
import java.util.Scanner;

public class Lse13 {
    private static final int[] ID_SEQUENCE = {101, 205, 309, 412, 518, 623, 734, 845, 956, 1067};

    public static int getValueAtIndex(int index) {
        if (index < 0 || index >= ID_SEQUENCE.length) {
            throw new IllegalArgumentException("Index out of range.");
        }
        return ID_SEQUENCE[index];
    }

    public static void main(String[] args) {
        System.out.println("Enter an index to retrieve from id_sequence:");
        try {
            if (System.in.available() > 0) {
                Scanner sc = new Scanner(System.in);
                if (sc.hasNextInt()) {
                    int idx = sc.nextInt();
                    try {
                        int val = getValueAtIndex(idx);
                        System.out.println("Value at index " + idx + " is: " + val);
                    } catch (IllegalArgumentException ex) {
                        System.out.println("Error: " + ex.getMessage());
                    }
                } else {
                    System.out.println("No valid integer input provided.");
                }
                sc.close();
            } else {
                System.out.println("No input detected. Skipping user query.");
            }
        } catch (IOException ioe) {
            System.out.println("I/O error while checking input: " + ioe.getMessage());
        }

        int[] tests = {0, 3, 9, -1, 100};
        System.out.println("Running 5 test cases:");
        for (int t : tests) {
            try {
                int val = getValueAtIndex(t);
                System.out.println("Index " + t + " => " + val);
            } catch (IllegalArgumentException ex) {
                System.out.println("Index " + t + " => Error: " + ex.getMessage());
            }
        }
    }
}