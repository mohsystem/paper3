import java.security.SecureRandom;
import java.util.HashSet;
import java.util.Objects;
import java.util.Set;

public class Task184 {
    // Chain-of-Through: 1) Problem understanding + 2) Security requirements
    // - Generate random robot names (AA000-AA999) with uniqueness among active robots.
    // - Ensure randomness is not predictable (use SecureRandom).
    // - Ensure thread safety and avoid global state manipulation issues.
    // - Release names on reset to allow future reuse when not held by any robot.

    // Registry that ensures uniqueness and secure name generation
    static final class NameRegistry {
        private static final Set<String> ACTIVE = new HashSet<>();
        private static final SecureRandom RNG = new SecureRandom();
        private static final int MAX_ATTEMPTS = 1_000_000;

        private NameRegistry() {}

        private static String randomName() {
            char[] buf = new char[5];
            buf[0] = (char) ('A' + RNG.nextInt(26));
            buf[1] = (char) ('A' + RNG.nextInt(26));
            buf[2] = (char) ('0' + RNG.nextInt(10));
            buf[3] = (char) ('0' + RNG.nextInt(10));
            buf[4] = (char) ('0' + RNG.nextInt(10));
            return new String(buf);
        }

        public static synchronized String acquireUniqueName() {
            for (int i = 0; i < MAX_ATTEMPTS; i++) {
                String candidate = randomName();
                if (!ACTIVE.contains(candidate)) {
                    ACTIVE.add(candidate);
                    return candidate;
                }
            }
            throw new IllegalStateException("Unable to generate a unique robot name after many attempts.");
        }

        public static synchronized void releaseName(String name) {
            if (name != null) {
                ACTIVE.remove(name);
            }
        }

        public static synchronized boolean isActive(String name) {
            return ACTIVE.contains(name);
        }
    }

    static final class Robot {
        private String name;

        public Robot() {
            // Initially unnamed
        }

        public synchronized String getName() {
            if (name == null) {
                name = NameRegistry.acquireUniqueName();
            }
            return name;
        }

        public synchronized void reset() {
            if (name != null) {
                NameRegistry.releaseName(name);
                name = null;
            }
        }
    }

    // Chain-of-Through: 3) Secure coding generation + 4) Code review integrated in comments
    // - All state modifications are synchronized to avoid race conditions.
    // - Using SecureRandom for unpredictable names.
    // - Ensuring release on reset to maintain uniqueness only among existing robots.

    // Demonstration with 5 test cases
    public static void main(String[] args) {
        // Test 1: First robot gets a valid random name
        Robot r1 = new Robot();
        String n1 = r1.getName();
        System.out.println("Test1 r1 name: " + n1 + " active=" + NameRegistry.isActive(n1));

        // Test 2: Second robot gets a different unique name
        Robot r2 = new Robot();
        String n2 = r2.getName();
        System.out.println("Test2 r2 name: " + n2 + " unique_vs_r1=" + !Objects.equals(n1, n2));

        // Test 3: getName is stable until reset
        String n2_again = r2.getName();
        System.out.println("Test3 r2 stable: " + n2 + " == " + n2_again + " -> " + Objects.equals(n2, n2_again));

        // Test 4: After reset, robot gets a new name and old is released
        r2.reset();
        String n2_new = r2.getName();
        System.out.println("Test4 r2 reset new name: " + n2_new + " old_different=" + !Objects.equals(n2, n2_new));

        // Test 5: Create many robots and verify uniqueness count
        int count = 1000;
        Set<String> seen = new HashSet<>();
        Robot[] robots = new Robot[count];
        boolean unique = true;
        for (int i = 0; i < count; i++) {
            robots[i] = new Robot();
            String nm = robots[i].getName();
            if (!seen.add(nm)) {
                unique = false;
                break;
            }
        }
        System.out.println("Test5 1000 robots unique=" + unique + " size=" + seen.size());
    }
}