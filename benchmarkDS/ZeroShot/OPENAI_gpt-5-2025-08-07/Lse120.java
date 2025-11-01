import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Lse120 {
    public static String copyAndPrint(String input) {
        if (input == null) {
            input = "";
        }
        byte[] src = input.getBytes(StandardCharsets.UTF_8);
        byte[] dst = new byte[src.length];
        System.arraycopy(src, 0, dst, 0, src.length);
        String output = new String(dst, StandardCharsets.UTF_8);
        System.out.println(output);
        return output;
    }

    private static String repeatChar(char c, int count) {
        char[] arr = new char[count];
        Arrays.fill(arr, c);
        return new String(arr);
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "",
            "Hello, World!",
            "Secure Copy 123!@#",
            "こんにちは",
            repeatChar('a', 100)
        };
        for (String t : tests) {
            copyAndPrint(t);
        }
    }
}