
import java.io.*;
import java.net.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.security.MessageDigest;
import java.util.*;
import javax.net.ssl.*;

public class Task129 {
    private static final String BASE_DIRECTORY = "/var/app/safe_files";
    private static final Set<String> ALLOWED_EXTENSIONS = new HashSet<>(Arrays.asList(".txt", ".pdf", ".jpg", ".png"));
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB
    
    public static String retrieveFileFromServer(String serverUrl, String fileName) {
        if (serverUrl == null || serverUrl.trim().isEmpty()) {
            return "Error: Server URL cannot be null or empty";
        }
        if (fileName == null || fileName.trim().isEmpty()) {
            return "Error: File name cannot be null or empty";
        }
        
        // Validate and sanitize file name
        String sanitizedFileName = sanitizeFileName(fileName);
        if (sanitizedFileName == null) {
            return "Error: Invalid file name";
        }
        
        // Validate server URL
        if (!isValidUrl(serverUrl)) {
            return "Error: Invalid server URL";
        }
        
        // Construct safe file path
        Path basePath = Paths.get(BASE_DIRECTORY).normalize().toAbsolutePath();
        Path targetPath = basePath.resolve(sanitizedFileName).normalize().toAbsolutePath();
        
        // Prevent path traversal
        if (!targetPath.startsWith(basePath)) {
            return "Error: Path traversal attempt detected";
        }
        
        // Validate file extension
        if (!hasAllowedExtension(sanitizedFileName)) {
            return "Error: File type not allowed";
        }
        
        try {
            // Create base directory if it doesn't exist
            Files.createDirectories(basePath);
            
            // Download file from server
            String fullUrl = serverUrl + "/" + URLEncoder.encode(sanitizedFileName, StandardCharsets.UTF_8.name());
            URL url = new URL(fullUrl);
            
            // Configure SSL context for secure connections
            HttpsURLConnection connection = null;
            if (url.getProtocol().equalsIgnoreCase("https")) {
                connection = (HttpsURLConnection) url.openConnection();
                connection.setHostnameVerifier(new DefaultHostnameVerifier());
            } else if (url.getProtocol().equalsIgnoreCase("http")) {
                return "Error: Only HTTPS connections are allowed";
            } else {
                return "Error: Unsupported protocol";
            }
            
            connection.setRequestMethod("GET");
            connection.setConnectTimeout(10000);
            connection.setReadTimeout(10000);
            connection.setInstanceFollowRedirects(false);
            
            int responseCode = connection.getResponseCode();
            if (responseCode != HttpURLConnection.HTTP_OK) {
                return "Error: Server returned status code " + responseCode;
            }
            
            long contentLength = connection.getContentLengthLong();
            if (contentLength > MAX_FILE_SIZE) {
                return "Error: File size exceeds maximum allowed size";
            }
            
            // Write to temporary file first
            Path tempFile = Files.createTempFile(basePath, "tmp_", ".download");
            
            try (InputStream in = connection.getInputStream();
                 OutputStream out = Files.newOutputStream(tempFile, StandardOpenOption.WRITE)) {
                
                byte[] buffer = new byte[8192];
                int bytesRead;
                long totalBytesRead = 0;
                
                while ((bytesRead = in.read(buffer)) != -1) {
                    totalBytesRead += bytesRead;
                    if (totalBytesRead > MAX_FILE_SIZE) {
                        Files.deleteIfExists(tempFile);
                        return "Error: File size exceeds maximum allowed size";
                    }
                    out.write(buffer, 0, bytesRead);
                }
                
                out.flush();
            }
            
            // Atomic move to final location
            Files.move(tempFile, targetPath, StandardCopyOption.REPLACE_EXISTING, StandardCopyOption.ATOMIC_MOVE);
            
            return "Success: File retrieved and saved to " + targetPath.toString();
            
        } catch (MalformedURLException e) {
            return "Error: Invalid URL format";
        } catch (IOException e) {
            return "Error: Failed to retrieve file - " + e.getMessage();
        }
    }
    
    private static String sanitizeFileName(String fileName) {
        if (fileName == null || fileName.isEmpty()) {
            return null;
        }
        
        // Remove path separators and null bytes
        String sanitized = fileName.replaceAll("[/\\\\\\\\\\\\x00]", "");
        
        // Remove leading/trailing dots and spaces
        sanitized = sanitized.trim().replaceAll("^\\\\.+", "");
        
        // Check for valid characters only
        if (!sanitized.matches("^[a-zA-Z0-9._-]+$")) {
            return null;
        }
        
        // Limit length
        if (sanitized.length() > 255) {
            return null;
        }
        
        return sanitized;
    }
    
    private static boolean hasAllowedExtension(String fileName) {
        String lowerFileName = fileName.toLowerCase();
        for (String ext : ALLOWED_EXTENSIONS) {
            if (lowerFileName.endsWith(ext)) {
                return true;
            }
        }
        return false;
    }
    
    private static boolean isValidUrl(String urlString) {
        try {
            URL url = new URL(urlString);
            if (!url.getProtocol().equalsIgnoreCase("https")) {
                return false;
            }
            String host = url.getHost();
            if (host == null || host.isEmpty()) {
                return false;
            }
            return true;
        } catch (MalformedURLException e) {
            return false;
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Valid HTTPS URL and file name");
        System.out.println(retrieveFileFromServer("https://example.com/files", "document.txt"));
        
        System.out.println("\\nTest Case 2: Path traversal attempt");
        System.out.println(retrieveFileFromServer("https://example.com/files", "../../../etc/passwd"));
        
        System.out.println("\\nTest Case 3: Invalid file extension");
        System.out.println(retrieveFileFromServer("https://example.com/files", "malware.exe"));
        
        System.out.println("\\nTest Case 4: Invalid URL (HTTP instead of HTTPS)");
        System.out.println(retrieveFileFromServer("http://example.com/files", "document.txt"));
        
        System.out.println("\\nTest Case 5: Null or empty inputs");
        System.out.println(retrieveFileFromServer("", "document.txt"));
    }
}
