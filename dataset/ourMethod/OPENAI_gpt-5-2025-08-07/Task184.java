import java.security.SecureRandom;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;

public class Task184 {

    // Global registry ensuring uniqueness among existing robots
    private static final Set<String> REGISTRY = Collections.newSetFromMap(new ConcurrentHashMap<>());
    private static final SecureRandom RNG = new SecureRandom();
    private static final int MAX_NAMES = 26 * 26 * 1000; // 676,000 total possible names

    public static final class Robot {
        private String name;

        public Robot() {
            this.name = null;
        }

        public synchronized String getName() {
            if (this.name == null) {
                this.name = acquireUniqueName();
            }
            return this.name;
        }

        public synchronized boolean reset() {
            if (this.name != null) {
                releaseName(this.name);
                this.name = null;
                return true;
            }
            return false;
        }
    }

    // Generate a random candidate name without checking uniqueness
    private static String generateCandidateName() {
        char[] arr = new char[5];
        arr[0] = (char) ('A' + RNG.nextInt(26));
        arr[1] = (char) ('A' + RNG.nextInt(26));
        arr[2] = (char) ('0' + RNG.nextInt(10));
        arr[3] = (char) ('0' + RNG.nextInt(10));
        arr[4] = (char) ('0' + RNG.nextInt(10));
        return new String(arr);
    }

    private static boolean validateNameFormat(String name) {
        if (name == null || name.length() != 5) return false;
        for (int i = 0; i < 2; i++) {
            char c = name.charAt(i);
            if (c < 'A' || c > 'Z') return false;
        }
        for (int i = 2; i < 5; i++) {
            char c = name.charAt(i);
            if (c < '0' || c > '9') return false;
        }
        return true;
    }

    private static String acquireUniqueName() {
        if (REGISTRY.size() >= MAX_NAMES) {
            throw new IllegalStateException("Name space exhausted: cannot allocate more unique names");
        }
        // Attempt until success; collision probability is low with SecureRandom
        for (int attempts = 0; attempts < MAX_NAMES * 2; attempts++) {
            String candidate = generateCandidateName();
            if (!validateNameFormat(candidate)) continue;
            if (REGISTRY.add(candidate)) {
                return candidate;
            }
        }
        throw new IllegalStateException("Unable to acquire a unique name after many attempts");
    }

    private static boolean releaseName(String name) {
        if (name == null) return false;
        return REGISTRY.remove(name);
    }

    // Test helpers
    private static boolean isUnique(List<String> names) {
        Set<String> s = new HashSet<>(names);
        return s.size() == names.size();
    }

    // Tests
    private static boolean test1_singleRobotNamePattern() {
        Robot r = new Robot();
        String n = r.getName();
        return validateNameFormat(n);
    }

    private static boolean test2_multipleRobotsUnique() {
        List<Robot> robots = new ArrayList<>();
        for (int i = 0; i < 10; i++) robots.add(new Robot());
        List<String> names = new ArrayList<>();
        for (Robot r : robots) names.add(r.getName());
        return isUnique(names);
    }

    private static boolean test3_resetChangesNameAndIsUnique() {
        Robot r1 = new Robot();
        Robot r2 = new Robot();
        String n1 = r1.getName();
        String n2 = r2.getName();
        boolean beforeUnique = !n1.equals(n2);
        r1.reset();
        String n1b = r1.getName();
        boolean changed = !n1.equals(n1b);
        boolean stillUnique = !n1b.equals(r2.getName());
        return beforeUnique && changed && stillUnique;
    }

    private static boolean test4_repeatedResetsYieldDifferentNames() {
        Robot r = new Robot();
        Set<String> seen = new HashSet<>();
        for (int i = 0; i < 6; i++) {
            String n = r.getName();
            if (seen.contains(n)) return false;
            seen.add(n);
            r.reset();
        }
        return true;
    }

    private static boolean test5_stressUniqueness() {
        int N = 500;
        List<Robot> robots = new ArrayList<>();
        for (int i = 0; i < N; i++) robots.add(new Robot());
        List<String> names = new ArrayList<>(N);
        for (Robot r : robots) names.add(r.getName());
        return isUnique(names);
    }

    public static void main(String[] args) {
        System.out.println("Test1 pattern: " + test1_singleRobotNamePattern());
        System.out.println("Test2 uniqueness among 10: " + test2_multipleRobotsUnique());
        System.out.println("Test3 reset changes and unique: " + test3_resetChangesNameAndIsUnique());
        System.out.println("Test4 repeated resets yield different names: " + test4_repeatedResetsYieldDifferentNames());
        System.out.println("Test5 stress uniqueness among 500: " + test5_stressUniqueness());
    }
}