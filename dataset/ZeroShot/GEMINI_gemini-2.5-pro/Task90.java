import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

public class Task90 {

    // A thread-safe list to store writer objects for all connected clients.
    // Using CopyOnWriteArrayList is efficient for broadcast scenarios where reads (iteration)
    // are far more frequent than writes (adding/removing clients).
    private static final List<PrintWriter> clientWriters = new CopyOnWriteArrayList<>();
    private static final int PORT = 8080;

    public static void main(String[] args) {
        /*
         * How to test this server:
         * 1. Compile and run this Java file. The server will start listening on port 8080.
         * 2. Open a terminal/command prompt and connect using a client like telnet or netcat:
         *    `telnet localhost 8080` or `nc localhost 8080`
         * 3. Open another terminal and connect again. You now have two clients.
         * 4. Type a message in one terminal and press Enter. It should appear in the other terminal.
         * 5. Repeat with more clients (e.g., 5 test cases means 5 connected clients).
         * 6. Close a client's terminal. The server will handle the disconnection gracefully.
        */
        System.out.println("Chat Server is running on port " + PORT);
        // Using try-with-resources to ensure the ServerSocket is automatically closed.
        try (ServerSocket serverSocket = new ServerSocket(PORT)) {
            while (true) {
                // Accept new client connections. This is a blocking call.
                Socket clientSocket = serverSocket.accept();
                System.out.println("New client connected: " + clientSocket.getRemoteSocketAddress());
                // Create and start a new thread for each client to handle them concurrently.
                new Thread(new ClientHandler(clientSocket)).start();
            }
        } catch (IOException e) {
            System.err.println("Error in server: " + e.getMessage());
            // This is a serious error, so we should probably exit.
        }
    }

    private static class ClientHandler implements Runnable {
        private final Socket clientSocket;
        private PrintWriter writer;

        public ClientHandler(Socket socket) {
            this.clientSocket = socket;
        }

        @Override
        public void run() {
            try (
                BufferedReader reader = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
                PrintWriter writer = new PrintWriter(clientSocket.getOutputStream(), true)
            ) {
                this.writer = writer;
                clientWriters.add(writer);

                String clientAddress = clientSocket.getRemoteSocketAddress().toString();
                broadcastMessage("Server: Client " + clientAddress + " has joined.");

                String message;
                // Read messages from the client until they disconnect (readLine returns null).
                while ((message = reader.readLine()) != null) {
                    if (message.trim().isEmpty()) {
                        continue; // Ignore empty messages
                    }
                    // Secure: Limiting message length could be added here.
                    String broadcastMsg = clientAddress + ": " + message;
                    broadcastMessage(broadcastMsg);
                }
            } catch (IOException e) {
                // This exception is expected when a client disconnects unexpectedly.
                System.err.println("Error handling client " + clientSocket.getRemoteSocketAddress() + ": " + e.getMessage());
            } finally {
                // Cleanup resources and notify others of disconnection.
                if (writer != null) {
                    clientWriters.remove(writer);
                }
                broadcastMessage("Server: Client " + clientSocket.getRemoteSocketAddress() + " has left.");
                try {
                    // Secure: Ensure the socket is always closed.
                    clientSocket.close();
                } catch (IOException e) {
                    // Ignore, as we are already in a cleanup phase.
                }
            }
        }

        private void broadcastMessage(String message) {
            // Iterate over the thread-safe list to send the message to all clients.
            for (PrintWriter writer : clientWriters) {
                // Do not send message back to the sender (optional, good for some UIs)
                // if (writer != this.writer) {
                    writer.println(message);
                // }
            }
            System.out.println("Broadcasted: " + message);
        }
    }
}