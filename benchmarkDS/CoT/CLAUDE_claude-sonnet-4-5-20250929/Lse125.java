
public class Lse125 {
    public static String processArgument(String arg) {
        if (arg == null) {
            return "";
        }
        // Trim trailing whitespace
        return arg.replaceAll("\\\\s+$", "");
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testCases = {
            "Hello World   ",
            "NoSpaces",
            "Trailing\\t\\t",
            "   LeadingAndTrailing   ",
            ""
        };
        
        System.out.println("Test cases:");
        for (int i = 0; i < testCases.length; i++) {
            String result = processArgument(testCases[i]);
            System.out.println("Test " + (i + 1) + ": [" + result + "]");
        }
        
        // Process command line argument if provided
        if (args.length > 0) {
            String trimmed = processArgument(args[0]);
            System.out.println("\\nCommand line argument: [" + trimmed + "]");
        } else {
            System.out.println("\\nNo command line argument provided");
        }
    }
}
