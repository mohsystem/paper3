public class Task104 {

    /**
     * Securely handles user input by copying it into a fixed-size buffer.
     * In Java, direct buffer overflow is prevented by the language's memory safety.
     * This function simulates secure handling by truncating the input if it's too long.
     *
     * @param input The user-provided string.
     * @param bufferSize The maximum size of the buffer.
     * @return The potentially truncated string that fits in the buffer.
     */
    public static String handleInput(String input, int bufferSize) {
        if (input == null || bufferSize <= 0) {
            return "";
        }
        if (input.length() >= bufferSize) {
            // Truncate the string to fit the buffer size.
            // In Java, this prevents an exception if copying to a fixed char array.
            return input.substring(0, bufferSize);
        }
        return input;
    }

    public static void main(String[] args) {
        final int BUFFER_SIZE = 16;

        // Test Cases
        String[] testInputs = {
            "Short string",      // Case 1: Shorter than buffer
            "Exactly 16 chars",  // Case 2: Exactly buffer size
            "This is a very long string that will be truncated", // Case 3: Longer than buffer
            "",                  // Case 4: Empty string
            "Fifteen chars!!",   // Case 5: One less than buffer size
        };

        System.out.println("Java Test Cases (Buffer Size: " + BUFFER_SIZE + ")");
        System.out.println("------------------------------------");
        for (int i = 0; i < testInputs.length; i++) {
            String original = testInputs[i];
            String processed = handleInput(original, BUFFER_SIZE);
            System.out.printf("Test Case %d:\n", i + 1);
            System.out.printf("  Original:  \"%s\" (length: %d)\n", original, original.length());
            System.out.printf("  Processed: \"%s\" (length: %d)\n\n", processed, processed.length());
        }
    }
}