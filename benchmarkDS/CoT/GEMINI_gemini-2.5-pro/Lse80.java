import java.util.Scanner;

public class Lse80 {

    /**
     * Reads a line from the given Scanner, populates the buffer up to a 
     * maximum of 1023 characters, and returns the number of characters read.
     * The newline character is consumed by scanner.nextLine() but not stored in the buffer.
     *
     * @param scanner The Scanner to read input from.
     * @param buffer A StringBuilder to store the read string.
     * @return The number of characters read and stored in the buffer.
     */
    public static int scan(Scanner scanner, StringBuilder buffer) {
        String line = "";
        if (scanner.hasNextLine()) {
            line = scanner.nextLine();
        }
        
        // Clear previous buffer content before use
        buffer.setLength(0);

        if (line.length() > 1023) {
            buffer.append(line.substring(0, 1023));
        } else {
            buffer.append(line);
        }
        return buffer.length();
    }

    public static void main(String[] args) {
        System.out.println("Running 5 test cases. Please provide input for each.");
        Scanner mainScanner = new Scanner(System.in);
        for (int i = 1; i <= 5; i++) {
            System.out.println("\n--- Test Case " + i + " ---");
            System.out.print("Enter a string: ");
            
            StringBuilder resultBuffer = new StringBuilder();
            int charsRead = scan(mainScanner, resultBuffer);

            System.out.println("String read: \"" + resultBuffer.toString() + "\"");
            System.out.println("Number of characters read: " + charsRead);
        }
        mainScanner.close();
    }
}