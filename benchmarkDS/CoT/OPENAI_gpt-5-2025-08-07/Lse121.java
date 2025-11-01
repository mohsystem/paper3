import java.nio.charset.StandardCharsets;

public class Lse121 {
    public static byte[] copyAndManage(String input) {
        if (input == null) input = "";
        byte[] first = input.getBytes(StandardCharsets.UTF_8);
        byte[] second = new byte[first.length];
        System.arraycopy(first, 0, second, 0, first.length);
        // Free first chunk (eligible for GC)
        first = null;
        return second;
    }

    public static void main(String[] args) {
        String[] tests = {"Hello", "World!", "Memory Test", "12345", "Symbols!@#"};
        for (String s : tests) {
            byte[] second = copyAndManage(s);
            System.out.println(new String(second, StandardCharsets.UTF_8));
            System.out.println(new String(second, StandardCharsets.UTF_8));
            // Free second chunk (eligible for GC)
            second = null;
            System.gc();
        }
    }
}