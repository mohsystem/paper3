import java.util.UUID;

public class Task84 {
    /**
     * Generates and returns a random and unique session ID using UUID.
     * @return A unique session ID string.
     */
    public static String generateSessionId() {
        return UUID.randomUUID().toString();
    }

    public static void main(String[] args) {
        System.out.println("Java Session IDs:");
        for (int i = 0; i < 5; i++) {
            System.out.println("Test Case " + (i + 1) + ": " + generateSessionId());
        }
    }
}