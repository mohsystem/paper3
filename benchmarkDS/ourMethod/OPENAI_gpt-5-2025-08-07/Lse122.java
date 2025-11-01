import java.util.Arrays;

public class Lse122 {
    // Allocates two chunks of size 10, copies first to second, frees both (via zeroing and nulling), returns a copy of second.
    public static byte[] allocateCopyFree(byte[] input) {
        final int SIZE = 10;

        byte[] chunk1 = new byte[SIZE]; // zero-initialized
        if (input != null) {
            int len = Math.min(input.length, SIZE);
            if (len > 0) {
                System.arraycopy(input, 0, chunk1, 0, len);
            }
        }

        byte[] chunk2 = new byte[SIZE];
        System.arraycopy(chunk1, 0, chunk2, 0, SIZE);

        // Detach a safe result copy before "freeing" chunks
        byte[] result = Arrays.copyOf(chunk2, SIZE);

        // Zeroize then drop references (simulate freeing in GC language)
        Arrays.fill(chunk1, (byte) 0);
        Arrays.fill(chunk2, (byte) 0);
        chunk1 = null;
        chunk2 = null;

        return result;
    }

    private static void printBytes(byte[] b) {
        StringBuilder sb = new StringBuilder();
        sb.append("[");
        for (int i = 0; i < b.length; i++) {
            if (i > 0) sb.append(", ");
            sb.append(b[i] & 0xFF);
        }
        sb.append("]");
        System.out.println(sb.toString());
    }

    public static void main(String[] args) {
        // 5 test cases
        byte[] t1 = allocateCopyFree(new byte[]{1, 2, 3});
        byte[] t2 = allocateCopyFree(null);
        byte[] t3 = allocateCopyFree(new byte[]{0, 1, 2, 3, 4, 5, 6, 7, 8, 9});
        byte[] t4 = allocateCopyFree(new byte[]{10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20}); // longer than 10
        byte[] t5 = allocateCopyFree(new byte[0]);

        printBytes(t1);
        printBytes(t2);
        printBytes(t3);
        printBytes(t4);
        printBytes(t5);
    }
}