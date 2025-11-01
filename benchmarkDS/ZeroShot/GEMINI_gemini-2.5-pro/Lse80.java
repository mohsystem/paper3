import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;

public class Lse80 {

    /**
     * Reads a line from the provided BufferedReader, storing at most 1023 characters,
     * which emulates a 1024-byte buffer with a null terminator.
     * The newline character is read from the stream but not stored in the buffer.
     *
     * @param reader The BufferedReader connected to the input stream (e.g., System.in).
     * @param buffer A StringBuilder to store the resulting string. It will be cleared before use.
     * @return The number of characters read and stored in the buffer, or -1 on I/O error.
     */
    public static int scan(BufferedReader reader, StringBuilder buffer) {
        if (buffer == null || reader == null) {
            return -1; // Indicate error for invalid arguments
        }
        // Clear the buffer for the new input
        buffer.setLength(0);

        try {
            String line = reader.readLine();
            
            // readLine() returns null on end of stream
            if (line == null) {
                return 0;
            }

            // The prompt requires emulating a 1024-byte buffer,
            // so we can store at most 1023 characters.
            final int BUFFER_LIMIT = 1023;
            if (line.length() > BUFFER_LIMIT) {
                buffer.append(line.substring(0, BUFFER_LIMIT));
            } else {
                buffer.append(line);
            }
            
            return buffer.length();

        } catch (IOException e) {
            // In a real application, you might log this error.
            System.err.println("An I/O error occurred: " + e.getMessage());
            return -1; // Indicate error
        }
    }

    public static void main(String[] args) {
        // Create a single BufferedReader for System.in to be used for all reads.
        BufferedReader reader = new BufferedReader(new InputStreamReader(System.in));
        StringBuilder buffer = new StringBuilder(1024);
        
        String[] testPrompts = {
            "a short string (e.g., 'hello world'):",
            "an empty string (just press Enter):",
            "a string with exactly 1023 'a' characters:",
            "a string longer than 1023 'b' characters (e.g., 1030 'b's):",
            "a string with special characters and spaces (e.g., '  !@#$%^&*()  '):"
        };
        
        System.out.println("Running 5 test cases. Please provide input for each.");

        for (int i = 0; i < 5; i++) {
            System.out.println("\n--- Test Case " + (i + 1) + " ---");
            System.out.println("Please enter " + testPrompts[i]);
            
            int charsRead = scan(reader, buffer);
            
            if (charsRead >= 0) {
                System.out.println("Function returned (chars read): " + charsRead);
                System.out.println("Buffer content: \"" + buffer.toString() + "\"");
                System.out.println("Buffer content length: " + buffer.length());
            } else {
                System.err.println("Test case failed due to an error.");
                break;
            }
        }
        
        try {
            reader.close();
        } catch (IOException e) {
            System.err.println("Failed to close reader: " + e.getMessage());
        }
    }
}