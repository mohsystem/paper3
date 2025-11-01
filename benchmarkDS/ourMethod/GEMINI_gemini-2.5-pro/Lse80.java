import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;

public class Lse80 {

    private static final int MAX_CHARS = 1023;

    /**
     * Reads a line from standard input, limiting its length.
     * The function reads a line until it sees a newline character.
     * The newline character is consumed but not included in the returned string.
     * The function will read at most 1023 characters from a single line.
     *
     * @return The string read from standard input, truncated to 1023 characters if necessary,
     *         or null if the end of the stream is reached.
     */
    public static String scan() {
        // A single reader for System.in is sufficient.
        // We do not close it as it would close the standard input stream.
        BufferedReader reader = new BufferedReader(new InputStreamReader(System.in));
        String line = null;
        try {
            line = reader.readLine();
        } catch (IOException e) {
            System.err.println("An I/O error occurred: " + e.getMessage());
            return null;
        }

        if (line == null) {
            return null; // End of stream
        }

        // Enforce the character limit by truncating if necessary.
        if (line.length() > MAX_CHARS) {
            return line.substring(0, MAX_CHARS);
        }
        
        return line;
    }

    public static void main(String[] args) {
        System.out.println("--- Running 5 Test Cases ---");
        for (int i = 1; i <= 5; i++) {
            System.out.println("\nTest Case " + i + ":");
            System.out.println("Please enter a line of text (or press Ctrl+D/Ctrl+Z for EOF):");
            
            String input = scan();

            if (input != null) {
                System.out.println("String read: \"" + input + "\"");
                System.out.println("Length: " + input.length());
            } else {
                System.out.println("End of input reached.");
                break; // Exit loop if EOF
            }
        }
        System.out.println("\n--- Tests Complete ---");
    }
}