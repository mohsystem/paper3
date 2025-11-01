public class Lse19 {

    /**
     * Adds 1000 to the given integer.
     * @param number The input integer.
     * @return The input number plus 1000.
     */
    public static int addThousand(int number) {
        return number + 1000;
    }

    public static void main(String[] args) {
        // If a command-line argument is provided, process it.
        if (args.length > 0) {
            try {
                int inputNumber = Integer.parseInt(args[0]);
                int result = addThousand(inputNumber);
                System.out.println(result);
            } catch (NumberFormatException e) {
                System.err.println("Error: Input must be a valid integer.");
            }
        } else {
            // Otherwise, run the 5 built-in test cases.
            System.out.println("No command-line argument provided. Running test cases:");
            String[] testCases = {"10", "-50", "0", "12345", "-1000"};
            for (String test : testCases) {
                try {
                    int inputNumber = Integer.parseInt(test);
                    int result = addThousand(inputNumber);
                    System.out.println("Input: " + test + ", Output: " + result);
                } catch (NumberFormatException e) {
                     System.err.println("Error processing test case: " + test);
                }
            }
        }
    }
}