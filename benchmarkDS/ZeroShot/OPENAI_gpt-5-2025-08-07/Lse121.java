import java.nio.charset.StandardCharsets;

public class Lse121 {
    public static String process(String input) {
        String safeInput = (input == null) ? "" : input;

        byte[] chunk1 = safeInput.getBytes(StandardCharsets.UTF_8);
        byte[] chunk2 = new byte[chunk1.length];

        System.arraycopy(chunk1, 0, chunk2, 0, chunk1.length);

        String printedOnce = new String(chunk2, StandardCharsets.UTF_8);
        System.out.println(printedOnce);

        // "Free" the first chunk (eligible for GC)
        chunk1 = null;

        String printedTwice = new String(chunk2, StandardCharsets.UTF_8);
        System.out.println(printedTwice);

        // "Free" the second chunk (eligible for GC)
        chunk2 = null;

        return printedOnce + "\n" + printedTwice + "\n";
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "Hello, World!",
            "",
            "UTF-8: こんにちは",
            "Numbers: 1234567890",
            "Line1\nLine2\tTabbed!"
        };

        for (String t : tests) {
            String out = process(t);
            System.out.println("Returned:");
            System.out.print(out);
        }
    }
}