public class Lse61 {
    // Allocates a buffer of size bufferSize, then frees it in a loop.
    // Returns the number of times the buffer was actually freed (non-null to null transition).
    public static int allocateAndFreeInLoop(int bufferSize, int loops) {
        byte[] buffer = new byte[bufferSize];
        int frees = 0;
        for (int i = 0; i < loops; i++) {
            if (buffer != null) {
                buffer = null; // "Free" by releasing the reference
                frees++;
            }
            // Subsequent iterations do nothing since buffer is already null
        }
        return frees;
    }

    public static void main(String[] args) {
        int[] sizes = {0, 1, 16, 1024, 65536};
        int[] loops = {1, 3, 5, 2, 10};
        for (int i = 0; i < 5; i++) {
            int result = allocateAndFreeInLoop(sizes[i], loops[i]);
            System.out.println("Test " + (i + 1) + ": size=" + sizes[i] + ", loops=" + loops[i] + ", frees=" + result);
        }
    }
}