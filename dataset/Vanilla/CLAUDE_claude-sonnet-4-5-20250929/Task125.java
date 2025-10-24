
import java.util.*;

public class Task125 {
    private Map<String, List<String>> userResources;
    private Map<String, String> userCredentials;
    
    public Task125() {
        userResources = new HashMap<>();
        userCredentials = new HashMap<>();
        initializeData();
    }
    
    private void initializeData() {
        // Initialize user credentials
        userCredentials.put("admin", "admin123");
        userCredentials.put("user1", "pass1");
        userCredentials.put("user2", "pass2");
        userCredentials.put("guest", "guest123");
        userCredentials.put("manager", "manager456");
        
        // Initialize user resources
        userResources.put("admin", Arrays.asList("Dashboard", "Users", "Settings", "Reports", "Database"));
        userResources.put("user1", Arrays.asList("Dashboard", "Profile", "Documents"));
        userResources.put("user2", Arrays.asList("Dashboard", "Profile", "Messages"));
        userResources.put("guest", Arrays.asList("Dashboard", "Help"));
        userResources.put("manager", Arrays.asList("Dashboard", "Reports", "Team", "Projects"));
    }
    
    public boolean authenticate(String username, String password) {
        return userCredentials.containsKey(username) && 
               userCredentials.get(username).equals(password);
    }
    
    public List<String> getAccessibleResources(String username) {
        return userResources.getOrDefault(username, new ArrayList<>());
    }
    
    public boolean canAccessResource(String username, String resource) {
        List<String> resources = getAccessibleResources(username);
        return resources.contains(resource);
    }
    
    public String accessResource(String username, String password, String resource) {
        if (!authenticate(username, password)) {
            return "Authentication failed: Invalid username or password";
        }
        
        if (canAccessResource(username, resource)) {
            return "Access granted: " + username + " can access " + resource;
        } else {
            return "Access denied: " + username + " cannot access " + resource;
        }
    }
    
    public static void main(String[] args) {
        Task125 system = new Task125();
        
        System.out.println("Test Case 1: Admin accessing Dashboard");
        System.out.println(system.accessResource("admin", "admin123", "Dashboard"));
        System.out.println();
        
        System.out.println("Test Case 2: User1 accessing Documents");
        System.out.println(system.accessResource("user1", "pass1", "Documents"));
        System.out.println();
        
        System.out.println("Test Case 3: Guest trying to access Database");
        System.out.println(system.accessResource("guest", "guest123", "Database"));
        System.out.println();
        
        System.out.println("Test Case 4: Invalid credentials");
        System.out.println(system.accessResource("user1", "wrongpass", "Dashboard"));
        System.out.println();
        
        System.out.println("Test Case 5: Manager accessing Reports");
        System.out.println(system.accessResource("manager", "manager456", "Reports"));
        System.out.println();
        
        System.out.println("Bonus: Listing all accessible resources for user1");
        List<String> resources = system.getAccessibleResources("user1");
        System.out.println("user1 can access: " + resources);
    }
}
