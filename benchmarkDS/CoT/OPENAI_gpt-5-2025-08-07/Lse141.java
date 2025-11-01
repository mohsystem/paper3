import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;
import java.net.InetSocketAddress;
import java.sql.Connection;
import java.sql.SQLClientInfoException;
import java.sql.Struct;
import java.sql.Array;
import java.sql.Blob;
import java.sql.CallableStatement;
import java.sql.Clob;
import java.sql.DatabaseMetaData;
import java.sql.NClob;
import java.sql.PreparedStatement;
import java.sql.SQLWarning;
import java.sql.SQLXML;
import java.sql.Savepoint;
import java.sql.Statement;
import java.time.Duration;
import java.util.Map;
import java.util.Properties;
import java.util.concurrent.Executor;

public class Lse141 {

    // Create a minimal web app using JDK's built-in HttpServer
    public static HttpServer createWebApp(int port) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                String resp = "Hello from Lse141 web app";
                exchange.sendResponseHeaders(200, resp.getBytes().length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(resp.getBytes());
                }
            }
        });
        server.setExecutor(null);
        return server;
    }

    public static String getServerName() {
        return "localhost";
    }

    public static String getDatabaseName() {
        return "appdb";
    }

    public static String getDatabaseUser() {
        return "appuser";
    }

    public static String getDatabasePassword() {
        return "secret";
    }

    // Return a mock JDBC Connection via dynamic proxy (no external DB/driver needed)
    public static Connection getDatabaseConnection(String server, String db, String user, String password) {
        class ConnHandler implements InvocationHandler {
            private boolean closed = false;
            @Override
            public Object invoke(Object proxy, Method method, Object[] args) throws Throwable {
                String name = method.getName();
                switch (name) {
                    case "isClosed":
                        return closed;
                    case "close":
                        closed = true;
                        return null;
                    case "isValid":
                        return !closed; // treat as valid if not closed
                    case "toString":
                        return "MockConnection{server=" + server + ", db=" + db + ", user=" + user + "}";
                    case "unwrap":
                        return null;
                    case "isWrapperFor":
                        return false;
                    case "setAutoCommit":
                    case "commit":
                    case "rollback":
                    case "clearWarnings":
                        return null;
                    case "getAutoCommit":
                        return true;
                    case "getMetaData":
                    case "prepareStatement":
                    case "createStatement":
                    case "nativeSQL":
                    case "getWarnings":
                    case "getHoldability":
                    case "getSchema":
                    case "getTypeMap":
                    case "getTransactionIsolation":
                    case "getClientInfo":
                    case "getCatalog":
                    case "getNetworkTimeout":
                    case "getSavepoint":
                    case "getCurrentSchema":
                        return null;
                    case "getClientInfo":
                        if (args == null) return new Properties();
                        return null;
                    case "setClientInfo":
                        return null;
                    default:
                        // For any other method in java.sql.Connection, return a sensible default
                        Class<?> rt = method.getReturnType();
                        if (rt.equals(boolean.class)) return false;
                        if (rt.equals(int.class)) return 0;
                        if (rt.equals(long.class)) return 0L;
                        if (rt.equals(double.class)) return 0.0;
                        return null;
                }
            }
        }
        return (Connection) Proxy.newProxyInstance(
                Lse141.class.getClassLoader(),
                new Class[]{Connection.class},
                new ConnHandler()
        );
    }

    public static void main(String[] args) throws Exception {
        // Start web app on a port, run tests, then stop it
        HttpServer server = createWebApp(8080);
        server.start();

        // 5 test cases
        String s1 = getServerName();
        System.out.println("Test1 - Server Name: " + s1);

        String s2 = getDatabaseName();
        System.out.println("Test2 - Database Name: " + s2);

        String s3 = getDatabaseUser();
        System.out.println("Test3 - Database User: " + s3);

        String s4 = getDatabasePassword();
        System.out.println("Test4 - Database Password (masked): " + "*".repeat(s4.length()));

        Connection conn = getDatabaseConnection(s1, s2, s3, s4);
        System.out.println("Test5 - Connection: " + conn.toString() + ", isValid=" + conn.isValid(0));
        conn.close();

        server.stop(0);
    }
}