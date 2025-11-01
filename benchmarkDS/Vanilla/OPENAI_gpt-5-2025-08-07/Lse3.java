import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Lse3 {
    public static byte[] copyBytes(byte[] dest, byte[] src, int n) {
        for (int i = 0; i < n; i++) {
            dest[i] = src[i];
        }
        return dest;
    }

    public static void main(String[] args) {
        byte[] src = "Hello".getBytes(StandardCharsets.UTF_8);

        byte[] dest1 = new byte[10];
        copyBytes(dest1, src, 5);
        System.out.println("Test1: " + new String(dest1, 0, 5, StandardCharsets.UTF_8));

        byte[] dest2 = new byte[5];
        Arrays.fill(dest2, (byte) 'x');
        copyBytes(dest2, src, 5);
        System.out.println("Test2: " + new String(dest2, 0, 5, StandardCharsets.UTF_8));

        byte[] dest3 = new byte[6];
        copyBytes(dest3, src, 5);
        System.out.println("Test3: " + new String(dest3, 0, 5, StandardCharsets.UTF_8));

        byte[] dest4 = new byte[100];
        copyBytes(dest4, src, 5);
        System.out.println("Test4: " + new String(dest4, 0, 5, StandardCharsets.UTF_8));

        byte[] dest5 = new byte[5];
        copyBytes(dest5, src, 5);
        System.out.println("Test5: " + new String(dest5, 0, 5, StandardCharsets.UTF_8));
    }
}