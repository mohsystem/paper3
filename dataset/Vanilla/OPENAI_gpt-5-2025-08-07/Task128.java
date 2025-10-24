import java.util.*;
import java.security.SecureRandom;

public class Task128 {

    public static List<Integer> generateRandomIntList(int count, int minInclusive, int maxInclusive, Long seed) {
        if (count < 0) throw new IllegalArgumentException("count must be >= 0");
        if (minInclusive > maxInclusive) throw new IllegalArgumentException("minInclusive cannot be greater than maxInclusive");
        Random rng = (seed == null) ? new SecureRandom() : new Random(seed);
        List<Integer> result = new ArrayList<>(count);
        int bound = (maxInclusive - minInclusive) + 1;
        for (int i = 0; i < count; i++) {
            int val = minInclusive + rng.nextInt(bound);
            result.add(val);
        }
        return result;
    }

    public static String generateToken(int length, String allowedChars, Long seed) {
        if (length < 0) throw new IllegalArgumentException("length must be >= 0");
        String defaultChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
        String chars = (allowedChars == null || allowedChars.isEmpty()) ? defaultChars : allowedChars;
        Random rng = (seed == null) ? new SecureRandom() : new Random(seed);
        StringBuilder sb = new StringBuilder(length);
        for (int i = 0; i < length; i++) {
            int idx = rng.nextInt(chars.length());
            sb.append(chars.charAt(idx));
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        System.out.println("Java Test Case 1:");
        System.out.println(generateRandomIntList(10, 1, 100, 12345L));
        System.out.println(generateToken(16, null, 12345L));

        System.out.println("Java Test Case 2:");
        System.out.println(generateRandomIntList(5, 0, 9, null));
        System.out.println(generateToken(8, null, null));

        System.out.println("Java Test Case 3:");
        System.out.println(generateRandomIntList(7, -50, 50, 999L));
        System.out.println(generateToken(12, "ABCDEF0123", 42L));

        System.out.println("Java Test Case 4:");
        System.out.println(generateRandomIntList(3, 1000, 2000, 55555L));
        System.out.println(generateToken(24, "abcdef", 55555L));

        System.out.println("Java Test Case 5:");
        System.out.println(generateRandomIntList(0, 0, 10, 2021L));
        System.out.println("[" + generateToken(0, null, 2021L) + "]");
    }
}