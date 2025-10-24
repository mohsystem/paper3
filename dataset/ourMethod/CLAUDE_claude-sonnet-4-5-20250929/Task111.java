
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.*;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.*;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

public class Task111 {
    private static final int MAX_REQUEST_SIZE = 1048576; // 1MB
    private static final SecureRandom secureRandom = new SecureRandom();

    public static void main(String[] args) {
        System.out.println("=== XML-RPC Server Test Cases ===\\n");
        
        // Test Case 1: Add two numbers
        String request1 = createXmlRpcRequest("add", new Object[]{5, 3});
        String response1 = handleRequest(request1);
        System.out.println("Test 1 - Add(5, 3):");
        System.out.println("Response: " + response1 + "\\n");

        // Test Case 2: Multiply two numbers
        String request2 = createXmlRpcRequest("multiply", new Object[]{4, 7});
        String response2 = handleRequest(request2);
        System.out.println("Test 2 - Multiply(4, 7):");
        System.out.println("Response: " + response2 + "\\n");

        // Test Case 3: Echo string
        String request3 = createXmlRpcRequest("echo", new Object[]{"Hello World"});
        String response3 = handleRequest(request3);
        System.out.println("Test 3 - Echo('Hello World'):");
        System.out.println("Response: " + response3 + "\\n");

        // Test Case 4: Invalid method
        String request4 = createXmlRpcRequest("invalidMethod", new Object[]{1});
        String response4 = handleRequest(request4);
        System.out.println("Test 4 - Invalid Method:");
        System.out.println("Response: " + response4 + "\\n");

        // Test Case 5: Hash string
        String request5 = createXmlRpcRequest("hashString", new Object[]{"testdata"});
        String response5 = handleRequest(request5);
        System.out.println("Test 5 - HashString('testdata'):");
        System.out.println("Response: " + response5 + "\\n");
    }

    private static String createXmlRpcRequest(String methodName, Object[] params) {
        StringBuilder xml = new StringBuilder();
        xml.append("<?xml version=\\"1.0\\"?>");
        xml.append("<methodCall>");
        xml.append("<methodName>").append(escapeXml(methodName)).append("</methodName>");
        xml.append("<params>");
        for (Object param : params) {
            xml.append("<param><value>");
            if (param instanceof Integer) {
                xml.append("<int>").append(param).append("</int>");
            } else if (param instanceof String) {
                xml.append("<string>").append(escapeXml((String) param)).append("</string>");
            }
            xml.append("</value></param>");
        }
        xml.append("</params>");
        xml.append("</methodCall>");
        return xml.toString();
    }

    private static String escapeXml(String input) {
        if (input == null) return "";
        return input.replace("&", "&amp;")
                   .replace("<", "&lt;")
                   .replace(">", "&gt;")
                   .replace("\\"", "&quot;")
                   .replace("'", "&apos;");
    }

    public static String handleRequest(String xmlRequest) {
        if (xmlRequest == null || xmlRequest.isEmpty()) {
            return createFaultResponse(-32700, "Parse error: Empty request");
        }

        if (xmlRequest.length() > MAX_REQUEST_SIZE) {
            return createFaultResponse(-32700, "Parse error: Request too large");
        }

        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            factory.setFeature("http://xml.org/sax/features/external-general-entities", false);
            factory.setFeature("http://xml.org/sax/features/external-parameter-entities", false);
            factory.setFeature("http://apache.org/xml/features/disallow-doctype-decl", true);
            factory.setFeature("http://apache.org/xml/features/nonvalidating/load-external-dtd", false);
            factory.setXIncludeAware(false);
            factory.setExpandEntityReferences(false);

            DocumentBuilder builder = factory.newDocumentBuilder();
            Document doc = builder.parse(new ByteArrayInputStream(xmlRequest.getBytes(StandardCharsets.UTF_8)));

            NodeList methodNameNodes = doc.getElementsByTagName("methodName");
            if (methodNameNodes.getLength() == 0) {
                return createFaultResponse(-32600, "Invalid Request: No method name");
            }

            String methodName = methodNameNodes.item(0).getTextContent();
            if (methodName == null || methodName.trim().isEmpty()) {
                return createFaultResponse(-32600, "Invalid Request: Empty method name");
            }

            List<Object> params = new ArrayList<>();
            NodeList paramNodes = doc.getElementsByTagName("param");
            
            for (int i = 0; i < paramNodes.getLength(); i++) {
                Element paramElement = (Element) paramNodes.item(i);
                NodeList valueNodes = paramElement.getElementsByTagName("value");
                
                if (valueNodes.getLength() > 0) {
                    Element valueElement = (Element) valueNodes.item(0);
                    NodeList intNodes = valueElement.getElementsByTagName("int");
                    NodeList stringNodes = valueElement.getElementsByTagName("string");
                    
                    if (intNodes.getLength() > 0) {
                        String intValue = intNodes.item(0).getTextContent();
                        try {
                            params.add(Integer.parseInt(intValue));
                        } catch (NumberFormatException e) {
                            return createFaultResponse(-32602, "Invalid params: Invalid integer value");
                        }
                    } else if (stringNodes.getLength() > 0) {
                        params.add(stringNodes.item(0).getTextContent());
                    }
                }
            }

            return executeMethod(methodName, params);

        } catch (Exception e) {
            return createFaultResponse(-32700, "Parse error: Invalid XML");
        }
    }

    private static String executeMethod(String methodName, List<Object> params) {
        try {
            switch (methodName) {
                case "add":
                    if (params.size() != 2 || !(params.get(0) instanceof Integer) || !(params.get(1) instanceof Integer)) {
                        return createFaultResponse(-32602, "Invalid params: add requires two integers");
                    }
                    int sum = (Integer) params.get(0) + (Integer) params.get(1);
                    return createSuccessResponse(sum);

                case "multiply":
                    if (params.size() != 2 || !(params.get(0) instanceof Integer) || !(params.get(1) instanceof Integer)) {
                        return createFaultResponse(-32602, "Invalid params: multiply requires two integers");
                    }
                    int product = (Integer) params.get(0) * (Integer) params.get(1);
                    return createSuccessResponse(product);

                case "echo":
                    if (params.size() != 1 || !(params.get(0) instanceof String)) {
                        return createFaultResponse(-32602, "Invalid params: echo requires one string");
                    }
                    return createSuccessResponse((String) params.get(0));

                case "hashString":
                    if (params.size() != 1 || !(params.get(0) instanceof String)) {
                        return createFaultResponse(-32602, "Invalid params: hashString requires one string");
                    }
                    String hashed = hashWithSalt((String) params.get(0));
                    return createSuccessResponse(hashed);

                default:
                    return createFaultResponse(-32601, "Method not found: " + escapeXml(methodName));
            }
        } catch (Exception e) {
            return createFaultResponse(-32603, "Internal error");
        }
    }

    private static String hashWithSalt(String input) {
        try {
            byte[] salt = new byte[16];
            secureRandom.nextBytes(salt);
            
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            digest.update(salt);
            byte[] hash = digest.digest(input.getBytes(StandardCharsets.UTF_8));
            
            return Base64.getEncoder().encodeToString(salt) + ":" + Base64.getEncoder().encodeToString(hash);
        } catch (Exception e) {
            throw new RuntimeException("Hashing failed");
        }
    }

    private static String createSuccessResponse(Object value) {
        StringBuilder xml = new StringBuilder();
        xml.append("<?xml version=\\"1.0\\"?>");
        xml.append("<methodResponse>");
        xml.append("<params><param><value>");
        
        if (value instanceof Integer) {
            xml.append("<int>").append(value).append("</int>");
        } else if (value instanceof String) {
            xml.append("<string>").append(escapeXml((String) value)).append("</string>");
        }
        
        xml.append("</value></param></params>");
        xml.append("</methodResponse>");
        return xml.toString();
    }

    private static String createFaultResponse(int code, String message) {
        StringBuilder xml = new StringBuilder();
        xml.append("<?xml version=\\"1.0\\"?>");
        xml.append("<methodResponse>");
        xml.append("<fault><value><struct>");
        xml.append("<member><name>faultCode</name><value><int>").append(code).append("</int></value></member>");
        xml.append("<member><name>faultString</name><value><string>").append(escapeXml(message)).append("</string></value></member>");
        xml.append("</struct></value></fault>");
        xml.append("</methodResponse>");
        return xml.toString();
    }
}
