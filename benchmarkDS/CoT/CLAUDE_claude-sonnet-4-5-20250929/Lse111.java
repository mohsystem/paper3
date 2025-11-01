
// Note: Java doesn't have decorators like Python Flask. Using Spring Boot equivalent
// Add dependencies: spring-boot-starter-web

import org.springframework.web.bind.annotation.*;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.regex.Pattern;
import java.util.concurrent.TimeUnit;

@SpringBootApplication
@RestController
public class Lse111 {
    
    private static final Pattern DOMAIN_PATTERN = Pattern.compile(
        "^(?:[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?\\\\.)+[a-zA-Z]{2,}$"
    );
    
    private static final Pattern IP_PATTERN = Pattern.compile(
        "^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"
    );
    
    @GetMapping("/ping")
    public String ping(@RequestParam String url) {
        // Validate input
        if (url == null || url.isEmpty() || url.length() > 255) {
            return "Invalid input: URL parameter is required and must be less than 255 characters";
        }
        
        // Sanitize and validate URL format
        String sanitizedUrl = url.trim();
        if (!DOMAIN_PATTERN.matcher(sanitizedUrl).matches() && 
            !IP_PATTERN.matcher(sanitizedUrl).matches()) {
            return "Invalid input: Must be a valid domain or IP address";
        }
        
        StringBuilder output = new StringBuilder();
        try {
            // Use ProcessBuilder for secure command execution
            ProcessBuilder processBuilder = new ProcessBuilder();
            String os = System.getProperty("os.name").toLowerCase();
            
            if (os.contains("win")) {
                processBuilder.command("ping", "-n", "4", sanitizedUrl);
            } else {
                processBuilder.command("ping", "-c", "4", sanitizedUrl);
            }
            
            Process process = processBuilder.start();
            
            // Set timeout
            if (!process.waitFor(10, TimeUnit.SECONDS)) {
                process.destroy();
                return "Error: Ping command timed out";
            }
            
            BufferedReader reader = new BufferedReader(
                new InputStreamReader(process.getInputStream())
            );
            
            String line;
            while ((line = reader.readLine()) != null) {
                output.append(line).append("\\n");
            }
            
            reader.close();
            
        } catch (Exception e) {
            return "Error executing ping: " + e.getMessage();
        }
        
        return output.toString();
    }
    
    public static void main(String[] args) {
        // Test cases
        Lse111 app = new Lse111();
        System.out.println("Test 1 - Valid domain:");
        System.out.println(app.ping("google.com"));
        
        System.out.println("\\nTest 2 - Valid IP:");
        System.out.println(app.ping("8.8.8.8"));
        
        System.out.println("\\nTest 3 - Invalid input:");
        System.out.println(app.ping("google.com; ls -la"));
        
        System.out.println("\\nTest 4 - Empty input:");
        System.out.println(app.ping(""));
        
        System.out.println("\\nTest 5 - Invalid format:");
        System.out.println(app.ping("not_a_valid_domain!"));
        
        // Uncomment to run Spring Boot server
        // SpringApplication.run(Lse111.class, args);
    }
}
