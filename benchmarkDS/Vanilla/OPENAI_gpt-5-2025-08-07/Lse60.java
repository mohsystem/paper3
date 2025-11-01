import java.util.*;

public class Lse60 {
    public static String allocateAndDescribe(int size) {
        if (size < 0) size = 0;
        byte[] buffer = new byte[size]; // zero-initialized
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < buffer.length; i++) {
            if (i > 0) sb.append(' ');
            sb.append(String.format("%02X", buffer[i] & 0xFF));
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        if (args.length == 1) {
            int size;
            try {
                size = Integer.parseInt(args[0]);
            } catch (NumberFormatException e) {
                size = 0;
            }
            System.out.println(allocateAndDescribe(size));
        } else {
            int[] tests = {0, 1, 3, 8, 16};
            for (int t : tests) {
                System.out.println(allocateAndDescribe(t));
            }
        }
    }
}