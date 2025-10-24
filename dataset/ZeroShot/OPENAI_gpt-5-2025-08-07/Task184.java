import java.security.SecureRandom;
import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

public class Task184 {
    private static final SecureRandom RNG = new SecureRandom();
    private static final Set<String> ALLOCATED = ConcurrentHashMap.newKeySet();
    private static final int MAX_NAMES = 26 * 26 * 1000;

    public static class Robot {
        private String name;

        public synchronized String getName() {
            if (name == null) {
                name = newUniqueName();
            }
            return name;
        }

        public synchronized void reset() {
            name = null;
        }
    }

    private static String newUniqueName() {
        for (int attempts = 0; attempts < MAX_NAMES; attempts++) {
            String candidate = randomName();
            if (ALLOCATED.add(candidate)) {
                return candidate;
            }
        }
        throw new IllegalStateException("Exhausted all possible unique names");
    }

    private static String randomName() {
        char[] out = new char[5];
        out[0] = (char) ('A' + RNG.nextInt(26));
        out[1] = (char) ('A' + RNG.nextInt(26));
        out[2] = (char) ('0' + RNG.nextInt(10));
        out[3] = (char) ('0' + RNG.nextInt(10));
        out[4] = (char) ('0' + RNG.nextInt(10));
        return new String(out);
    }

    public static void main(String[] args) {
        Robot r1 = new Robot();
        Robot r2 = new Robot();
        Robot r3 = new Robot();

        String n1 = r1.getName();
        String n2 = r2.getName();
        String n3 = r3.getName();

        System.out.println(n1);
        System.out.println(n2);
        System.out.println(n3);

        String old2 = n2;
        r2.reset();
        String n2b = r2.getName();
        System.out.println(n2b);

        Robot r4 = new Robot();
        Robot r5 = new Robot();
        String n4 = r4.getName();
        String n5 = r5.getName();
        System.out.println(n4);
        System.out.println(n5);

        Set<String> uniq = new HashSet<>();
        uniq.add(n1);
        uniq.add(n2b);
        uniq.add(n3);
        uniq.add(n4);
        uniq.add(n5);
        if (uniq.size() != 5) {
            throw new IllegalStateException("Names are not unique");
        }
        if (old2.equals(n2b)) {
            throw new IllegalStateException("Reset did not change the name");
        }
    }
}