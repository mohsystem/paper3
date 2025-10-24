
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import java.io.ByteArrayInputStream;
import java.nio.charset.StandardCharsets;

public class Task61 {
    public static String parseAndGetRootElement(String xmlString) {
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            
            // Security configurations
            factory.setFeature("http://apache.org/xml/features/disallow-doctype-decl", true);
            factory.setFeature("http://xml.org/sax/features/external-general-entities", false);
            factory.setFeature("http://xml.org/sax/features/external-parameter-entities", false);
            factory.setFeature("http://apache.org/xml/features/nonvalidating/load-external-dtd", false);
            factory.setXIncludeAware(false);
            factory.setExpandEntityReferences(false);
            
            DocumentBuilder builder = factory.newDocumentBuilder();
            ByteArrayInputStream input = new ByteArrayInputStream(
                xmlString.getBytes(StandardCharsets.UTF_8));
            Document doc = builder.parse(input);
            Element root = doc.getDocumentElement();
            return root.getNodeName();
        } catch (Exception e) {
            return "Error: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Simple XML
        String test1 = "<?xml version=\\"1.0\\"?><root><child>value</child></root>";
        System.out.println("Test 1 - Root element: " + parseAndGetRootElement(test1));
        
        // Test case 2: XML with attributes
        String test2 = "<?xml version=\\"1.0\\"?><books><book id=\\"1\\">Java</book></books>";
        System.out.println("Test 2 - Root element: " + parseAndGetRootElement(test2));
        
        // Test case 3: Complex XML
        String test3 = "<?xml version=\\"1.0\\"?><company><employee><name>John</name></employee></company>";
        System.out.println("Test 3 - Root element: " + parseAndGetRootElement(test3));
        
        // Test case 4: XML with namespaces
        String test4 = "<?xml version=\\"1.0\\"?><ns:document xmlns:ns=\\"http://example.com\\"><ns:data>test</ns:data></ns:document>";
        System.out.println("Test 4 - Root element: " + parseAndGetRootElement(test4));
        
        // Test case 5: Single element XML
        String test5 = "<?xml version=\\"1.0\\"?><message/>";
        System.out.println("Test 5 - Root element: " + parseAndGetRootElement(test5));
    }
}
