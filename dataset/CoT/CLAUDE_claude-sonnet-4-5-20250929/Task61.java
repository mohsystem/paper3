
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import java.io.ByteArrayInputStream;
import java.nio.charset.StandardCharsets;

public class Task61 {
    public static String parseAndGetRootElement(String xmlString) {
        if (xmlString == null || xmlString.trim().isEmpty()) {
            return "Error: Empty or null XML string";
        }
        
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            
            // Security: Disable XXE attacks
            factory.setFeature("http://apache.org/xml/features/disallow-doctype-decl", true);
            factory.setFeature("http://xml.org/sax/features/external-general-entities", false);
            factory.setFeature("http://xml.org/sax/features/external-parameter-entities", false);
            factory.setFeature("http://apache.org/xml/features/nonvalidating/load-external-dtd", false);
            factory.setXIncludeAware(false);
            factory.setExpandEntityReferences(false);
            
            DocumentBuilder builder = factory.newDocumentBuilder();
            ByteArrayInputStream input = new ByteArrayInputStream(
                xmlString.getBytes(StandardCharsets.UTF_8)
            );
            
            Document document = builder.parse(input);
            Element rootElement = document.getDocumentElement();
            
            return rootElement.getNodeName();
            
        } catch (Exception e) {
            return "Error: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Simple XML
        String test1 = "<?xml version=\\"1.0\\"?><root><child>data</child></root>";
        System.out.println("Test 1 - Root element: " + parseAndGetRootElement(test1));
        
        // Test case 2: Complex XML
        String test2 = "<?xml version=\\"1.0\\" encoding=\\"UTF-8\\"?><books><book id=\\"1\\"><title>Java</title></book></books>";
        System.out.println("Test 2 - Root element: " + parseAndGetRootElement(test2));
        
        // Test case 3: Different root element
        String test3 = "<catalog><item>Product</item></catalog>";
        System.out.println("Test 3 - Root element: " + parseAndGetRootElement(test3));
        
        // Test case 4: Malformed XML
        String test4 = "<root><unclosed>";
        System.out.println("Test 4 - Root element: " + parseAndGetRootElement(test4));
        
        // Test case 5: Empty string
        String test5 = "";
        System.out.println("Test 5 - Root element: " + parseAndGetRootElement(test5));
    }
}
