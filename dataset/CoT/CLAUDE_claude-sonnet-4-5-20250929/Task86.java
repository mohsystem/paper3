
// WARNING: This code is inherently insecure. Use only in controlled environments.
// CWE-494: Download of Code Without Integrity Check
// CWE-829: Inclusion of Functionality from Untrusted Control Sphere

import java.io.*;
import java.net.*;
import java.nio.file.*;
import java.security.MessageDigest;
import javax.net.ssl.HttpsURLConnection;
import java.util.Scanner;

public class Task86 {
    
    /**
     * SECURITY WARNING: Downloading and executing arbitrary executables is dangerous.
     * This should only be used in controlled environments with trusted sources.
     */
    public static boolean downloadAndExecute(String urlString, String expectedChecksum) {
        System.out.println("⚠️  SECURITY WARNING: This operation is inherently dangerous!");
        System.out.println("⚠️  Only proceed with trusted sources in isolated environments.");
        
        try {
            // Validate URL
            if (!isValidUrl(urlString)) {
                System.err.println("❌ Invalid or insecure URL. Only HTTPS URLs are allowed.");
                return false;
            }
            
            // Download file
            Path downloadedFile = downloadFile(urlString);
            if (downloadedFile == null) {
                return false;
            }
            
            // Verify checksum if provided
            if (expectedChecksum != null && !expectedChecksum.isEmpty()) {
                String actualChecksum = calculateSHA256(downloadedFile);
                if (!actualChecksum.equalsIgnoreCase(expectedChecksum)) {
                    System.err.println("❌ Checksum mismatch! File may be compromised.");
                    Files.deleteIfExists(downloadedFile);
                    return false;
                }
                System.out.println("✓ Checksum verified");
            } else {
                System.out.println("⚠️  WARNING: No checksum verification performed!");
            }
            
            // Request user confirmation
            System.out.println("\\n⚠️  About to execute: " + downloadedFile.getFileName());
            System.out.println("Type 'EXECUTE' to confirm (anything else to cancel): ");
            Scanner scanner = new Scanner(System.in);
            String confirmation = scanner.nextLine();
            
            if (!"EXECUTE".equals(confirmation)) {
                System.out.println("❌ Execution cancelled by user");
                Files.deleteIfExists(downloadedFile);
                return false;
            }
            
            // Execute (Windows only - this is a demonstration)
            return executeFile(downloadedFile);
            
        } catch (Exception e) {
            System.err.println("❌ Error: " + e.getMessage());
            return false;
        }
    }
    
    private static boolean isValidUrl(String urlString) {
        try {
            URL url = new URL(urlString);
            // Only allow HTTPS for security
            return "https".equalsIgnoreCase(url.getProtocol());
        } catch (MalformedURLException e) {
            return false;
        }
    }
    
    private static Path downloadFile(String urlString) throws IOException {
        URL url = new URL(urlString);
        HttpsURLConnection connection = (HttpsURLConnection) url.openConnection();
        connection.setRequestMethod("GET");
        connection.setConnectTimeout(10000);
        connection.setReadTimeout(30000);
        
        int responseCode = connection.getResponseCode();
        if (responseCode != 200) {
            System.err.println("❌ Download failed. HTTP response: " + responseCode);
            return null;
        }
        
        // Create temporary file
        Path tempFile = Files.createTempFile("downloaded_", ".exe");
        
        try (InputStream in = connection.getInputStream();
             OutputStream out = Files.newOutputStream(tempFile)) {
            
            byte[] buffer = new byte[8192];
            int bytesRead;
            long totalBytes = 0;
            
            System.out.println("⬇️  Downloading...");
            while ((bytesRead = in.read(buffer)) != -1) {
                out.write(buffer, 0, bytesRead);
                totalBytes += bytesRead;
            }
            
            System.out.println("✓ Downloaded " + totalBytes + " bytes");
            return tempFile;
            
        } catch (IOException e) {
            Files.deleteIfExists(tempFile);
            throw e;
        }
    }
    
    private static String calculateSHA256(Path file) throws Exception {
        MessageDigest digest = MessageDigest.getInstance("SHA-256");
        try (InputStream fis = Files.newInputStream(file)) {
            byte[] buffer = new byte[8192];
            int bytesRead;
            while ((bytesRead = fis.read(buffer)) != -1) {
                digest.update(buffer, 0, bytesRead);
            }
        }
        
        byte[] hash = digest.digest();
        StringBuilder hexString = new StringBuilder();
        for (byte b : hash) {
            hexString.append(String.format("%02x", b));
        }
        return hexString.toString();
    }
    
    private static boolean executeFile(Path file) throws IOException {
        String os = System.getProperty("os.name").toLowerCase();
        
        if (os.contains("win")) {
            ProcessBuilder pb = new ProcessBuilder(file.toString());
            pb.start();
            System.out.println("✓ Execution started (Windows)");
            return true;
        } else {
            System.err.println("❌ Execution only supported on Windows in this demo");
            return false;
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== EXE Downloader and Executor (DEMONSTRATION ONLY) ===\\n");
        System.out.println("⚠️  CRITICAL SECURITY WARNING ⚠️");
        System.out.println("This program can execute arbitrary code from the internet.");
        System.out.println("This is extremely dangerous and should NEVER be used in production.\\n");
        
        // Test cases - These are hypothetical and should NOT be executed
        System.out.println("Test Case 1: Invalid URL (HTTP instead of HTTPS)");
        downloadAndExecute("http://example.com/file.exe", null);
        
        System.out.println("\\nTest Case 2: Invalid URL format");
        downloadAndExecute("not-a-valid-url", null);
        
        System.out.println("\\n⚠️  Remaining test cases are disabled for safety.");
        System.out.println("In a real scenario, you would need:");
        System.out.println("- A valid HTTPS URL to a trusted executable");
        System.out.println("- The SHA-256 checksum of that executable");
        System.out.println("- Proper antivirus scanning");
        System.out.println("- Sandboxed execution environment");
    }
}
