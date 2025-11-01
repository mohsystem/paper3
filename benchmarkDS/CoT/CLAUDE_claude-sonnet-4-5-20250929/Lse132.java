
// Java - Using Spring Boot Framework
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.util.HtmlUtils;

@SpringBootApplication
@Controller
public class Lse132 {
    
    @GetMapping("/hello/{username}")
    public String hello(@PathVariable String username, Model model) {
        // Input validation - only allow alphanumeric characters, underscore, and hyphen
        if (username == null || !username.matches("^[a-zA-Z0-9_-]{1,50}$")) {
            model.addAttribute("username", "Invalid User");
            model.addAttribute("error", "Username must be alphanumeric (1-50 chars)");
            return "hello";
        }
        
        // Sanitize input to prevent XSS
        String sanitizedUsername = HtmlUtils.htmlEscape(username);
        model.addAttribute("username", sanitizedUsername);
        
        return "hello";
    }
    
    public static void main(String[] args) {
        SpringApplication.run(Lse132.class, args);
        
        // Test cases demonstration (actual testing would require web server)
        System.out.println("Test cases for /hello/<username>:");
        System.out.println("1. /hello/john - Valid username");
        System.out.println("2. /hello/jane_doe - Valid with underscore");
        System.out.println("3. /hello/user-123 - Valid with hyphen");
        System.out.println("4. /hello/<script>alert('xss')</script> - Should be sanitized");
        System.out.println("5. /hello/verylongusernamethatshouldbevalidatedproperly - Length validation");
    }
}

// Template file (hello.html) should be placed in src/main/resources/templates/
// <!DOCTYPE html>
// <html xmlns:th="http://www.thymeleaf.org">
// <head><title>Hello</title></head>
// <body>
//     <h1>Hello, <span th:text="${username}">User</span>!</h1>
//     <p th:if="${error}" th:text="${error}" style="color: red;"></p>
// </body>
// </html>
