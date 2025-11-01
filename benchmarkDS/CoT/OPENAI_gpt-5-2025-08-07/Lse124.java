public class Lse124 {
    public static String trimTrailing(String s) {
        return s.replaceAll("\\s+$", "");
    }

    // Function: accepts input as parameter and returns outputs
    public static String[] processInput(String input) {
        return new String[]{input, trimTrailing(input)};
    }

    // Command-line behavior as described
    public static void runWithArgs(String[] args) {
        if (args.length != 1) {
            System.err.println("Error: exactly one argument required.");
            System.exit(1);
        }
        String input = args[0];
        System.out.println(input);
        String trimmed = trimTrailing(input);
        System.out.println(trimmed);
        System.exit(0);
    }

    public static void main(String[] args) {
        // 5 test cases
        String[] tests = new String[]{
            "hello world   ",
            "   leading and trailing\t\t",
            "no-trailing",
            "tabs-and-newline\t\t\n",
            ""
        };
        for (int i = 0; i < tests.length; i++) {
            String[] res = processInput(tests[i]);
            System.out.println("TEST" + (i + 1) + " original: [" + res[0] + "]");
            System.out.println("TEST" + (i + 1) + " trimmed: [" + res[1] + "]");
        }
        // If any CLI args are provided, run the specified behavior
        if (args.length > 0) {
            runWithArgs(args);
        }
    }
}