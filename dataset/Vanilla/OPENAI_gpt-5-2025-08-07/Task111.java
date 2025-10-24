import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import java.io.*;
import java.net.HttpURLConnection;
import java.net.InetSocketAddress;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Executors;

import org.w3c.dom.*;
import javax.xml.transform.*;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

public class Task111 {

    // Simple XML-RPC methods
    public static int add(int a, int b) { return a + b; }
    public static int subtract(int a, int b) { return a - b; }
    public static int multiply(int a, int b) { return a * b; }
    public static String concat(String a, String b) { return a + b; }
    public static String ping() { return "pong"; }

    // Server implementation
    static class XmlRpcHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                exchange.sendResponseHeaders(405, -1);
                return;
            }
            String body = readAll(exchange.getRequestBody());
            String responseXml;
            try {
                XmlRequest req = parseXmlRpcRequest(body);
                Object result = dispatch(req.methodName, req.params);
                responseXml = buildXmlRpcResponse(result);
            } catch (Exception e) {
                responseXml = buildXmlRpcFault(1, e.getMessage());
            }
            byte[] bytes = responseXml.getBytes(StandardCharsets.UTF_8);
            Headers headers = exchange.getResponseHeaders();
            headers.set("Content-Type", "text/xml; charset=utf-8");
            exchange.sendResponseHeaders(200, bytes.length);
            OutputStream os = exchange.getResponseBody();
            os.write(bytes);
            os.close();
        }

        private static class XmlRequest {
            String methodName;
            List<Object> params;
            XmlRequest(String m, List<Object> p) { methodName = m; params = p; }
        }

        private static XmlRequest parseXmlRpcRequest(String xml) throws Exception {
            DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
            dbf.setFeature("http://apache.org/xml/features/disallow-doctype-decl", true);
            DocumentBuilder db = dbf.newDocumentBuilder();
            Document doc = db.parse(new ByteArrayInputStream(xml.getBytes(StandardCharsets.UTF_8)));
            Element root = doc.getDocumentElement();
            String methodName = getTextContentByTag(root, "methodName");
            List<Object> params = new ArrayList<>();
            NodeList paramNodes = root.getElementsByTagName("param");
            for (int i = 0; i < paramNodes.getLength(); i++) {
                Element paramEl = (Element) paramNodes.item(i);
                Element valueEl = (Element) paramEl.getElementsByTagName("value").item(0);
                params.add(parseValue(valueEl));
            }
            return new XmlRequest(methodName, params);
        }

        private static String getTextContentByTag(Element el, String tag) {
            NodeList nl = el.getElementsByTagName(tag);
            if (nl.getLength() == 0) return null;
            return nl.item(0).getTextContent();
        }

        private static Object parseValue(Element valueEl) {
            // Value may directly contain text or typed child
            Node child = valueEl.getFirstChild();
            while (child != null && child.getNodeType() == Node.TEXT_NODE && child.getTextContent().trim().isEmpty()) {
                child = child.getNextSibling();
            }
            if (child == null) {
                return null;
            }
            if (child.getNodeType() == Node.ELEMENT_NODE) {
                Element typeEl = (Element) child;
                String tag = typeEl.getTagName();
                String text = typeEl.getTextContent();
                if ("int".equalsIgnoreCase(tag) || "i4".equalsIgnoreCase(tag)) {
                    return Integer.parseInt(text.trim());
                } else if ("string".equalsIgnoreCase(tag)) {
                    return text;
                } else if ("boolean".equalsIgnoreCase(tag)) {
                    return "1".equals(text.trim());
                } else if ("double".equalsIgnoreCase(tag)) {
                    return Double.parseDouble(text.trim());
                } else {
                    return text;
                }
            } else {
                return child.getTextContent();
            }
        }

        private static Object dispatch(String method, List<Object> params) {
            switch (method) {
                case "add":
                    return add(((Number)params.get(0)).intValue(), ((Number)params.get(1)).intValue());
                case "subtract":
                    return subtract(((Number)params.get(0)).intValue(), ((Number)params.get(1)).intValue());
                case "multiply":
                    return multiply(((Number)params.get(0)).intValue(), ((Number)params.get(1)).intValue());
                case "concat":
                    return concat(params.get(0).toString(), params.get(1).toString());
                case "ping":
                    return ping();
                default:
                    throw new RuntimeException("Unknown method: " + method);
            }
        }

        private static String buildXmlRpcResponse(Object result) {
            StringBuilder sb = new StringBuilder();
            sb.append("<?xml version=\"1.0\"?>");
            sb.append("<methodResponse><params><param><value>");
            if (result instanceof Integer) {
                sb.append("<int>").append(result).append("</int>");
            } else if (result instanceof Boolean) {
                sb.append("<boolean>").append(((Boolean) result) ? "1" : "0").append("</boolean>");
            } else if (result instanceof Double || result instanceof Float) {
                sb.append("<double>").append(result).append("</double>");
            } else {
                sb.append("<string>").append(escapeXml(String.valueOf(result))).append("</string>");
            }
            sb.append("</value></param></params></methodResponse>");
            return sb.toString();
        }

        private static String buildXmlRpcFault(int code, String message) {
            StringBuilder sb = new StringBuilder();
            sb.append("<?xml version=\"1.0\"?>");
            sb.append("<methodResponse><fault><value><struct>");
            sb.append("<member><name>faultCode</name><value><int>").append(code).append("</int></value></member>");
            sb.append("<member><name>faultString</name><value><string>").append(escapeXml(message)).append("</string></value></member>");
            sb.append("</struct></value></fault></methodResponse>");
            return sb.toString();
        }

        private static String escapeXml(String s) {
            return s.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")
                    .replace("\"", "&quot;").replace("'", "&apos;");
        }

        private static String readAll(InputStream is) throws IOException {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            byte[] buf = new byte[8192];
            int r;
            while ((r = is.read(buf)) != -1) baos.write(buf, 0, r);
            return baos.toString(StandardCharsets.UTF_8.name());
        }
    }

    // Client helper to invoke XML-RPC
    public static String invokeXmlRpc(String url, String method, List<Object> params) throws Exception {
        String req = buildXmlRpcRequest(method, params);
        HttpURLConnection conn = (HttpURLConnection) new URL(url).openConnection();
        conn.setRequestMethod("POST");
        conn.setDoOutput(true);
        conn.setRequestProperty("Content-Type", "text/xml; charset=utf-8");
        byte[] out = req.getBytes(StandardCharsets.UTF_8);
        conn.setFixedLengthStreamingMode(out.length);
        conn.connect();
        try (OutputStream os = conn.getOutputStream()) {
            os.write(out);
        }
        InputStream is = conn.getResponseCode() >= 400 ? conn.getErrorStream() : conn.getInputStream();
        String resp = readAll(is);
        return parseXmlRpcResponseValue(resp);
    }

    private static String buildXmlRpcRequest(String method, List<Object> params) {
        StringBuilder sb = new StringBuilder();
        sb.append("<?xml version=\"1.0\"?>");
        sb.append("<methodCall>");
        sb.append("<methodName>").append(method).append("</methodName>");
        sb.append("<params>");
        if (params != null) {
            for (Object p : params) {
                sb.append("<param><value>");
                if (p instanceof Integer) {
                    sb.append("<int>").append(p).append("</int>");
                } else if (p instanceof Boolean) {
                    sb.append("<boolean>").append(((Boolean) p) ? "1" : "0").append("</boolean>");
                } else if (p instanceof Double || p instanceof Float) {
                    sb.append("<double>").append(p).append("</double>");
                } else {
                    sb.append("<string>").append(XmlRpcHandler.escapeXml(String.valueOf(p))).append("</string>");
                }
                sb.append("</value></param>");
            }
        }
        sb.append("</params>");
        sb.append("</methodCall>");
        return sb.toString();
    }

    private static String parseXmlRpcResponseValue(String xml) throws Exception {
        DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
        dbf.setFeature("http://apache.org/xml/features/disallow-doctype-decl", true);
        DocumentBuilder db = dbf.newDocumentBuilder();
        Document doc = db.parse(new ByteArrayInputStream(xml.getBytes(StandardCharsets.UTF_8)));
        NodeList fault = doc.getElementsByTagName("fault");
        if (fault.getLength() > 0) {
            return "FAULT";
        }
        NodeList vals = doc.getElementsByTagName("value");
        if (vals.getLength() == 0) return "";
        Element valueEl = (Element) vals.item(0);
        Object v = XmlRpcHandler.parseValue(valueEl);
        return String.valueOf(v);
    }

    private static String readAll(InputStream is) throws IOException {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        byte[] buf = new byte[8192];
        int r;
        while ((r = is.read(buf)) != -1) baos.write(buf, 0, r);
        return baos.toString(StandardCharsets.UTF_8.name());
    }

    // Server start/stop helpers
    public static HttpServer startServer(int port) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/rpc", new XmlRpcHandler());
        server.setExecutor(Executors.newCachedThreadPool());
        server.start();
        return server;
    }

    public static void stopServer(HttpServer server) {
        if (server != null) server.stop(0);
    }

    // Main with 5 test cases
    public static void main(String[] args) throws Exception {
        int port = 9090;
        HttpServer server = startServer(port);
        // Small delay to ensure server readiness
        Thread.sleep(200);

        String url = "http://127.0.0.1:" + port + "/rpc";

        // 5 test cases
        String r1 = invokeXmlRpc(url, "add", List.of(7, 5));
        String r2 = invokeXmlRpc(url, "subtract", List.of(10, 3));
        String r3 = invokeXmlRpc(url, "multiply", List.of(6, 7));
        String r4 = invokeXmlRpc(url, "concat", List.of("Hello, ", "World"));
        String r5 = invokeXmlRpc(url, "ping", List.of());

        System.out.println("add(7,5) = " + r1);
        System.out.println("subtract(10,3) = " + r2);
        System.out.println("multiply(6,7) = " + r3);
        System.out.println("concat('Hello, ', 'World') = " + r4);
        System.out.println("ping() = " + r5);

        stopServer(server);
    }
}