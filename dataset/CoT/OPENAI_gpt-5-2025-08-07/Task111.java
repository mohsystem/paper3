import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import javax.xml.XMLConstants;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import java.io.*;
import java.net.HttpURLConnection;
import java.net.InetSocketAddress;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

import org.w3c.dom.*;

public class Task111 {
    // Chain-of-Through Step 1: Problem understanding - Implement a minimalist XML-RPC server and a small XML-RPC client for tests.
    // Chain-of-Through Step 2: Security requirements - No external entities, limit request size, input validation, timeouts.
    // Chain-of-Through Step 3/4/5: Secure generation, review, and final output embedded in code.

    // Server methods (accept input as parameters and return outputs)
    public static int add(int a, int b) {
        return Math.addExact(a, b);
    }
    public static int multiply(int a, int b) {
        return Math.multiplyExact(a, b);
    }
    public static String concat(String a, String b) {
        if (a == null) a = "";
        if (b == null) b = "";
        // Mitigate potential log/response split by normalizing newlines
        return (a.replace("\r", "").replace("\n", ""))
             + (b.replace("\r", "").replace("\n", ""));
    }
    public static String echo(String s) {
        if (s == null) return "";
        return s.replace("\r", "").replace("\n", "");
    }
    public static String ping() {
        return "pong";
    }

    static class XmlRpcHandler implements HttpHandler {
        private static final int MAX_REQUEST_SIZE = 100 * 1024; // 100KB cap

        @Override
        public void handle(HttpExchange exchange) throws IOException {
            try {
                if (!"POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                    sendPlain(exchange, 405, "Method Not Allowed");
                    return;
                }
                Headers reqHeaders = exchange.getRequestHeaders();
                String ctype = reqHeaders.getFirst("Content-Type");
                if (ctype == null || !ctype.toLowerCase().contains("text/xml")) {
                    sendPlain(exchange, 415, "Unsupported Media Type");
                    return;
                }

                // Limit read size
                byte[] body = readLimited(exchange.getRequestBody(), MAX_REQUEST_SIZE);
                if (body == null) {
                    sendPlain(exchange, 413, "Request Entity Too Large");
                    return;
                }
                String xml = new String(body, StandardCharsets.UTF_8);

                Document doc = parseXmlSecure(xml);
                if (doc == null) {
                    sendXml(exchange, 400, buildXmlRpcFault(400, "Bad XML"));
                    return;
                }

                String methodName = getMethodName(doc);
                if (methodName == null || methodName.isEmpty()) {
                    sendXml(exchange, 400, buildXmlRpcFault(400, "No methodName"));
                    return;
                }
                List<Object> params = parseParams(doc);

                Object result;
                try {
                    switch (methodName) {
                        case "add":
                            if (params.size() != 2) throw new IllegalArgumentException("Need 2 params");
                            result = add(coerceInt(params.get(0)), coerceInt(params.get(1)));
                            break;
                        case "multiply":
                            if (params.size() != 2) throw new IllegalArgumentException("Need 2 params");
                            result = multiply(coerceInt(params.get(0)), coerceInt(params.get(1)));
                            break;
                        case "concat":
                            if (params.size() != 2) throw new IllegalArgumentException("Need 2 params");
                            result = concat(coerceString(params.get(0)), coerceString(params.get(1)));
                            break;
                        case "echo":
                            if (params.size() != 1) throw new IllegalArgumentException("Need 1 param");
                            result = echo(coerceString(params.get(0)));
                            break;
                        case "ping":
                            if (!params.isEmpty()) throw new IllegalArgumentException("No params");
                            result = ping();
                            break;
                        default:
                            sendXml(exchange, 500, buildXmlRpcFault(404, "Unknown method"));
                            return;
                    }
                } catch (Exception ex) {
                    sendXml(exchange, 500, buildXmlRpcFault(500, "Server error: " + safeMsg(ex.getMessage())));
                    return;
                }

                String resp = buildXmlRpcResponse(result);
                sendXml(exchange, 200, resp);
            } catch (Exception e) {
                sendPlain(exchange, 500, "Internal Server Error");
            } finally {
                try {
                    exchange.close();
                } catch (Exception ignore) {}
            }
        }

        private static String safeMsg(String m) {
            if (m == null) return "";
            return m.replace("\r", "").replace("\n", "");
        }

        private static byte[] readLimited(InputStream in, int max) throws IOException {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            byte[] buf = new byte[4096];
            int total = 0;
            int r;
            while ((r = in.read(buf)) != -1) {
                total += r;
                if (total > max) return null;
                baos.write(buf, 0, r);
            }
            return baos.toByteArray();
        }

        private static Document parseXmlSecure(String xml) {
            try {
                DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
                dbf.setFeature(XMLConstants.FEATURE_SECURE_PROCESSING, true);
                dbf.setFeature("http://apache.org/xml/features/disallow-doctype-decl", true);
                dbf.setFeature("http://xml.org/sax/features/external-general-entities", false);
                dbf.setFeature("http://xml.org/sax/features/external-parameter-entities", false);
                dbf.setXIncludeAware(false);
                dbf.setExpandEntityReferences(false);
                dbf.setNamespaceAware(false);
                DocumentBuilder db = dbf.newDocumentBuilder();
                try (ByteArrayInputStream bais = new ByteArrayInputStream(xml.getBytes(StandardCharsets.UTF_8))) {
                    return db.parse(bais);
                }
            } catch (Exception e) {
                return null;
            }
        }

        private static String getMethodName(Document doc) {
            NodeList nl = doc.getElementsByTagName("methodName");
            if (nl.getLength() == 0) return null;
            return nl.item(0).getTextContent().trim();
        }

        private static List<Object> parseParams(Document doc) {
            List<Object> params = new ArrayList<>();
            NodeList plist = doc.getElementsByTagName("param");
            for (int i = 0; i < plist.getLength(); i++) {
                Node p = plist.item(i);
                Object val = parseValueElement(findFirstByTag((Element) p, "value"));
                params.add(val);
            }
            return params;
        }

        private static Element findFirstByTag(Element parent, String tag) {
            if (parent == null) return null;
            NodeList nl = parent.getElementsByTagName(tag);
            if (nl.getLength() == 0) return null;
            return (Element) nl.item(0);
        }

        private static Object parseValueElement(Element valueEl) {
            if (valueEl == null) return "";
            // Expect one type child or text
            Node child = valueEl.getFirstChild();
            while (child != null && child.getNodeType() != Node.ELEMENT_NODE && child.getNodeType() != Node.TEXT_NODE) {
                child = child.getNextSibling();
            }
            if (child == null) return "";
            if (child.getNodeType() == Node.ELEMENT_NODE) {
                String name = ((Element) child).getTagName();
                String text = child.getTextContent().trim();
                switch (name) {
                    case "int":
                    case "i4":
                        try { return Integer.parseInt(text); } catch (Exception ignore) { return 0; }
                    case "string":
                        return text;
                    case "boolean":
                        return "1".equals(text) || "true".equalsIgnoreCase(text);
                    default:
                        return child.getTextContent();
                }
            } else {
                return child.getTextContent();
            }
        }

        private static int coerceInt(Object o) {
            if (o instanceof Integer) return (Integer) o;
            try {
                return Integer.parseInt(String.valueOf(o));
            } catch (Exception e) {
                throw new IllegalArgumentException("Expected int");
            }
        }
        private static String coerceString(Object o) {
            return o == null ? "" : String.valueOf(o);
        }

        private static String buildXmlRpcResponse(Object result) {
            StringBuilder sb = new StringBuilder();
            sb.append("<?xml version=\"1.0\"?><methodResponse><params><param><value>");
            if (result instanceof Integer) {
                sb.append("<int>").append(result).append("</int>");
            } else if (result instanceof Boolean) {
                sb.append("<boolean>").append(((Boolean) result) ? "1" : "0").append("</boolean>");
            } else {
                sb.append("<string>").append(xmlEscape(String.valueOf(result))).append("</string>");
            }
            sb.append("</value></param></params></methodResponse>");
            return sb.toString();
        }

        private static String buildXmlRpcFault(int code, String message) {
            return "<?xml version=\"1.0\"?><methodResponse><fault><value><struct>" +
                    "<member><name>faultCode</name><value><int>" + code + "</int></value></member>" +
                    "<member><name>faultString</name><value><string>" + xmlEscape(message) + "</string></value></member>" +
                    "</struct></value></fault></methodResponse>";
        }

        private static String xmlEscape(String s) {
            if (s == null) return "";
            StringBuilder out = new StringBuilder(s.length());
            for (char c : s.toCharArray()) {
                switch (c) {
                    case '&': out.append("&amp;"); break;
                    case '<': out.append("&lt;"); break;
                    case '>': out.append("&gt;"); break;
                    case '"': out.append("&quot;"); break;
                    case '\'': out.append("&apos;"); break;
                    default: out.append(c);
                }
            }
            return out.toString();
        }

        private static void sendPlain(HttpExchange ex, int status, String msg) throws IOException {
            byte[] data = msg.getBytes(StandardCharsets.UTF_8);
            ex.getResponseHeaders().set("Content-Type", "text/plain; charset=UTF-8");
            ex.sendResponseHeaders(status, data.length);
            try (OutputStream os = ex.getResponseBody()) {
                os.write(data);
            }
        }

        private static void sendXml(HttpExchange ex, int status, String xml) throws IOException {
            byte[] data = xml.getBytes(StandardCharsets.UTF_8);
            ex.getResponseHeaders().set("Content-Type", "text/xml; charset=UTF-8");
            ex.sendResponseHeaders(status, data.length);
            try (OutputStream os = ex.getResponseBody()) {
                os.write(data);
            }
        }
    }

    // Minimal XML-RPC client for tests (accepts inputs as params and returns output)
    public static Object xmlRpcCall(String url, String method, List<Object> params) throws Exception {
        String req = buildXmlRpcRequest(method, params);
        byte[] data = req.getBytes(StandardCharsets.UTF_8);
        HttpURLConnection conn = (HttpURLConnection) new URL(url).openConnection();
        conn.setConnectTimeout(3000);
        conn.setReadTimeout(3000);
        conn.setRequestMethod("POST");
        conn.setDoOutput(true);
        conn.setRequestProperty("Content-Type", "text/xml; charset=UTF-8");
        conn.setFixedLengthStreamingMode(data.length);
        try (OutputStream os = conn.getOutputStream()) {
            os.write(data);
        }
        int code = conn.getResponseCode();
        InputStream is = (code >= 200 && code < 300) ? conn.getInputStream() : conn.getErrorStream();
        String resp = readAll(is);
        if (code != 200) throw new IOException("HTTP " + code + ": " + resp);

        Document doc = XmlRpcHandler.parseXmlSecure(resp);
        if (doc == null) throw new IOException("Bad XML response");
        // Check fault
        NodeList faults = doc.getElementsByTagName("fault");
        if (faults.getLength() > 0) {
            String msg = doc.getDocumentElement().getTextContent();
            throw new IOException("Fault: " + msg);
        }
        // Extract first param value
        NodeList vals = doc.getElementsByTagName("value");
        if (vals.getLength() == 0) return null;
        return XmlRpcHandler.parseValueElement((Element) vals.item(0));
    }

    private static String readAll(InputStream is) throws IOException {
        if (is == null) return "";
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        byte[] buf = new byte[4096];
        int r;
        while ((r = is.read(buf)) != -1) baos.write(buf, 0, r);
        return baos.toString(StandardCharsets.UTF_8.name());
    }

    private static String buildXmlRpcRequest(String method, List<Object> params) {
        StringBuilder sb = new StringBuilder();
        sb.append("<?xml version=\"1.0\"?>");
        sb.append("<methodCall><methodName>").append(XmlRpcHandler.xmlEscape(method)).append("</methodName>");
        sb.append("<params>");
        if (params != null) {
            for (Object p : params) {
                sb.append("<param><value>");
                if (p instanceof Integer) {
                    sb.append("<int>").append(p).append("</int>");
                } else if (p instanceof Boolean) {
                    sb.append("<boolean>").append(((Boolean) p) ? "1" : "0").append("</boolean>");
                } else {
                    sb.append("<string>").append(XmlRpcHandler.xmlEscape(String.valueOf(p))).append("</string>");
                }
                sb.append("</value></param>");
            }
        }
        sb.append("</params></methodCall>");
        return sb.toString();
    }

    public static void main(String[] args) throws Exception {
        // Start server on random free port
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", 0), 0);
        server.createContext("/RPC2", new XmlRpcHandler());
        server.setExecutor(java.util.concurrent.Executors.newFixedThreadPool(4));
        server.start();
        int port = server.getAddress().getPort();
        String url = "http://127.0.0.1:" + port + "/RPC2";

        // 5 test cases
        System.out.println("Test1 add: " + xmlRpcCall(url, "add", java.util.Arrays.asList(7, 5)));
        System.out.println("Test2 multiply: " + xmlRpcCall(url, "multiply", java.util.Arrays.asList(6, 9)));
        System.out.println("Test3 concat: " + xmlRpcCall(url, "concat", java.util.Arrays.asList("Hello, ", "XML-RPC")));
        System.out.println("Test4 echo: " + xmlRpcCall(url, "echo", java.util.Arrays.asList("safe <message> & chars")));
        System.out.println("Test5 ping: " + xmlRpcCall(url, "ping", java.util.Collections.emptyList()));

        server.stop(0);
        System.exit(0);
    }
}