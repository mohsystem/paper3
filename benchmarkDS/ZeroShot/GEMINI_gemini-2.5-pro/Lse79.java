import java.util.Scanner;

public class Lse79 {

    /**
     * Allocates two character buffers and securely reads up to 100 characters
     * from standard input into each.
     * @param scanner A Scanner instance to read from standard input.
     */
    public static void performAllocationAndRead(Scanner scanner) {
        // The prompt describes C-style memory management. In Java, we model this
        // by allocating arrays. Memory is managed automatically by the GC.
        
        // 1. Allocate a large buffer (1024 chars).
        char[] buf = new char[1024];
        
        // 2. Read a line from stdin for the first buffer.
        System.out.print("Enter text for the first buffer: ");
        String line1 = "";
        if (scanner.hasNextLine()) {
            line1 = scanner.nextLine();
        }

        // 3. Securely "copy in 100 characters" by truncating if the input is longer.
        String content1 = line1.length() > 100 ? line1.substring(0, 100) : line1;
        // Copy the processed string into our buffer.
        content1.getChars(0, content1.length(), buf, 0);

        // 4. Allocate the second buffer.
        char[] buf1 = new char[1024];

        // 5. Read and copy into the second buffer.
        System.out.print("Enter text for the second buffer: ");
        String line2 = "";
        if (scanner.hasNextLine()) {
            line2 = scanner.nextLine();
        }
        String content2 = line2.length() > 100 ? line2.substring(0, 100) : line2;
        content2.getChars(0, content2.length(), buf1, 0);

        // Print the contents read into the buffers.
        System.out.println("\n--- Contents ---");
        // Create new strings from the buffers, respecting the actual content length.
        System.out.println("First buffer: " + new String(buf, 0, content1.length()));
        System.out.println("Second buffer: " + new String(buf1, 0, content2.length()));
        System.out.println("----------------\n");
    }

    public static void main(String[] args) {
        // Use a single Scanner for the lifetime of the application to wrap System.in.
        // The try-with-resources statement ensures the scanner is closed automatically.
        try (Scanner scanner = new Scanner(System.in)) {
            for (int i = 1; i <= 5; i++) {
                System.out.println("--- Test Case " + i + " ---");
                performAllocationAndRead(scanner);
            }
        }
    }
}