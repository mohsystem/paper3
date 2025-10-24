/*
 * NOTE: This code requires the Apache Commons Net library.
 * You can download it from https://commons.apache.org/proper/commons-net/
 * Add the commons-net-x.x.x.jar to your classpath.
 * For example, to compile and run:
 * javac -cp ".;commons-net-3.9.0.jar" Task77.java
 * java -cp ".;commons-net-3.9.0.jar" Task77 <hostname> <user> <pass> <remote_file> <local_file>
 */
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import org.apache.commons.net.ftp.FTP;
import org.apache.commons.net.ftp.FTPClient;

public class Task77 {

    /**
     * Connects to an FTP server and downloads a file.
     *
     * @param host         The FTP server hostname.
     * @param user         The username for login.
     * @param password     The password for login.
     * @param remoteFile   The full path of the file to download from the server.
     * @param localFile    The name of the file to save locally.
     * @return true if download was successful, false otherwise.
     */
    public static boolean downloadFileFromFtp(String host, String user, String password, String remoteFile, String localFile) {
        FTPClient ftpClient = new FTPClient();
        try {
            ftpClient.connect(host);
            int replyCode = ftpClient.getReplyCode();
            if (replyCode != 220) {
                 System.err.println("FTP server refused connection.");
                 return false;
            }

            boolean loggedIn = ftpClient.login(user, password);
            if (!loggedIn) {
                System.err.println("Could not login to the server.");
                return false;
            }

            System.out.println("Logged in successfully.");

            // Enter passive mode to deal with firewalls.
            ftpClient.enterLocalPassiveMode();
            // Set file type to binary
            ftpClient.setFileType(FTP.BINARY_FILE_TYPE);

            try (OutputStream outputStream = new FileOutputStream(localFile)) {
                System.out.println("Downloading file " + remoteFile + " to " + localFile);
                boolean success = ftpClient.retrieveFile(remoteFile, outputStream);

                if (success) {
                    System.out.println("File downloaded successfully!");
                } else {
                    System.err.println("File download failed.");
                }
                return success;
            }

        } catch (IOException ex) {
            System.err.println("Error: " + ex.getMessage());
            ex.printStackTrace();
            return false;
        } finally {
            try {
                if (ftpClient.isConnected()) {
                    ftpClient.logout();
                    ftpClient.disconnect();
                }
            } catch (IOException ex) {
                ex.printStackTrace();
            }
        }
    }

    public static void main(String[] args) {
        /*
         * Main method to run the FTP download from command line arguments.
         *
         * 5 Test Cases (using public FTP servers):
         *
         * Test Case 1: Download readme.txt from test.rebex.net
         * java -cp ".;commons-net-3.9.0.jar" Task77 test.rebex.net demo password /readme.txt rebex_readme.txt
         *
         * Test Case 2: Download a test file from ftp.dlptest.com
         * java -cp ".;commons-net-3.9.0.jar" Task77 ftp.dlptest.com dlpuser rNrKYTX9g7z3RgJR /test-file-100kb.txt dlp_test_100kb.txt
         *
         * Test Case 3: Download a different file from test.rebex.net
         * java -cp ".;commons-net-3.9.0.jar" Task77 test.rebex.net demo password /pub/example/readme.txt rebex_pub_readme.txt
         *
         * Test Case 4: (Dummy) Download a backup file from a private server
         * java -cp ".;commons-net-3.9.0.jar" Task77 my-private-ftp.net admin secretpass /backups/2023.zip backup.zip
         *
         * Test Case 5: (Dummy) Download a log file from a local network server
         * java -cp ".;commons-net-3.9.0.jar" Task77 192.168.1.100 localuser 12345 /logs/app.log app_log_today.log
         *
         */

        if (args.length < 5) {
            System.out.println("Usage: java Task77 <hostname> <username> <password> <remote_filepath> <local_filename>");
            System.exit(1);
        }

        String hostname = args[0];
        String username = args[1];
        String password = args[2];
        String remoteFile = args[3];
        String localFile = args[4];

        downloadFileFromFtp(hostname, username, password, remoteFile, localFile);
    }
}