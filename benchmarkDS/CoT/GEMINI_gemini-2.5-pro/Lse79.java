import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.util.Arrays;

public class Lse79 {

    /**
     * Allocates two 1024-byte buffers, and for each, reads up to 100 characters
     * from standard input.
     *
     * @param inputReader A BufferedReader for reading from stdin.
     */
    public static void performAllocationAndRead(BufferedReader inputReader) {
        // The first method allocates 1024 bytes of memory and stores the address
        // of the allocated memory in the variable buf.
        char[] buf = new char[1024];

        // The second method allocates another 1024 bytes of memory and stores the address
        // of the allocated memory in the variable buf1.
        char[] buf1 = new char[1024];

        try {
            System.out.println("Enter up to 100 characters for the first buffer:");
            // The function reads up to 100 characters from standard input and stores them.
            // Reading at most 100 chars into a 1024 buffer is safe from overflow.
            int charsRead1 = inputReader.read(buf, 0, 100);
            if (charsRead1 == -1) { // End of stream
                System.out.println("End of input stream reached.");
                return;
            }
            // Consume the rest of the line to prevent it from being read by the next input
            if (inputReader.ready()) {
                 inputReader.readLine();
            }


            System.out.println("Enter up to 100 characters for the second buffer:");
            // Copy in 100 characters from stdin for the second buffer.
            int charsRead2 = inputReader.read(buf1, 0, 100);
             if (charsRead2 == -1) { // End of stream
                System.out.println("End of input stream reached.");
                return;
            }

            System.out.print("Content of first buffer: ");
            if (charsRead1 > 0) {
                System.out.println(new String(buf, 0, charsRead1));
            } else {
                System.out.println("[EMPTY]");
            }

            System.out.print("Content of second buffer: ");
            if (charsRead2 > 0) {
                System.out.println(new String(buf1, 0, charsRead2));
            } else {
                System.out.println("[EMPTY]");
            }

        } catch (IOException e) {
            System.err.println("An I/O error occurred: " + e.getMessage());
        }
        // In Java, memory is garbage collected, so no manual deallocation is needed.
    }

    public static void main(String[] args) {
        // Main method with 5 test cases.
        // The user will be prompted to provide input for each test case.
        BufferedReader inputReader = new BufferedReader(new InputStreamReader(System.in));
        for (int i = 1; i <= 5; i++) {
            System.out.println("\n--- Test Case " + i + " ---");
            performAllocationAndRead(inputReader);
            // Consume any leftover newline characters before the next test case
            try {
                if (inputReader.ready()) {
                    inputReader.readLine();
                }
            } catch(IOException e) {
                 System.err.println("An I/O error occurred while clearing buffer: " + e.getMessage());
            }
        }
        System.out.println("\nAll test cases finished.");
    }
}