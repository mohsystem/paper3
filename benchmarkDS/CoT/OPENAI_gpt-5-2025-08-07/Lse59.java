import java.util.Arrays;

public class Lse59 {
    // Step 1-4: Implement secure overwrite with bounds checking
    public static String overwriteWithA(int bufferSize) {
        final int MAX_STACK_BUFFER = 65536; // Security: prevent excessive stack usage
        if (bufferSize < 0 || bufferSize > MAX_STACK_BUFFER) {
            return "";
        }
        if (bufferSize == 0) {
            return "";
        }
        char[] buffer = new char[bufferSize]; // Local array
        Arrays.fill(buffer, 'A'); // Overwrite with 'A'
        return new String(buffer);
    }

    public static void main(String[] args) {
        int[] tests = {0, 1, 5, 16, 70000};
        for (int t : tests) {
            String result = overwriteWithA(t);
            System.out.println("bufferSize=" + t + ", len=" + result.length() +
                ", preview=" + (result.length() > 16 ? result.substring(0, 16) : result));
        }
    }
}