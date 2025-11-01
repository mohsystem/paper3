import java.util.Scanner;

public class Lse80 {

    /**
     * Reads a line from the provided Scanner, truncates it to 1023 characters,
     * places it into the given StringBuilder, and returns the number of characters read.
     * The newline character is consumed but not stored.
     *
     * @param buffer The StringBuilder to store the read string.
     * @param scanner The Scanner to read from (e.g., from System.in).
     * @return The number of characters read into the buffer.
     */
    public static int scan(StringBuilder buffer, Scanner scanner) {
        if (scanner.hasNextLine()) {
            String line = scanner.nextLine();
            String truncatedLine = (line.length() > 1023) ? line.substring(0, 1023) : line;
            
            buffer.setLength(0); // Clear the buffer
            buffer.append(truncatedLine);
            
            return truncatedLine.length();
        }
        return 0; // No line read
    }

    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);
        StringBuilder buffer = new StringBuilder(1024);

        System.out.println("Running 5 test cases for Java scan function.");

        for (int i = 1; i <= 5; i++) {
            System.out.println("\n--- Test Case " + i + " ---");
            System.out.print("Please enter a string: ");
            
            int charsRead = scan(buffer, scanner);
            
            System.out.println("Function returned (characters read): " + charsRead);
            System.out.println("Buffer content: \"" + buffer.toString() + "\"");
            System.out.println("Buffer length: " + buffer.length());
        }
        
        scanner.close();
    }
}