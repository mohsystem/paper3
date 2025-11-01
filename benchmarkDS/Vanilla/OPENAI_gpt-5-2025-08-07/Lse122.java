import java.util.Arrays;

public class Lse122 {
    public static int[] copyAndFree(int[] src) {
        if (src == null || src.length != 10) {
            throw new IllegalArgumentException("src must be length 10");
        }
        int[] first = new int[10];
        System.arraycopy(src, 0, first, 0, 10);

        int[] second = new int[10];
        System.arraycopy(first, 0, second, 0, 10);

        int[] result = Arrays.copyOf(second, 10);

        first = null; // simulate free
        second = null; // simulate free

        return result;
    }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
            new int[]{0,1,2,3,4,5,6,7,8,9},
            new int[]{10,11,12,13,14,15,16,17,18,19},
            new int[]{5,5,5,5,5,5,5,5,5,5},
            new int[]{-5,-4,-3,-2,-1,0,1,2,3,4},
            new int[]{42,0,-7,13,13,-1,2,2,999,-999}
        };

        for (int i = 0; i < tests.length; i++) {
            int[] out = copyAndFree(tests[i]);
            System.out.println("Test " + (i+1) + ": " + Arrays.toString(out));
        }
    }
}