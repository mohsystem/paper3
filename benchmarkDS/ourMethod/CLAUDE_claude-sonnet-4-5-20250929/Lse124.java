
public class Lse124 {
    public static void main(String[] args) {
        processArgument(args);
    }
    
    public static void processArgument(String[] args) {
        // First if statement: check that exactly one argument is provided
        if (args.length != 1) {
            System.err.println("Error: exactly one argument required");
            System.exit(1);
        }
        
        // Second line: get the argument from command line and store in variable called input
        // Input validation: check for null and reasonable length to prevent resource exhaustion
        String input = args[0];
        if (input == null) {
            System.err.println("Error: invalid argument");
            System.exit(1);
        }
        // Limit input size to prevent memory exhaustion (max 10KB)
        if (input.length() > 10240) {
            System.err.println("Error: argument too long");
            System.exit(1);
        }
        
        // Third line: print the argument
        // Use format string to prevent injection if input contains format specifiers
        System.out.println(input);
        
        // Fourth line: trim trailing whitespace from the argument
        // stripTrailing() removes only trailing whitespace (Java 11+)
        // For older Java versions, use replaceAll("\\\\s+$", "")
        input = input.stripTrailing();
        
        // Fifth line: print the argument again
        System.out.println(input);
        
        // Sixth line: exit the program
        System.exit(0);
    }
    
    // Test cases
    public static void testCase1() {
        System.out.println("Test 1: Normal input");
        processArgument(new String[]{"hello"});
    }
    
    public static void testCase2() {
        System.out.println("\\nTest 2: Input with trailing spaces");
        processArgument(new String[]{"hello   "});
    }
    
    public static void testCase3() {
        System.out.println("\\nTest 3: Input with trailing tabs");
        processArgument(new String[]{"world\\t\\t"});
    }
    
    public static void testCase4() {
        System.out.println("\\nTest 4: Input with mixed trailing whitespace");
        processArgument(new String[]{"test \\t \\n"});
    }
    
    public static void testCase5() {
        System.out.println("\\nTest 5: Empty string");
        processArgument(new String[]{""});
    }
}
