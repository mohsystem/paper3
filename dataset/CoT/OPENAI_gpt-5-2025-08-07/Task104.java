import java.util.Arrays;

public class Task104 {
    // Chain-of-Through: 1) Problem understanding: Safely copy user input into a fixed-size buffer.
    // Chain-of-Through: 2) Security requirements: Avoid overflows, handle null input, negative sizes.
    // Chain-of-Through: 3) Secure coding generation: Clamp sizes, avoid exceptions, safe substring.
    // Chain-of-Through: 4) Code review: Ensure no risky operations; no external input; safe printing.
    // Chain-of-Through: 5) Secure code output: Final safe implementation.

    public static String handleInputToFixedBuffer(String input, int bufferSize) {
        // Treat bufferSize as capacity (number of characters storable).
        if (bufferSize < 0) bufferSize = 0;
        if (input == null) input = "";
        int len = Math.min(bufferSize, input.length());
        return input.substring(0, len);
    }

    public static void main(String[] args) {
        String[] inputs = {"Hello", "HelloWorld", "", "SafeBuffer123", "EdgeCase"};
        int[] sizes = {10, 5, 8, 12, 0};

        for (int i = 0; i < inputs.length; i++) {
            String result = handleInputToFixedBuffer(inputs[i], sizes[i]);
            System.out.println("Test " + (i + 1) + ": input=\"" + inputs[i] + "\", size=" + sizes[i] + " -> \"" + result + "\"");
        }
    }
}