
import java.util.*;
import java.security.SecureRandom;

public class Task184 {
    private static final Set<String> usedNames = new HashSet<>();
    private static final SecureRandom random = new SecureRandom();
    private static final int MAX_NAMES = 26 * 26 * 1000; // 676,000 possible names
    
    private String name;
    
    public Task184() {
        this.name = generateUniqueName();
    }
    
    private static synchronized String generateUniqueName() {
        if (usedNames.size() >= MAX_NAMES) {
            throw new RuntimeException("All possible robot names have been exhausted");
        }
        
        String newName;
        do {
            newName = generateRandomName();
        } while (usedNames.contains(newName));
        
        usedNames.add(newName);
        return newName;
    }
    
    private static String generateRandomName() {
        char letter1 = (char) ('A' + random.nextInt(26));
        char letter2 = (char) ('A' + random.nextInt(26));
        int number = random.nextInt(1000);
        return String.format("%c%c%03d", letter1, letter2, number);
    }
    
    public String getName() {
        return name;
    }
    
    public synchronized void reset() {
        usedNames.remove(this.name);
        this.name = generateUniqueName();
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Creating new robots");
        Task184 robot1 = new Task184();
        Task184 robot2 = new Task184();
        System.out.println("Robot 1: " + robot1.getName());
        System.out.println("Robot 2: " + robot2.getName());
        System.out.println("Names are unique: " + !robot1.getName().equals(robot2.getName()));
        
        System.out.println("\\nTest Case 2: Resetting a robot");
        String oldName = robot1.getName();
        robot1.reset();
        String newName = robot1.getName();
        System.out.println("Old name: " + oldName);
        System.out.println("New name: " + newName);
        System.out.println("Names are different: " + !oldName.equals(newName));
        
        System.out.println("\\nTest Case 3: Creating multiple robots");
        List<Task184> robots = new ArrayList<>();
        for (int i = 0; i < 5; i++) {
            robots.add(new Task184());
        }
        Set<String> names = new HashSet<>();
        for (Task184 robot : robots) {
            names.add(robot.getName());
            System.out.println("Robot: " + robot.getName());
        }
        System.out.println("All names unique: " + (names.size() == robots.size()));
        
        System.out.println("\\nTest Case 4: Resetting multiple robots");
        for (Task184 robot : robots) {
            String before = robot.getName();
            robot.reset();
            System.out.println(before + " -> " + robot.getName());
        }
        
        System.out.println("\\nTest Case 5: Verify name format");
        Task184 robot3 = new Task184();
        String testName = robot3.getName();
        boolean validFormat = testName.matches("[A-Z]{2}\\\\d{3}");
        System.out.println("Robot name: " + testName);
        System.out.println("Valid format (2 letters + 3 digits): " + validFormat);
    }
}
