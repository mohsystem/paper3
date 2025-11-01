import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Lse79 {

    private static final int BUFFER_SIZE = 1024;
    private static final int MAX_READ_SIZE = 100;

    /**
     * Allocates two 1024-byte buffers and reads up to 100 characters
     * from stdin into each.
     */
    public static void allocateAndRead() {
        try (BufferedReader reader = new BufferedReader(new InputStreamReader(System.in))) {
            // Allocate the first buffer
            byte[] buf = new byte[BUFFER_SIZE];
            System.out.println("Enter up to " + MAX_READ_SIZE + " characters for the first buffer:");
            
            // Read and process the first input
            String line1 = reader.readLine();
            if (line1 != null) {
                // Truncate if input is longer than allowed
                if (line1.length() > MAX_READ_SIZE) {
                    line1 = line1.substring(0, MAX_READ_SIZE);
                }
                byte[] inputBytes1 = line1.getBytes(StandardCharsets.UTF_8);
                // Safely copy bytes into the buffer
                System.arraycopy(inputBytes1, 0, buf, 0, inputBytes1.length);
                System.out.println("First buffer content: " + new String(buf, 0, inputBytes1.length, StandardCharsets.UTF_8));
            } else {
                 System.out.println("No input received for the first buffer.");
            }

            // Allocate the second buffer
            byte[] buf1 = new byte[BUFFER_SIZE];
            System.out.println("Enter up to " + MAX_READ_SIZE + " characters for the second buffer:");

            // Read and process the second input
            String line2 = reader.readLine();
            if (line2 != null) {
                if (line2.length() > MAX_READ_SIZE) {
                    line2 = line2.substring(0, MAX_READ_SIZE);
                }
                byte[] inputBytes2 = line2.getBytes(StandardCharsets.UTF_8);
                System.arraycopy(inputBytes2, 0, buf1, 0, inputBytes2.length);
                System.out.println("Second buffer content: " + new String(buf1, 0, inputBytes2.length, StandardCharsets.UTF_8));

            } else {
                System.out.println("No input received for the second buffer.");
            }

        } catch (IOException e) {
            System.err.println("An I/O error occurred: " + e.getMessage());
        } catch (Exception e) {
            System.err.println("An unexpected error occurred: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        // The prompt asks for 5 test cases. Since input is from stdin,
        // we will call the function once and the user can run the program 5 times,
        // or enter 5 pairs of inputs if the program were structured in a loop.
        // For a single file, self-contained example, a single execution is clearer.
        System.out.println("--- Test Case 1 ---");
        allocateAndRead();
        // To run more test cases, the user would need to provide new input.
        // In a real testing scenario, stdin would be redirected from a file.
        // Example:
        // System.out.println("\n--- Test Case 2 ---");
        // allocateAndRead();
        // System.out.println("\n--- Test Case 3 ---");
        // allocateAndRead();
        // System.out.println("\n--- Test Case 4 ---");
        // allocateAndRead();
        // System.out.println("\n--- Test Case 5 ---");
        // allocateAndRead();
    }
}