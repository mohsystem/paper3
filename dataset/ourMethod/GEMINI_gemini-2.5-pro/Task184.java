import java.security.SecureRandom;
import java.util.HashSet;
import java.util.Set;

class Robot {
    private String name;
    private static final Set<String> usedNames = new HashSet<>();
    private static final SecureRandom random = new SecureRandom();
    private static final Object lock = new Object();

    public Robot() {
        this.name = null;
    }

    public String getName() {
        if (this.name == null) {
            this.name = generateUniqueName();
        }
        return this.name;
    }

    public void reset() {
        if (this.name != null) {
            synchronized (lock) {
                usedNames.remove(this.name);
            }
            this.name = null;
        }
    }

    private static String generateUniqueName() {
        while (true) {
            char letter1 = (char) ('A' + random.nextInt(26));
            char letter2 = (char) ('A' + random.nextInt(26));
            int number = random.nextInt(1000);
            String newName = String.format("%c%c%03d", letter1, letter2, number);

            synchronized (lock) {
                if (!usedNames.contains(newName)) {
                    usedNames.add(newName);
                    return newName;
                }
            }
        }
    }
}

public class Task184 {
    public static void main(String[] args) {
        // Test Case 1: Create a robot and get its name
        Robot robot1 = new Robot();
        String name1 = robot1.getName();
        System.out.println("Test Case 1: Robot 1's first name: " + name1);

        // Test Case 2: Get the same robot's name again, should be the same
        String name1_again = robot1.getName();
        System.out.println("Test Case 2: Robot 1's name again: " + name1_again);
        System.out.println("Name is consistent: " + name1.equals(name1_again));

        // Test Case 3: Create a second robot, its name should be different
        Robot robot2 = new Robot();
        String name2 = robot2.getName();
        System.out.println("Test Case 3: Robot 2's name: " + name2);
        System.out.println("Names are unique: " + !name1.equals(name2));

        // Test Case 4: Reset the first robot and get its new name
        System.out.println("Test Case 4: Resetting Robot 1...");
        robot1.reset();
        String newName1 = robot1.getName();
        System.out.println("Robot 1's new name: " + newName1);
        System.out.println("New name is different from old name: " + !name1.equals(newName1));
        System.out.println("New name is unique from Robot 2: " + !newName1.equals(name2));

        // Test Case 5: Create more robots
        System.out.println("Test Case 5: Creating more robots...");
        Robot robot3 = new Robot();
        Robot robot4 = new Robot();
        System.out.println("Robot 3's name: " + robot3.getName());
        System.out.println("Robot 4's name: " + robot4.getName());
    }
}