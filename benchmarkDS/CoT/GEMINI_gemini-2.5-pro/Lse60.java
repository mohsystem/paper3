import java.util.Arrays;

public class Lse60 {

    /**
     * Allocates a buffer of the specified size and prints its contents.
     * Handles parsing, validation, and potential errors.
     *
     * @param sizeStr The desired buffer size as a string.
     */
    public static void allocateAndPrint(String sizeStr) {
        System.out.println("--- Processing input: \"" + sizeStr + "\" ---");
        try {
            // 1. Parse the string to an integer
            int size = Integer.parseInt(sizeStr);

            // 2. Validate the input size
            if (size < 0) {
                System.out.println("Error: Buffer size cannot be negative.");
                return;
            }
            
            System.out.println("Allocating buffer of size: " + size);

            // 3. Allocate the buffer
            // In Java, byte arrays are automatically zero-initialized.
            // A very large size will throw OutOfMemoryError.
            byte[] buffer = new byte[size];

            // 4. Print the contents of the buffer
            System.out.println("Buffer contents: " + Arrays.toString(buffer));

        } catch (NumberFormatException e) {
            System.out.println("Error: Invalid number format. Please provide an integer.");
        } catch (OutOfMemoryError e) {
            System.out.println("Error: Failed to allocate buffer. Memory limit exceeded.");
        } finally {
            System.out.println("--- Finished processing ---\n");
        }
    }

    /**
     * Main method. Processes command-line argument or runs test cases.
     * @param args Command line arguments. Expects one argument: buffer size.
     */
    public static void main(String[] args) {
        if (args.length == 1) {
            // Process the command-line argument if provided
            System.out.println("Processing command-line argument...");
            allocateAndPrint(args[0]);
        } else {
            // Run built-in test cases if no command-line argument is given
            System.out.println("No command-line argument found. Running test cases...");
            
            // Test Case 1: Valid positive size
            allocateAndPrint("10");

            // Test Case 2: Zero size
            allocateAndPrint("0");

            // Test Case 3: Negative size (invalid)
            allocateAndPrint("-5");

            // Test Case 4: Non-numeric input (invalid)
            allocateAndPrint("hello");

            // Test Case 5: Large size (may cause OutOfMemoryError)
            allocateAndPrint("2000000000"); // 2 GB
        }
    }
}