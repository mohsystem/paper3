import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.PrintWriter;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.util.Arrays;
import java.util.List;
import java.util.UUID;
import java.util.stream.Collectors;

import jakarta.servlet.ServletException;
import jakarta.servlet.annotation.MultipartConfig;
import jakarta.servlet.http.HttpServlet;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;
import jakarta.servlet.http.Part;
import org.eclipse.jetty.server.Server;
import org.eclipse.jetty.servlet.ServletContextHandler;
import org.eclipse.jetty.servlet.ServletHolder;

public class Lse71 {

    // Define a secure location for file uploads.
    // In a real application, this path should be outside the web application's deployment directory
    // and configured in an external configuration file.
    private static final String UPLOAD_DIR = System.getProperty("java.io.tmpdir") + File.separator + "webapp_uploads";
    
    @MultipartConfig(
        fileSizeThreshold = 1024 * 1024 * 1,  // 1 MB
        maxFileSize = 1024 * 1024 * 10, // 10 MB
        maxRequestSize = 1024 * 1024 * 15 // 15 MB
    )
    public static class FileUploadServlet extends HttpServlet {

        private static final long serialVersionUID = 1L;
        // Whitelist of allowed file extensions to prevent upload of malicious scripts.
        private static final List<String> ALLOWED_EXTENSIONS = Arrays.asList("txt", "pdf", "png", "jpg", "jpeg", "gif", "doc", "docx");

        @Override
        public void init() throws ServletException {
            super.init();
            // Create the upload directory if it doesn't exist
            File uploadDir = new File(UPLOAD_DIR);
            if (!uploadDir.exists()) {
                if (!uploadDir.mkdirs()) {
                    throw new ServletException("Could not create upload directory: " + UPLOAD_DIR);
                }
            }
        }

        @Override
        protected void doGet(HttpServletRequest req, HttpServletResponse resp) throws ServletException, IOException {
            resp.setContentType("text/html");
            PrintWriter writer = resp.getWriter();
            writer.println("<html><head><title>Java File Upload</title></head><body>");
            writer.println("<h1>Upload a File</h1>");
            writer.println("<form method='post' action='/upload' enctype='multipart/form-data'>");
            writer.println("<input type='file' name='file' />");
            writer.println("<input type='submit' value='Upload' />");
            writer.println("</form>");
            writer.println("</body></html>");
        }

        @Override
        protected void doPost(HttpServletRequest req, HttpServletResponse resp) throws ServletException, IOException {
            resp.setContentType("text/html");
            PrintWriter writer = resp.getWriter();
            String message;

            try {
                // Get the file part from the request
                Part filePart = req.getPart("file");
                if (filePart == null || filePart.getSize() == 0) {
                     message = "No file was uploaded.";
                } else {
                    String originalFileName = Paths.get(filePart.getSubmittedFileName()).getFileName().toString();

                    // Security: Validate file extension against a whitelist
                    String fileExtension = getFileExtension(originalFileName);
                    if (fileExtension == null || !ALLOWED_EXTENSIONS.contains(fileExtension.toLowerCase())) {
                        message = "Error: File type not allowed. Allowed types are: " + String.join(", ", ALLOWED_EXTENSIONS);
                    } else {
                        // Security: Generate a new, unique filename to prevent path traversal and overwriting files.
                        String newFileName = UUID.randomUUID().toString() + "." + fileExtension;
                        Path destinationPath = Paths.get(UPLOAD_DIR, newFileName);

                        // Save the file
                        try (InputStream fileContent = filePart.getInputStream()) {
                            Files.copy(fileContent, destinationPath, StandardCopyOption.REPLACE_EXISTING);
                            message = "File uploaded successfully and saved as: " + newFileName;
                            System.out.println("Saved file to: " + destinationPath.toAbsolutePath());
                        } catch (IOException e) {
                            message = "Error saving file: " + e.getMessage();
                            e.printStackTrace();
                        }
                    }
                }
            } catch (Exception e) {
                // Catches errors like file size exceeding limits set in @MultipartConfig
                message = "Error during upload: " + e.getMessage();
                 e.printStackTrace();
            }

            writer.println("<html><body><h2>" + message + "</h2>");
            writer.println("<a href='/'>Upload another file</a></body></html>");
        }
        
        private String getFileExtension(String fileName) {
            if (fileName == null || fileName.lastIndexOf(".") == -1) {
                return null;
            }
            return fileName.substring(fileName.lastIndexOf(".") + 1);
        }
    }

    public static void main(String[] args) throws Exception {
        // Embed a Jetty server to run the servlet.
        // This makes the application self-contained and runnable from a single file.
        Server server = new Server(8080);
        ServletContextHandler context = new ServletContextHandler(ServletContextHandler.SESSIONS);
        context.setContextPath("/");
        server.setHandler(context);
        
        context.addServlet(new ServletHolder(new FileUploadServlet()), "/*");
        
        System.out.println("Server starting on port 8080...");
        System.out.println("Access the application at http://localhost:8080");
        System.out.println("Uploaded files will be saved in: " + UPLOAD_DIR);
        System.out.println("\n--- How to Test ---");
        System.out.println("1. Open http://localhost:8080 in your web browser.");
        System.out.println("2. Test Case 1 (Success): Upload a valid file like a .txt or .png file (less than 10MB).");
        System.out.println("3. Test Case 2 (Invalid Type): Try to upload a disallowed file like a .html or .exe file.");
        System.out.println("4. Test Case 3 (No File): Click the 'Upload' button without selecting a file.");
        System.out.println("5. Test Case 4 (File Too Large): Try to upload a file larger than 10MB.");
        System.out.println("6. Test Case 5 (Path Traversal attempt): Rename a file to `../../evil.txt` and try to upload it. The server will safely handle it.");
        
        server.start();
        server.join();
    }
}