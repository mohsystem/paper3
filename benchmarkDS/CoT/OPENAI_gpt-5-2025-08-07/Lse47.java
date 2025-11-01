import java.io.*;
import java.lang.annotation.*;
import java.lang.reflect.*;
import java.nio.file.*;
import java.util.*;
import java.util.regex.*;

@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.METHOD)
@interface Route {
    String path();
}

class Response {
    public final int status;
    public final Map<String, String> headers;
    public final byte[] body;

    public Response(int status, Map<String, String> headers, byte[] body) {
        this.status = status;
        this.headers = headers != null ? headers : new HashMap<>();
        this.body = body != null ? body : new byte[0];
    }

    @Override
    public String toString() {
        return "Response{status=" + status + ", headers=" + headers + ", bodyLength=" + body.length + "}";
    }
}

public class Lse47 {
    private static final List<RouteEntry> ROUTES = new ArrayList<>();
    private static final File IMAGES_DIR = new File("images");

    private static class RouteEntry {
        final Pattern pattern;
        final Method method;
        final List<String> params;

        RouteEntry(Pattern p, Method m, List<String> params) {
            this.pattern = p;
            this.method = m;
            this.params = params;
        }
    }

    static {
        registerAnnotatedRoutes();
    }

    private static void registerAnnotatedRoutes() {
        Method[] methods = Lse47.class.getDeclaredMethods();
        for (Method m : methods) {
            if (m.isAnnotationPresent(Route.class)) {
                Route r = m.getAnnotation(Route.class);
                String path = r.path();
                // Convert template like /images/{name} to regex with named-like capture
                List<String> params = new ArrayList<>();
                StringBuilder regex = new StringBuilder("^");
                for (int i = 0; i < path.length(); ) {
                    char c = path.charAt(i);
                    if (c == '{') {
                        int end = path.indexOf('}', i + 1);
                        if (end < 0) throw new IllegalArgumentException("Unmatched { in route path");
                        String name = path.substring(i + 1, end).trim();
                        params.add(name);
                        // do not allow slashes in param
                        regex.append("([^/]+)");
                        i = end + 1;
                    } else {
                        if ("\\.[]{}()*+-?^$|".indexOf(c) >= 0) {
                            regex.append('\\');
                        }
                        regex.append(c);
                        i++;
                    }
                }
                regex.append("$");
                Pattern p = Pattern.compile(regex.toString());
                ROUTES.add(new RouteEntry(p, m, params));
            }
        }
    }

    public static Response serveRequest(String path) {
        for (RouteEntry entry : ROUTES) {
            Matcher matcher = entry.pattern.matcher(path);
            if (matcher.matches()) {
                try {
                    Object[] args = new Object[matcher.groupCount()];
                    for (int i = 0; i < matcher.groupCount(); i++) {
                        args[i] = matcher.group(i + 1);
                    }
                    Object r = entry.method.invoke(null, args);
                    if (r instanceof Response) {
                        return (Response) r;
                    } else {
                        return new Response(500, Map.of("Content-Type", "text/plain"), ("Invalid handler return").getBytes());
                    }
                } catch (Exception e) {
                    return new Response(500, Map.of("Content-Type", "text/plain"), ("Server error").getBytes());
                }
            }
        }
        return new Response(404, Map.of("Content-Type", "text/plain"), ("Not Found").getBytes());
    }

    private static boolean isSafeFilename(String name) {
        if (name == null || name.length() == 0 || name.length() > 255) return false;
        // allow letters, digits, underscore, dash, dot
        if (!name.matches("[A-Za-z0-9._-]+")) return false;
        // no hidden files or dot-only names
        if (name.equals(".") || name.equals("..")) return false;
        return true;
    }

    private static Response sendFileIfExists(String filename) {
        try {
            if (!isSafeFilename(filename)) {
                return new Response(400, Map.of("Content-Type", "text/plain"), "Bad filename".getBytes());
            }
            File base = IMAGES_DIR.getCanonicalFile();
            File target = new File(base, filename).getCanonicalFile();
            if (!target.getPath().startsWith(base.getPath() + File.separator)) {
                return new Response(400, Map.of("Content-Type", "text/plain"), "Invalid path".getBytes());
            }
            if (!target.exists() || !target.isFile()) {
                return new Response(404, Map.of("Content-Type", "text/plain"), "Not Found".getBytes());
            }
            byte[] data = Files.readAllBytes(target.toPath());
            String ct = guessMime(filename);
            Map<String, String> headers = new HashMap<>();
            headers.put("Content-Type", ct);
            headers.put("Content-Length", String.valueOf(data.length));
            return new Response(200, headers, data);
        } catch (IOException e) {
            return new Response(500, Map.of("Content-Type", "text/plain"), "IO Error".getBytes());
        }
    }

    private static String guessMime(String filename) {
        String f = filename.toLowerCase(Locale.ROOT);
        if (f.endsWith(".png")) return "image/png";
        if (f.endsWith(".jpg") || f.endsWith(".jpeg")) return "image/jpeg";
        if (f.endsWith(".gif")) return "image/gif";
        if (f.endsWith(".webp")) return "image/webp";
        return "application/octet-stream";
    }

    @Route(path = "/images/{name}")
    public static Response imageHandler(String name) {
        return sendFileIfExists(name);
    }

    private static void writeBytes(File file, byte[] data) throws IOException {
        file.getParentFile().mkdirs();
        try (FileOutputStream fos = new FileOutputStream(file)) {
            fos.write(data);
        }
    }

    public static void main(String[] args) throws Exception {
        // Prepare test files
        IMAGES_DIR.mkdirs();
        writeBytes(new File(IMAGES_DIR, "hello.png"), new byte[]{(byte)0x89, 'P', 'N', 'G'});
        writeBytes(new File(IMAGES_DIR, "cat.jpg"), new byte[]{(byte)0xFF, (byte)0xD8, (byte)0xFF});
        writeBytes(new File(IMAGES_DIR, "dots..png"), new byte[]{1,2,3,4});

        // 5 test cases
        List<String> tests = List.of(
                "/images/hello.png",            // exists
                "/images/missing.png",          // missing
                "/images/../../secret.txt",     // traversal attempt blocked by route regex
                "/images/dots..png",            // exists with dots
                "/images/CAT.JPG"               // case-sensitive file system may be missing
        );
        for (String t : tests) {
            Response r = serveRequest(t);
            System.out.println(t + " -> " + r);
        }
    }
}