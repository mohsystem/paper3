import java.io.*;
import java.util.*;

public class Lse60 {
    private static final int MAX_SIZE = 1_048_576; // 1 MiB upper bound for safety

    // Allocates a zero-initialized buffer of the given size and returns its contents as a hex string
    public static String allocateAndGetHex(int size) throws IllegalArgumentException {
        if (size < 0) {
            throw new IllegalArgumentException("Size must be non-negative.");
        }
        if (size > MAX_SIZE) {
            throw new IllegalArgumentException("Requested size exceeds safe limit.");
        }
        byte[] buffer = new byte[size]; // zero-initialized by default
        final char[] HEX = "0123456789abcdef".toCharArray();
        StringBuilder sb = new StringBuilder(size * 2);
        for (byte b : buffer) {
            int v = b & 0xFF;
            sb.append(HEX[v >>> 4]);
            sb.append(HEX[v & 0x0F]);
        }
        return sb.toString();
    }

    private static Integer parseSizeArg(String arg) {
        try {
            long val = Long.parseLong(arg);
            if (val < 0 || val > Integer.MAX_VALUE) return null;
            return (int) val;
        } catch (NumberFormatException e) {
            return null;
        }
    }

    public static void main(String[] args) {
        if (args.length == 1) {
            Integer size = parseSizeArg(args[0]);
            if (size == null) {
                System.err.println("Invalid size argument.");
                System.exit(1);
            }
            try {
                String out = allocateAndGetHex(size);
                System.out.println(out);
            } catch (IllegalArgumentException ex) {
                System.err.println(ex.getMessage());
                System.exit(1);
            }
            return;
        }
        // 5 test cases when no CLI arg is provided
        int[] tests = {0, 1, 8, 16, 32};
        for (int t : tests) {
            try {
                String out = allocateAndGetHex(t);
                System.out.println(out);
            } catch (IllegalArgumentException ex) {
                System.err.println("Test case failed for size " + t + ": " + ex.getMessage());
            }
        }
    }
}