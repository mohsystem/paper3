
// Java requires a web framework like Spring Boot
// Note: This is a complete Spring Boot application

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;

@SpringBootApplication
@RestController
public class Lse133 {
    
    @GetMapping("/")
    public String hello() {
        return "Hello, World!";
    }
    
    public static void main(String[] args) {
        // Test cases - Spring Boot web applications are tested via HTTP requests
        // The application will run on http://localhost:8080/
        SpringApplication.run(Lse133.class, args);
        
        // Note: For actual testing, you would use tools like:
        // - JUnit with MockMvc
        // - REST Assured
        // - Postman
        // Direct method call for demonstration:
        Lse133 app = new Lse133();
        System.out.println("Test 1: " + app.hello());
        System.out.println("Test 2: " + app.hello());
        System.out.println("Test 3: " + app.hello());
        System.out.println("Test 4: " + app.hello());
        System.out.println("Test 5: " + app.hello());
    }
}
