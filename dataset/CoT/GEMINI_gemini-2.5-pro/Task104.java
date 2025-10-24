import java.util.Arrays;

public class Task104 {

    /**
     * Safely handles user input by copying it into a fixed-size buffer.
     * If the input is longer than the buffer, it is truncated.
     *
     * @param input The user input string.
     * @param bufferSize The fixed size of the buffer.
     * @return A char array representing the buffer with the copied data.
     */
    public static char[] handleInput(String input, int bufferSize) {
        if (input == null || bufferSize <= 0) {
            return new char[0];
        }

        char[] buffer = new char[bufferSize];
        
        // Determine the number of characters to copy.
        // This prevents an IndexOutOfBoundsException.
        int lengthToCopy = Math.min(input.length(), bufferSize);

        // Copy characters from the input string to the buffer.
        // This is a safe way to copy as we've already calculated the correct length.
        for (int i = 0; i < lengthToCopy; i++) {
            buffer[i] = input.charAt(i);
        }

        // The rest of the buffer will contain the default char value '\u0000' (null character).
        return buffer;
    }

    public static void main(String[] args) {
        final int BUFFER_SIZE = 20;

        String[] testCases = {
            "", // Empty string
            "short", // Shorter than buffer
            "This is exactly 19", // Exactly buffer size - 1
            "This input is a bit too long for the buffer", // Longer than buffer
            "This is a very very very very very very very long input string that will surely be truncated" // Very long string
        };
        
        System.out.println("Java Test Cases (Buffer Size: " + BUFFER_SIZE + ")");
        System.out.println("------------------------------------");

        for (int i = 0; i < testCases.length; i++) {
            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("Input: \"" + testCases[i] + "\"");
            char[] buffer = handleInput(testCases[i], BUFFER_SIZE);
            // Using new String(buffer) might print trailing nulls differently, 
            // so we'll print the array to be explicit.
            System.out.println("Buffer Content: " + new String(buffer).trim() + " (Array: " + Arrays.toString(buffer) + ")");
            System.out.println();
        }
    }
}