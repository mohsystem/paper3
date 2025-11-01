import java.io.*;
import java.nio.file.*;
import java.util.*;
import java.util.regex.*;

public class Lse132 {
    // Ensures templates directory and hello.html exist with content
    private static void ensureTemplateFile() {
        try {
            Path templatesDir = Paths.get("templates");
            if (!Files.exists(templatesDir)) {
                Files.createDirectories(templatesDir);
            }
            Path templateFile = templates.get("hello");
            if (!Files.exists(templateFile)) {
                String content = "<!doctype html>\n" +
                                 "<html>\n" +
                                 "<head><title>Hello</title></head>\n" +
                                 "<body>\n" +
                                 "<h1>Hello, ${username}!</h1>\n" +
                                 "</body>\n" +
                                 "</html>\n";
                Files.writeString(templateFile, content);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    // Map to hold template paths
    private static final Map<String, Path> templates = new HashMap<>();
    static {
        templates.put("hello", Paths.get("templates", "hello.html"));
    }

    // Renders a template by replacing ${key} placeholders
    public static String renderTemplate(String templatePath, Map<String, String> context) {
        try {
            String template = Files.readString(Paths.get(templatePath));
            for (Map.Entry<String, String> e : context.entrySet()) {
                template = template.replace("${" + e.getKey() + "}", e.getValue());
            }
            return template;
        } catch (IOException e) {
            return "Error loading template: " + e.getMessage();
        }
    }

    // Handler bound to URL /hello/{username} conceptually
    public static String hello(String username) {
        Map<String, String> ctx = new HashMap<>();
        ctx.put("username", username);
        return renderTemplate(templates.get("hello").toString(), ctx);
    }

    // Simple router to demonstrate URL -> handler binding
    public static String handleRequest(String url) {
        Pattern p = Pattern.compile("^/hello/([^/]+)$");
        Matcher m = p.matcher(url);
        if (m.matches()) {
            String username = m.group(1);
            return hello(username);
        }
        return "404 Not Found";
    }

    public static void main(String[] args) {
        ensureTemplateFile();
        String[] tests = {
            "/hello/alice",
            "/hello/Bob",
            "/hello/charlie_123",
            "/hello/Dana-XYZ",
            "/hello/Eve"
        };
        for (String url : tests) {
            String out = handleRequest(url);
            System.out.println("URL: " + url);
            System.out.println(out);
            System.out.println("-----");
        }
    }
}