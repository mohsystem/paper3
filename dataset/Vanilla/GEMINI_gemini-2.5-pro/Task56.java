import java.util.UUID;

public class Task56 {

    /**
     * Generates a unique authentication token using UUID.
     * @return A String representing the token.
     */
    public static String generateToken() {
        // UUIDs are 128-bit values that are highly unlikely to be repeated,
        // making them suitable for simple session tokens.
        return UUID.randomUUID().toString();
    }

    public static void main(String[] args) {
        System.out.println("Java: Generating 5 authentication tokens.");
        for (int i = 0; i < 5; i++) {
            String token = generateToken();
            System.out.println("Test Case " + (i + 1) + ": " + token);
        }
    }
}