import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Lse121 {

    private static final int MAX_SIZE = 1 << 20; // 1 MiB cap

    // Allocates two chunks, copies first to second, prints second, frees first, prints second, frees second.
    // Returns a copy of the second chunk's content captured before freeing it.
    public static String demoCopy(String input) {
        if (input == null) {
            System.out.println("Invalid input");
            return "";
        }

        byte[] src = input.getBytes(StandardCharsets.UTF_8);
        if (src.length > MAX_SIZE) {
            System.out.println("Input too large");
            return "";
        }

        // Allocate two chunks
        byte[] chunk1 = new byte[src.length];
        byte[] chunk2 = new byte[src.length];

        // Copy input -> chunk1
        System.arraycopy(src, 0, chunk1, 0, src.length);

        // Copy chunk1 -> chunk2 with bounds check
        if (chunk2.length < chunk1.length) {
            System.out.println("Allocation error");
            Arrays.fill(chunk1, (byte) 0);
            return "";
        }
        System.arraycopy(chunk1, 0, chunk2, 0, chunk1.length);

        // Print second chunk
        String secondAsString = new String(chunk2, StandardCharsets.UTF_8);
        System.out.println(secondAsString);

        // "Free" first chunk (zeroize then drop reference)
        Arrays.fill(chunk1, (byte) 0);
        chunk1 = null;

        // Print second chunk again (should remain unchanged)
        System.out.println(new String(chunk2, StandardCharsets.UTF_8));

        // Capture return value before freeing second
        String ret = new String(chunk2, StandardCharsets.UTF_8);

        // "Free" second chunk
        Arrays.fill(chunk2, (byte) 0);
        chunk2 = null;

        return ret;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "Hello, world!",
            "",
            "A longer string with multiple words.",
            "Unicode: 😊🚀✨",
            "Line1\nLine2\tTabbed"
        };
        for (String t : tests) {
            String result = demoCopy(t);
            System.out.println("Returned: " + result);
            System.out.println("---");
        }
    }
}