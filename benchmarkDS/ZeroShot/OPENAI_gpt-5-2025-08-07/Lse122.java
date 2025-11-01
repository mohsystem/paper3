import java.util.Arrays;

public class Lse122 {

    public static byte[] copyTwoChunks(byte[] src) {
        if (src == null || src.length != 10) {
            throw new IllegalArgumentException("Source must be non-null and length 10.");
        }

        byte[] chunk1 = new byte[10];
        byte[] chunk2 = new byte[10];

        System.arraycopy(src, 0, chunk1, 0, 10);
        System.arraycopy(chunk1, 0, chunk2, 0, 10);

        byte[] out = Arrays.copyOf(chunk2, 10);

        // Securely clear and release references (simulate free)
        Arrays.fill(chunk1, (byte) 0);
        Arrays.fill(chunk2, (byte) 0);
        chunk1 = null;
        chunk2 = null;

        return out;
    }

    private static void printBytes(byte[] a) {
        StringBuilder sb = new StringBuilder();
        sb.append("[");
        for (int i = 0; i < a.length; i++) {
            if (i > 0) sb.append(", ");
            sb.append(Byte.toUnsignedInt(a[i]));
        }
        sb.append("]");
        System.out.println(sb.toString());
    }

    public static void main(String[] args) {
        byte[][] tests = new byte[5][];
        tests[0] = new byte[]{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        tests[1] = new byte[]{10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
        tests[2] = new byte[]{(byte) 255, (byte) 254, (byte) 253, (byte) 252, (byte) 251, (byte) 250, (byte) 249, (byte) 248, (byte) 247, (byte) 246};
        tests[3] = new byte[]{42, 42, 42, 42, 42, 42, 42, 42, 42, 42};
        tests[4] = new byte[]{9, 8, 7, 6, 5, 4, 3, 2, 1, 0};

        for (byte[] t : tests) {
            byte[] res = copyTwoChunks(t);
            printBytes(res);
        }
    }
}