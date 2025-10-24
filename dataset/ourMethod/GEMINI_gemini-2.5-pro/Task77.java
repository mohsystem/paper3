import org.apache.commons.net.ftp.FTP;
import org.apache.commons.net.ftp.FTPSClient;
import org.apache.commons.net.ftp.FTPReply;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.nio.file.InvalidPathException;

// Note: This code requires the Apache Commons Net library.
// For Maven, add this to your pom.xml:
// <dependency>
//     <groupId>commons-net</groupId>
//     <artifactId>commons-net</artifactId>
//     <version>3.9.0</version>
// </dependency>

public class Task77 {

    /**
     * Connects to an FTP server using FTPS, logs in, and downloads a file securely.
     * @param hostname The FTP server hostname.
     * @param username The username for authentication.
     * @param password The password for authentication.
     * @param remoteFilePath The full path of the file to download from the server.
     * @return true if download was successful, false otherwise.
     */
    public static boolean downloadFile(String hostname, String username, String password, String remoteFilePath) {
        // Rule #11: Validate and sanitize the remote path to get a safe local filename.
        String localFileName;
        try {
            Path remotePath = Paths.get(remoteFilePath);
            localFileName = remotePath.getFileName().toString();
        } catch (InvalidPathException e) {
            System.err.println("Error: Invalid remote file path provided.");
            return false;
        }


        if (localFileName.isEmpty() || localFileName.contains("/") || localFileName.contains("\\")) {
            System.err.println("Error: Invalid remote file path resulting in an unsafe local filename.");
            return false;
        }
        
        Path localPath = Paths.get(localFileName);
        
        // Use FTPS for a secure connection.
        FTPSClient ftpsClient = new FTPSClient();
        Path tempFile = null;

        try {
            ftpsClient.connect(hostname);
            int reply = ftpsClient.getReplyCode();

            if (!FTPReply.isPositiveCompletion(reply)) {
                ftpsClient.disconnect();
                System.err.println("FTP server refused connection.");
                return false;
            }

            if (!ftpsClient.login(username, password)) {
                System.err.println("Login failed. Check username and password.");
                ftpsClient.logout();
                return false;
            }

            ftpsClient.execPROT("P"); // Set data channel protection to Private
            ftpsClient.enterLocalPassiveMode();
            ftpsClient.setFileType(FTP.BINARY_FILE_TYPE);
            
            // Rule #15: Download to a temporary file to prevent partial downloads.
            tempFile = Files.createTempFile("download-", ".tmp");

            try (OutputStream outputStream = new FileOutputStream(tempFile.toFile())) {
                System.out.println("Downloading file " + remoteFilePath + " to " + localPath + "...");
                boolean success = ftpsClient.retrieveFile(remoteFilePath, outputStream);
                
                if (!success) {
                    System.err.println("File download failed. Check remote file path and permissions.");
                    return false;
                }
            }
            
            // Rule #15: Atomically move the temporary file to the final destination.
            Files.move(tempFile, localPath, StandardCopyOption.REPLACE_EXISTING, StandardCopyOption.ATOMIC_MOVE);
            System.out.println("File downloaded successfully.");
            return true;

        } catch (IOException e) {
            System.err.println("An error occurred: " + e.getMessage());
            return false;
        } finally {
            if (tempFile != null) {
                try {
                    Files.deleteIfExists(tempFile);
                } catch (IOException e) {
                    System.err.println("Failed to delete temporary file: " + e.getMessage());
                }
            }
            if (ftpsClient.isConnected()) {
                try {
                    ftpsClient.logout();
                    ftpsClient.disconnect();
                } catch (IOException e) {
                    // Ignored
                }
            }
        }
    }

    public static void main(String[] args) {
        if (args.length != 4) {
            System.out.println("Usage: java -cp \".:commons-net-3.9.0.jar\" Task77 <hostname> <username> <password> <remoteFilePath>");
            System.out.println("WARNING: Providing a password on the command line is insecure.");
            return;
        }

        String hostname = args[0];
        String username = args[1];
        String password = args[2];
        String remoteFilePath = args[3];

        System.out.println("--- Running Main Logic ---");
        downloadFile(hostname, username, password, remoteFilePath);
        
        System.out.println("\n--- Mock Test Cases (will fail without a running FTP server) ---");

        // Test Case 1: Valid inputs (placeholders)
        System.out.println("\nTest Case 1: Valid inputs");
        downloadFile("ftp.example.com", "user", "pass", "/pub/file.txt");

        // Test Case 2: Invalid hostname
        System.out.println("\nTest Case 2: Invalid hostname");
        downloadFile("invalid.hostname.nonexistent", "user", "pass", "/pub/file.txt");

        // Test Case 3: Invalid credentials
        System.out.println("\nTest Case 3: Invalid credentials");
        downloadFile("ftp.example.com", "wronguser", "wrongpass", "/pub/file.txt");

        // Test Case 4: File not found
        System.out.println("\nTest Case 4: File not found");
        downloadFile("ftp.example.com", "user", "pass", "/nonexistent/file.txt");
        
        // Test Case 5: Path traversal attempt
        System.out.println("\nTest Case 5: Path traversal attempt");
        downloadFile("ftp.example.com", "user", "pass", "../../../etc/passwd");
    }
}