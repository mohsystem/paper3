
import java.io.*;
import java.nio.file.*;
import java.security.MessageDigest;
import java.util.*;
import javax.servlet.*;
import javax.servlet.http.*;
import javax.servlet.annotation.*;

@WebServlet("/upload")
@MultipartConfig(maxFileSize = 5242880) // 5MB max
public class Lse70 extends HttpServlet {
    private static final String UPLOAD_DIR = "uploads";
    private static final Set<String> ALLOWED_EXTENSIONS = new HashSet<>(
        Arrays.asList("txt", "pdf", "png", "jpg", "jpeg", "gif", "doc", "docx")
    );
    private static final long MAX_FILE_SIZE = 5242880; // 5MB

    protected void doPost(HttpServletRequest request, HttpServletResponse response)
            throws ServletException, IOException {
        String result = upload_file(request);
        response.setContentType("text/html");
        response.getWriter().println(result);
    }

    private String upload_file(HttpServletRequest request) {
        try {
            // Create upload directory if it doesn't exist\n            File uploadDir = new File(UPLOAD_DIR);\n            if (!uploadDir.exists()) {\n                uploadDir.mkdir();\n            }\n\n            Part filePart = request.getPart("file");\n            if (filePart == null) {\n                return "No file uploaded";\n            }\n\n            String fileName = getFileName(filePart);\n            if (fileName == null || fileName.isEmpty()) {\n                return "Invalid file name";\n            }\n\n            // Validate file extension\n            String extension = getFileExtension(fileName);\n            if (!ALLOWED_EXTENSIONS.contains(extension.toLowerCase())) {\n                return "File type not allowed: " + extension;\n            }\n\n            // Validate file size\n            if (filePart.getSize() > MAX_FILE_SIZE) {\n                return "File size exceeds maximum allowed (5MB)";\n            }\n\n            // Generate unique filename to prevent overwriting\n            String safeFileName = sanitizeFileName(fileName);\n            String uniqueFileName = System.currentTimeMillis() + "_" + safeFileName;\n            String filePath = UPLOAD_DIR + File.separator + uniqueFileName;\n\n            // Save file\n            try (InputStream input = filePart.getInputStream()) {\n                Files.copy(input, Paths.get(filePath), StandardCopyOption.REPLACE_EXISTING);\n            }\n\n            return "File uploaded successfully: " + uniqueFileName;\n        } catch (Exception e) {\n            return "Error uploading file: " + e.getMessage();\n        }\n    }\n\n    private String getFileName(Part part) {\n        String contentDisp = part.getHeader("content-disposition");\n        String[] tokens = contentDisp.split(";");\n        for (String token : tokens) {\n            if (token.trim().startsWith("filename")) {\n                return token.substring(token.indexOf("=") + 2, token.length() - 1);\n            }\n        }\n        return null;\n    }\n\n    private String getFileExtension(String fileName) {\n        int lastDot = fileName.lastIndexOf('.');
        if (lastDot > 0) {
            return fileName.substring(lastDot + 1);
        }
        return "";
    }

    private String sanitizeFileName(String fileName) {
        // Remove path traversal characters and keep only safe characters
        return fileName.replaceAll("[^a-zA-Z0-9\\\\.\\\\-_]", "_");
    }

    public static void main(String[] args) {
        System.out.println("File Upload Security Features Implemented:");
        System.out.println("1. File extension whitelist validation");
        System.out.println("2. File size limit (5MB)");
        System.out.println("3. Filename sanitization");
        System.out.println("4. Unique filename generation");
        System.out.println("5. Path traversal prevention");
    }
}
