public class Lse20 {
    private static final int OFFSET = 1000;

    /**
     * Adds a constant offset to the given integer value.
     * @param value The integer input.
     * @return The value with the offset added.
     */
    public static int addOffset(int value) {
        return value + OFFSET;
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println("--- Running 5 Test Cases ---");
        System.out.println("Input: 10, Output: " + addOffset(10));
        System.out.println("Input: 0, Output: " + addOffset(0));
        System.out.println("Input: -50, Output: " + addOffset(-50));
        System.out.println("Input: 2000, Output: " + addOffset(2000));
        System.out.println("Input: -1000, Output: " + addOffset(-1000));
        System.out.println("--------------------------\n");

        // Handling command line input
        if (args.length > 0) {
            try {
                int inputValue = Integer.parseInt(args[0]);
                int result = addOffset(inputValue);
                System.out.println("Command line input: " + inputValue);
                System.out.println("Result with offset: " + result);
            } catch (NumberFormatException e) {
                System.err.println("Error: The provided command line argument is not a valid integer.");
            }
        } else {
            System.out.println("Usage: java Lse20 <integer_value>");
        }
    }
}