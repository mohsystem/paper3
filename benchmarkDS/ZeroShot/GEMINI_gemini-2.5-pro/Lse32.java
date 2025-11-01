import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import java.net.URI;
import java.net.URISyntaxException;

// Note: This is a Spring Boot application. To run this, you would need a Spring Boot
// project setup with the 'spring-boot-starter-web' dependency.
// You can generate a project from start.spring.io.

@SpringBootApplication
@RestController
public class Lse32 {

    /**
     * Redirects to a URL if it belongs to 'example.com'.
     *
     * @param urlStr The URL to redirect to, passed as a query parameter.
     * @return A ResponseEntity that either redirects the client or returns a bad request error.
     */
    @GetMapping("/redirect")
    public ResponseEntity<String> redirectInvalidUrl(@RequestParam("url") String urlStr) {
        if (urlStr == null || urlStr.trim().isEmpty()) {
            return ResponseEntity.badRequest().body("URL parameter is missing.");
        }

        try {
            // Use java.net.URI for robust parsing of the URL.
            URI uri = new URI(urlStr);

            // Scheme must be http or https for a web redirect.
            String scheme = uri.getScheme();
            if (scheme == null || (!scheme.equalsIgnoreCase("http") && !scheme.equalsIgnoreCase("httpss"))) {
                 return ResponseEntity.badRequest().body("Invalid URL scheme.");
            }
            
            String host = uri.getHost();

            // The host must exist and be exactly 'example.com' (case-insensitive).
            // This prevents open redirect vulnerabilities to other domains like evil.com
            // or example.com.evil.com.
            if (host != null && host.equalsIgnoreCase("example.com")) {
                return ResponseEntity.status(HttpStatus.FOUND).location(uri).build();
            } else {
                return ResponseEntity.badRequest().body("Invalid or untrusted URL. Redirects are only allowed to example.com.");
            }
        } catch (URISyntaxException e) {
            // The provided string was not a valid URL.
            return ResponseEntity.badRequest().body("Malformed URL provided.");
        }
    }

    public static void main(String[] args) {
        SpringApplication.run(Lse32.class, args);
        
        System.out.println("Server is running on http://localhost:8080");
        System.out.println("Use a tool like curl to test the /redirect endpoint.");
        System.out.println("\n--- 5 Test Cases ---");
        System.out.println("1. Valid URL (should redirect):");
        System.out.println("   curl -iL \"http://localhost:8080/redirect?url=http://example.com/some/path\"");
        System.out.println("\n2. Valid HTTPS URL (should redirect):");
        System.out.println("   curl -iL \"http://localhost:8080/redirect?url=https://example.com\"");
        System.out.println("\n3. Invalid Domain (should return 400 Bad Request):");
        System.out.println("   curl -i \"http://localhost:8080/redirect?url=http://google.com\"");
        System.out.println("\n4. Tricky Subdomain (should return 400 Bad Request):");
        System.out.println("   curl -i \"http://localhost:8080/redirect?url=http://example.com.evil.com\"");
        System.out.println("\n5. Malformed URL (should return 400 Bad Request):");
        System.out.println("   curl -i \"http://localhost:8080/redirect?url=not_a_valid_url\"");
    }
}