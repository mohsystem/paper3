public class Lse122 {
    public static int doMemoryOps(byte[] data) {
        final int SIZE = 10;
        byte[] first = new byte[SIZE];
        byte[] second = new byte[SIZE];

        if (data != null) {
            int n = Math.min(SIZE, data.length);
            System.arraycopy(data, 0, first, 0, n);
        }

        System.arraycopy(first, 0, second, 0, SIZE);

        int sum = 0;
        for (int i = 0; i < SIZE; i++) {
            sum += (second[i] & 0xFF);
        }

        first = null;  // free first chunk (eligible for GC)
        second = null; // free second chunk (eligible for GC)

        return sum;
    }

    public static void main(String[] args) {
        byte[] t1 = new byte[] {0,1,2,3,4,5,6,7,8,9};
        System.out.println(doMemoryOps(t1)); // 45

        byte[] t2 = new byte[10];
        java.util.Arrays.fill(t2, (byte)1);
        System.out.println(doMemoryOps(t2)); // 10

        byte[] t3 = new byte[] {10,11,12,13,14,15,16,17,18,19};
        System.out.println(doMemoryOps(t3)); // 145

        byte[] t4 = new byte[10];
        java.util.Arrays.fill(t4, (byte)0xFF);
        System.out.println(doMemoryOps(t4)); // 2550

        byte[] t5 = new byte[] {5,4,3,2,1,0,(byte)250,(byte)251,(byte)252,(byte)253};
        System.out.println(doMemoryOps(t5)); // 1021
    }
}