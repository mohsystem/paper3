
// Java doesn't have a direct equivalent to Flask's WSGI framework
// Using Spring Boot as the closest equivalent for a web application

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;

@SpringBootApplication
@RestController
class Lse133 {
    
    @GetMapping("/")
    public String hello() {
        return "Hello, World!";
    }
    
    public static void main(String[] args) {
        SpringApplication.run(Lse133.class, args);
        
        // Test cases (Note: For actual testing, you would need to run the server and make HTTP requests)
        System.out.println("Server starting...");
        System.out.println("Test Case 1: Visit http://localhost:8080/ to see 'Hello, World!'");
        System.out.println("Test Case 2: Check if server responds to GET request");
        System.out.println("Test Case 3: Verify response status code is 200");
        System.out.println("Test Case 4: Verify response content-type is text/html or text/plain");
        System.out.println("Test Case 5: Verify application starts without errors");
    }
}
