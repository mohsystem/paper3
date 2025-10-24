import java.security.SecureRandom;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

public class Task184 {

    static class RobotFactory {
        private static final SecureRandom RNG = new SecureRandom();
        private static final Set<String> ALLOCATED = Collections.synchronizedSet(new HashSet<>());
        private static final int CAPACITY = 26 * 26 * 1000;

        private static String randomName() {
            int l1 = RNG.nextInt(26);
            int l2 = RNG.nextInt(26);
            int num = RNG.nextInt(1000);
            return new StringBuilder(5)
                    .append((char) ('A' + l1))
                    .append((char) ('A' + l2))
                    .append(String.format("%03d", num))
                    .toString();
        }

        public static synchronized String generateUnique() {
            if (ALLOCATED.size() >= CAPACITY) {
                throw new IllegalStateException("All possible robot names have been allocated.");
            }
            String name;
            do {
                name = randomName();
            } while (ALLOCATED.contains(name));
            ALLOCATED.add(name);
            return name;
        }

        public static synchronized void release(String name) {
            if (name != null) {
                ALLOCATED.remove(name);
            }
        }
    }

    static class Robot {
        private String name;

        public synchronized String getName() {
            if (name == null) {
                name = RobotFactory.generateUnique();
            }
            return name;
        }

        public synchronized void reset() {
            if (name != null) {
                RobotFactory.release(name);
                name = null;
            }
        }
    }

    public static void main(String[] args) {
        Robot r1 = new Robot();
        Robot r2 = new Robot();
        Robot r3 = new Robot();
        System.out.println("R1: " + r1.getName());
        System.out.println("R2: " + r2.getName());
        System.out.println("R3: " + r3.getName());
        Robot r4 = new Robot();
        System.out.println("R4: " + r4.getName());
        r2.reset();
        System.out.println("R2 after reset: " + r2.getName());
    }
}