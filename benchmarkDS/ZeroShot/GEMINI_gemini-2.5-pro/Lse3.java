import java.nio.charset.StandardCharsets;

public class Lse3 {

    /**
     * Securely copies a number of bytes from a source byte array to a destination byte array.
     *
     * @param dest The destination byte array.
     * @param src The source byte array.
     * @param numBytes The number of bytes to copy.
     * @return true if the copy was successful, false otherwise.
     */
    public static boolean copyBytes(byte[] dest, byte[] src, int numBytes) {
        // 1. Check for null arrays
        if (dest == null || src == null) {
            System.err.println("Error: Destination or source array is null.");
            return false;
        }

        // 2. Check for a negative number of bytes to copy
        if (numBytes < 0) {
            System.err.println("Error: Number of bytes to copy cannot be negative.");
            return false;
        }
        
        // A zero-byte copy is a no-op and is considered a success.
        if (numBytes == 0) {
            return true;
        }

        // 3. Check for potential buffer overflow on destination
        if (numBytes > dest.length) {
            System.err.println("Error: Buffer overflow detected. Cannot copy " + numBytes + " bytes into a destination of size " + dest.length);
            return false;
        }
        
        // 4. Check for potential out-of-bounds read on source
        if (numBytes > src.length) {
            System.err.println("Error: Source read out of bounds. Cannot copy " + numBytes + " bytes from a source of size " + src.length);
            return false;
        }

        // Use System.arraycopy, which is an efficient and safe native implementation.
        // It handles overlapping regions correctly if src and dest are the same array.
        System.arraycopy(src, 0, dest, 0, numBytes);
        return true;
    }

    public static void main(String[] args) {
        System.out.println("--- Java Test Cases ---");

        String sourceString = "Hello";
        // Get bytes including the null terminator for C-style string compatibility
        byte[] sourceBytes = (sourceString + "\0").getBytes(StandardCharsets.UTF_8); 
        int bytesToCopy = sourceBytes.length; // 6 bytes for "Hello\0"

        // Test Case 1: Normal copy
        System.out.println("\n--- Test Case 1: Normal copy ---");
        byte[] dest1 = new byte[10];
        if (copyBytes(dest1, sourceBytes, bytesToCopy)) {
            // -1 to exclude null terminator for printing
            System.out.println("Success! Copied string: \"" + new String(dest1, 0, bytesToCopy - 1, StandardCharsets.UTF_8) + "\"");
        } else {
            System.out.println("Failed!");
        }

        // Test Case 2: Exact size copy
        System.out.println("\n--- Test Case 2: Exact size copy ---");
        byte[] dest2 = new byte[bytesToCopy];
        if (copyBytes(dest2, sourceBytes, bytesToCopy)) {
            System.out.println("Success! Copied string: \"" + new String(dest2, 0, bytesToCopy - 1, StandardCharsets.UTF_8) + "\"");
        } else {
            System.out.println("Failed!");
        }

        // Test Case 3: Buffer overflow attempt
        System.out.println("\n--- Test Case 3: Buffer overflow attempt ---");
        byte[] dest3 = new byte[4];
        if (copyBytes(dest3, sourceBytes, bytesToCopy)) {
            System.out.println("Success! (This should not happen)");
        } else {
            System.out.println("Failed as expected.");
        }

        // Test Case 4: Null pointer
        System.out.println("\n--- Test Case 4: Null pointer ---");
        byte[] dest4 = null;
        if (copyBytes(dest4, sourceBytes, bytesToCopy)) {
            System.out.println("Success! (This should not happen)");
        } else {
            System.out.println("Failed as expected.");
        }

        // Test Case 5: Zero bytes copy
        System.out.println("\n--- Test Case 5: Zero bytes copy ---");
        byte[] dest5 = new byte[10];
        if (copyBytes(dest5, sourceBytes, 0)) {
            System.out.println("Success! Copied 0 bytes.");
        } else {
            System.out.println("Failed! (This should not happen)");
        }
    }
}