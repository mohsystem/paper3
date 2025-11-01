import java.util.Arrays;
import java.nio.charset.StandardCharsets;

public class Lse120 {
    public static byte[] copyChunk(byte[] input) {
        byte[] chunk1 = Arrays.copyOf(input, input.length);
        byte[] chunk2 = new byte[chunk1.length];
        System.arraycopy(chunk1, 0, chunk2, 0, chunk1.length);
        return chunk2;
    }

    private static void printBytes(byte[] data) {
        System.out.println(Arrays.toString(data));
    }

    public static void main(String[] args) {
        byte[][] tests = new byte[5][];
        tests[0] = "Hello, World!".getBytes(StandardCharsets.UTF_8);
        tests[1] = "foo".getBytes(StandardCharsets.UTF_8);
        tests[2] = "The quick brown fox".getBytes(StandardCharsets.UTF_8);
        tests[3] = new byte[0];
        tests[4] = new byte[] {0, 1, 2, 3, 127, (byte)0xFF};

        for (int i = 0; i < tests.length; i++) {
            byte[] copied = copyChunk(tests[i]);
            printBytes(copied);
        }
    }
}