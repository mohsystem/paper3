import java.nio.charset.StandardCharsets;

public class Lse79 {
    public static String allocAndCopy100(String input) {
        byte[] buf = new byte[1024];
        byte[] src = input == null ? new byte[0] : input.getBytes(StandardCharsets.UTF_8);
        int len = Math.min(100, Math.min(src.length, 1024));
        System.arraycopy(src, 0, buf, 0, len);
        return new String(buf, 0, len, StandardCharsets.UTF_8);
    }

    public static String allocAndCopy100Second(String input) {
        byte[] buf1 = new byte[1024];
        byte[] src = input == null ? new byte[0] : input.getBytes(StandardCharsets.UTF_8);
        int len = Math.min(100, Math.min(src.length, 1024));
        System.arraycopy(src, 0, buf1, 0, len);
        return new String(buf1, 0, len, StandardCharsets.UTF_8);
    }

    public static String readUpTo1024(String input) {
        byte[] mem = new byte[1024];
        byte[] src = input == null ? new byte[0] : input.getBytes(StandardCharsets.UTF_8);
        int len = Math.min(src.length, 1024);
        System.arraycopy(src, 0, mem, 0, len);
        return new String(mem, 0, len, StandardCharsets.UTF_8);
    }

    public static void main(String[] args) {
        String s1 = "Hello World";
        String s2 = "A".repeat(100);
        String s3 = "B".repeat(150);
        String s4 = "";
        String s5 = "Unicode 😃🚀 " + "C".repeat(1100);

        String[] tests = {s1, s2, s3, s4, s5};

        for (int i = 0; i < tests.length; i++) {
            String t = tests[i];
            String r1 = allocAndCopy100(t);
            String r2 = allocAndCopy100Second(t);
            String r3 = readUpTo1024(t);
            System.out.println("Test " + (i + 1));
            System.out.println("allocAndCopy100: " + r1);
            System.out.println("allocAndCopy100Second: " + r2);
            System.out.println("readUpTo1024: " + r3.substring(0, Math.min(80, r3.length())) + (r3.length() > 80 ? "...(truncated print)" : ""));
            System.out.println("---");
        }
    }
}