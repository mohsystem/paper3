import java.nio.charset.StandardCharsets;

public class Lse120 {
    public static String copyAndPrint(String input) {
        byte[] src = input.getBytes(StandardCharsets.UTF_8);
        byte[] chunk1 = new byte[src.length];
        System.arraycopy(src, 0, chunk1, 0, src.length);

        byte[] chunk2 = new byte[chunk1.length];
        System.arraycopy(chunk1, 0, chunk2, 0, chunk1.length);

        String out = new String(chunk2, StandardCharsets.UTF_8);
        System.out.println(out);
        return out;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "Hello",
            "",
            "Memory test 123",
            "こんにちは",
            "Emoji 😀🚀"
        };
        for (String t : tests) {
            String r = copyAndPrint(t);
            System.out.println("Returned: " + r);
        }
    }
}