import java.util.Arrays;

public class Lse59 {
    // Note: Java does not support explicit stack allocation for arrays; arrays are heap-allocated.
    public static char[] fillBuffer(int bufferSize) {
        int n = Math.max(0, bufferSize);
        char[] buf = new char[n];
        Arrays.fill(buf, 'A');
        return buf;
    }

    public static void main(String[] args) {
        int[] tests = {0, 1, 5, 10, 100};
        for (int t : tests) {
            char[] buf = fillBuffer(t);
            System.out.println("size=" + t + " -> \"" + new String(buf) + "\"");
        }
    }
}