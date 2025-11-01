import java.util.Arrays;

public class Lse59 {

    public static char[] overwriteBuffer(int buffer_size) {
        if (buffer_size < 0) {
            return new char[0];
        }
        char[] buffer = new char[buffer_size];
        Arrays.fill(buffer, 'A');
        return buffer;
    }

    public static void main(String[] args) {
        // Test Case 1
        int size1 = 10;
        System.out.println("Test Case 1 (size=" + size1 + "): " + new String(overwriteBuffer(size1)));

        // Test Case 2
        int size2 = 5;
        System.out.println("Test Case 2 (size=" + size2 + "): " + new String(overwriteBuffer(size2)));

        // Test Case 3
        int size3 = 20;
        System.out.println("Test Case 3 (size=" + size3 + "): " + new String(overwriteBuffer(size3)));

        // Test Case 4
        int size4 = 1;
        System.out.println("Test Case 4 (size=" + size4 + "): " + new String(overwriteBuffer(size4)));

        // Test Case 5
        int size5 = 0;
        System.out.println("Test Case 5 (size=" + size5 + "): " + new String(overwriteBuffer(size5)));
    }
}