import java.util.Arrays;

public class Task104 {
    private static final int BUFFER_SIZE = 64;

    /**
     * Safely copies an input string into a fixed-size character buffer.
     * If the input is longer than the buffer, it is truncated.
     * The buffer is padded with null characters.
     *
     * @param input The string to be copied into the buffer.
     * @return A char array representing the buffer with the copied content.
     */
    public static char[] handleInput(String input) {
        char[] buffer = new char[BUFFER_SIZE];
        // Initialize buffer with null characters, similar to C-style buffers.
        Arrays.fill(buffer, '\0');

        if (input == null) {
            return buffer; // Return empty buffer for null input
        }

        // Determine the number of characters to copy, preventing overflow.
        int lengthToCopy = Math.min(input.length(), BUFFER_SIZE);

        // Safely copy characters from the input string to the buffer.
        input.getChars(0, lengthToCopy, buffer, 0);

        return buffer;
    }

    public static void main(String[] args) {
        String[] testCases = {
            "This is a short input.",
            "This is a very long input string that is definitely going to be longer than the sixty-four character buffer we have defined, thus causing truncation.",
            "",
            "This input string is exactly 64 characters long to test boundary.",
            null
        };

        for (int i = 0; i < testCases.length; i++) {
            System.out.println("Test Case #" + (i + 1));
            System.out.println("Input: " + (testCases[i] == null ? "null" : "\"" + testCases[i] + "\""));

            char[] resultBuffer = handleInput(testCases[i]);

            // Find the first null character to correctly represent the C-style string length.
            int firstNull = -1;
            for (int j = 0; j < resultBuffer.length; j++) {
                if (resultBuffer[j] == '\0') {
                    firstNull = j;
                    break;
                }
            }

            String resultString = (firstNull != -1)
                ? new String(resultBuffer, 0, firstNull)
                : new String(resultBuffer);

            System.out.println("Buffer Content: \"" + resultString + "\"");
            System.out.println("Buffer Size: " + resultBuffer.length);
            System.out.println("--------------------");
        }
    }
}