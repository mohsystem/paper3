import java.io.*;
import java.net.*;
import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicInteger;

public class Task90 {
    // Chat server implementation
    public static class ChatServer {
        private final int port;
        private ServerSocket serverSocket;
        private final Set<ClientHandler> clients = ConcurrentHashMap.newKeySet();
        private final ExecutorService pool = Executors.newCachedThreadPool();
        private volatile boolean running = false;
        private Thread acceptThread;
        private final AtomicInteger clientIdGen = new AtomicInteger(1);

        public ChatServer(int port) {
            this.port = port;
        }

        public boolean start() {
            try {
                serverSocket = new ServerSocket(port);
                running = true;
                acceptThread = new Thread(this::acceptLoop, "AcceptLoop");
                acceptThread.start();
                return true;
            } catch (IOException e) {
                e.printStackTrace();
                return false;
            }
        }

        public void stop() {
            running = false;
            try {
                if (serverSocket != null && !serverSocket.isClosed()) {
                    serverSocket.close();
                }
            } catch (IOException ignored) {}
            for (ClientHandler ch : clients) {
                ch.close();
            }
            pool.shutdownNow();
            if (acceptThread != null) {
                try {
                    acceptThread.join(1000);
                } catch (InterruptedException ignored) {}
            }
        }

        private void acceptLoop() {
            while (running) {
                try {
                    Socket s = serverSocket.accept();
                    s.setTcpNoDelay(true);
                    ClientHandler ch = new ClientHandler(s, "Client-" + clientIdGen.getAndIncrement());
                    clients.add(ch);
                    pool.execute(ch);
                } catch (SocketException se) {
                    break; // server socket closed
                } catch (IOException e) {
                    if (running) e.printStackTrace();
                }
            }
        }

        public void broadcast(String message) {
            for (ClientHandler ch : clients) {
                ch.send(message);
            }
        }

        private class ClientHandler implements Runnable {
            private final Socket socket;
            private final String name;
            private PrintWriter out;
            private BufferedReader in;
            private volatile boolean open = true;

            ClientHandler(Socket socket, String name) {
                this.socket = socket;
                this.name = name;
                try {
                    out = new PrintWriter(new OutputStreamWriter(socket.getOutputStream(), "UTF-8"), true);
                    in = new BufferedReader(new InputStreamReader(socket.getInputStream(), "UTF-8"));
                    send("Welcome " + name);
                } catch (IOException e) {
                    close();
                }
            }

            void send(String msg) {
                try {
                    if (out != null) out.println(msg);
                } catch (Exception ignored) {}
            }

            void close() {
                open = false;
                try { if (in != null) in.close(); } catch (IOException ignored) {}
                if (out != null) try { out.close(); } catch (Exception ignored) {}
                try { if (socket != null && !socket.isClosed()) socket.close(); } catch (IOException ignored) {}
                clients.remove(this);
            }

            @Override
            public void run() {
                try {
                    String line;
                    while (open && (line = in.readLine()) != null) {
                        broadcast(name + ": " + line);
                    }
                } catch (IOException ignored) {
                } finally {
                    close();
                }
            }
        }
    }

    // API function to start server with specified port
    public static ChatServer startServer(int port) {
        ChatServer server = new ChatServer(port);
        if (!server.start()) return null;
        return server;
    }

    public static void main(String[] args) throws Exception {
        int port = 50590;
        ChatServer server = startServer(port);
        if (server == null) {
            System.out.println("Failed to start server");
            return;
        }

        // 5 test client cases
        class ClientSim implements Callable<Void> {
            private final String name;
            private final String msg;
            private final int port;

            ClientSim(String name, String msg, int port) {
                this.name = name; this.msg = msg; this.port = port;
            }

            @Override
            public Void call() {
                try (Socket s = new Socket("127.0.0.1", port)) {
                    s.setSoTimeout(1000);
                    PrintWriter out = new PrintWriter(new OutputStreamWriter(s.getOutputStream(), "UTF-8"), true);
                    BufferedReader in = new BufferedReader(new InputStreamReader(s.getInputStream(), "UTF-8"));
                    // Read welcome
                    String line = in.readLine();
                    if (line != null) System.out.println(name + " recv: " + line);
                    out.println(msg);
                    long end = System.currentTimeMillis() + 500;
                    while (System.currentTimeMillis() < end) {
                        try {
                            String l = in.readLine();
                            if (l == null) break;
                            System.out.println(name + " recv: " + l);
                        } catch (SocketTimeoutException ste) {
                            break;
                        }
                    }
                } catch (IOException e) {
                    System.out.println(name + " error: " + e.getMessage());
                }
                return null;
            }
        }

        ExecutorService testPool = Executors.newFixedThreadPool(5);
        List<Future<Void>> futures = new ArrayList<>();
        for (int i = 1; i <= 5; i++) {
            futures.add(testPool.submit(new ClientSim("TClient-" + i, "Hello from TClient-" + i, port)));
        }
        for (Future<Void> f : futures) { try { f.get(); } catch (Exception ignored) {} }
        testPool.shutdown();
        server.stop();
        System.out.println("Server stopped.");
    }
}