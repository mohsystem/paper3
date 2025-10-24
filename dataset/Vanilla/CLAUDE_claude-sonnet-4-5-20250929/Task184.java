
import java.util.*;

public class Task184 {
    private static Set<String> usedNames = new HashSet<>();
    private static Random random = new Random();
    
    private String name;
    
    public Task184() {
        this.name = generateUniqueName();
    }
    
    public String getName() {
        return this.name;
    }
    
    public void reset() {
        if (this.name != null) {
            usedNames.remove(this.name);
        }
        this.name = generateUniqueName();
    }
    
    private static String generateUniqueName() {
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
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Create a robot and get its name");
        Task184 robot1 = new Task184();
        String name1 = robot1.getName();
        System.out.println("Robot 1 name: " + name1);
        System.out.println("Name matches pattern: " + name1.matches("[A-Z]{2}\\\\d{3}"));
        
        System.out.println("\\nTest Case 2: Create another robot with different name");
        Task184 robot2 = new Task184();
        String name2 = robot2.getName();
        System.out.println("Robot 2 name: " + name2);
        System.out.println("Names are different: " + (!name1.equals(name2)));
        
        System.out.println("\\nTest Case 3: Reset robot and verify new name");
        String oldName = robot1.getName();
        robot1.reset();
        String newName = robot1.getName();
        System.out.println("Old name: " + oldName);
        System.out.println("New name: " + newName);
        System.out.println("Names are different after reset: " + (!oldName.equals(newName)));
        
        System.out.println("\\nTest Case 4: Create multiple robots and verify uniqueness");
        Set<String> names = new HashSet<>();
        for (int i = 0; i < 10; i++) {
            Task184 robot = new Task184();
            names.add(robot.getName());
        }
        System.out.println("Created 10 robots, unique names count: " + names.size());
        System.out.println("All names are unique: " + (names.size() == 10));
        
        System.out.println("\\nTest Case 5: Reset multiple times");
        Task184 robot3 = new Task184();
        Set<String> resetNames = new HashSet<>();
        resetNames.add(robot3.getName());
        for (int i = 0; i < 5; i++) {
            robot3.reset();
            resetNames.add(robot3.getName());
        }
        System.out.println("Robot reset 5 times, unique names: " + resetNames.size());
        System.out.println("All reset names are unique: " + (resetNames.size() == 6));
    }
}
