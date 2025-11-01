
// Java using Spring Boot framework
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;

@SpringBootApplication
@Controller
public class Lse132 {
    
    @GetMapping("/hello/{username}")
    public String hello(@PathVariable String username, Model model) {
        model.addAttribute("username", username);
        return "hello";
    }
    
    public static void main(String[] args) {
        SpringApplication.run(Lse132.class, args);
        
        // Test cases (for demonstration - actual testing would require HTTP requests):
        // 1. Visit: http://localhost:8080/hello/John
        // 2. Visit: http://localhost:8080/hello/Alice
        // 3. Visit: http://localhost:8080/hello/Bob
        // 4. Visit: http://localhost:8080/hello/Charlie
        // 5. Visit: http://localhost:8080/hello/Diana
        
        System.out.println("Server started. Test the following URLs:");
        System.out.println("http://localhost:8080/hello/John");
        System.out.println("http://localhost:8080/hello/Alice");
        System.out.println("http://localhost:8080/hello/Bob");
        System.out.println("http://localhost:8080/hello/Charlie");
        System.out.println("http://localhost:8080/hello/Diana");
    }
}

// Note: Create a file named hello.html in src/main/resources/templates/ directory:
// <!DOCTYPE html>
// <html>
// <head><title>Hello</title></head>
// <body>
//     <h1>Hello, <span th:text="${username}"></span>!</h1>
// </body>
// </html>
