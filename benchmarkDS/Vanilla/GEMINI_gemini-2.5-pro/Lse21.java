public class Lse21 {

    /**
     * Adds 1000 to the given number.
     *
     * @param number The integer to which 1000 will be added.
     * @return The sum of the number and 1000.
     */
    public static int addThousand(int number) {
        return number + 1000;
    }

    /**
     * Main method. If a command-line argument is provided, it adds 1000 to it and prints the result.
     * Otherwise, it runs 5 hardcoded test cases.
     *
     * @param args Command-line arguments. Expects one integer argument.
     */
    public static void main(String[] args) {
        if (args.length > 0) {
            try {
                int inputVal = Integer.parseInt(args[0]);
                int result = addThousand(inputVal);
                System.out.println(result);
            } catch (NumberFormatException e) {
                System.err.println("Error: The provided argument is not a valid integer.");
                // Exiting with a non-zero status code to indicate an error.
                System.exit(1);
            }
        } else {
            System.out.println("No command line argument provided. Running test cases:");
            // 5 test cases
            int[] testValues = {5, -10, 0, 999, -2000};
            for (int val : testValues) {
                int result = addThousand(val);
                System.out.println("Input: " + val + ", Output: " + result);
            }
        }
        // The program returns 0 by default on successful completion.
    }
}