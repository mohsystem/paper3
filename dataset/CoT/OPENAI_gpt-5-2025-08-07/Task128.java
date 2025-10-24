// Chain-of-Through Process:
// 1) Problem understanding: Implement utilities to generate random numbers and secure tokens.
// 2) Security requirements: Use cryptographically secure PRNG (SecureRandom). Validate inputs to prevent errors.
// 3) Secure coding generation: Avoid predictable RNGs, validate ranges, handle exceptions, and avoid modulo bias where needed.
// 4) Code review: Ensure no insecure PRNGs, proper bounds checks, no unchecked exceptions in main.
// 5) Secure code output: Final code uses SecureRandom, validates inputs, and demonstrates 5 test cases.

import java.security.SecureRandom;
import java.util.Base64;
import java.util.Arrays;

public class Task128 {
    private static final SecureRandom SECURE_RANDOM = new SecureRandom();

    // Generate an array of cryptographically-secure random integers in the inclusive range [min, max].
    public static int[] generateRandomNumbers(int count, int min, int max) {
        if (count < 0) {
            throw new IllegalArgumentException("count must be non-negative");
        }
        if (min > max) {
            throw new IllegalArgumentException("min must be <= max");
        }
        int[] result = new int[count];
        int bound = (max - min) + 1;
        for (int i = 0; i < count; i++) {
            // SecureRandom.nextInt(bound) is uniform across the bound
            int r = (bound > 0) ? SECURE_RANDOM.nextInt(bound) : 0; // if bound == 0, min == max
            result[i] = min + r;
        }
        return result;
    }

    // Generate a URL-safe Base64 token derived from cryptographically secure random bytes.
    public static String generateSecureToken(int byteLength) {
        if (byteLength <= 0) {
            throw new IllegalArgumentException("byteLength must be positive");
        }
        byte[] bytes = new byte[byteLength];
        SECURE_RANDOM.nextBytes(bytes);
        // URL-safe Base64 without padding
        String token = Base64.getUrlEncoder().withoutPadding().encodeToString(bytes);
        return token;
    }

    private static void printArray(int[] arr) {
        System.out.println(Arrays.toString(arr));
    }

    public static void main(String[] args) {
        // 5 test cases
        try {
            // 1) 5 numbers between 1 and 10
            int[] a = generateRandomNumbers(5, 1, 10);
            System.out.println("Test1 numbers [1,10]:");
            printArray(a);

            // 2) Secure token with 16 bytes
            String t1 = generateSecureToken(16);
            System.out.println("Test2 token (16 bytes):");
            System.out.println(t1);

            // 3) 10 numbers between -50 and 50
            int[] b = generateRandomNumbers(10, -50, 50);
            System.out.println("Test3 numbers [-50,50]:");
            printArray(b);

            // 4) Secure token with 32 bytes
            String t2 = generateSecureToken(32);
            System.out.println("Test4 token (32 bytes):");
            System.out.println(t2);

            // 5) 3 numbers in a zero-width range [0,0]
            int[] c = generateRandomNumbers(3, 0, 0);
            System.out.println("Test5 numbers [0,0]:");
            printArray(c);
        } catch (IllegalArgumentException ex) {
            System.err.println("Input error: " + ex.getMessage());
        }
    }
}