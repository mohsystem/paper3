
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import java.io.ByteArrayInputStream;

public class Task61 {
    public static String getRootElement(String xmlString) {
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();
            ByteArrayInputStream input = new ByteArrayInputStream(xmlString.getBytes("UTF-8"));
            Document document = builder.parse(input);
            Element root = document.getDocumentElement();
            return root.getNodeName();
        } catch (Exception e) {
            return "Error: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Simple XML
        String xml1 = "<?xml version=\\"1.0\\"?><root><child>value</child></root>";
        System.out.println("Test 1 - Root element: " + getRootElement(xml1));
        
        // Test case 2: XML with attributes
        String xml2 = "<?xml version=\\"1.0\\"?><book id=\\"1\\"><title>Java Programming</title></book>";
        System.out.println("Test 2 - Root element: " + getRootElement(xml2));
        
        // Test case 3: XML with nested elements
        String xml3 = "<?xml version=\\"1.0\\"?><catalog><product><name>Item1</name></product></catalog>";
        System.out.println("Test 3 - Root element: " + getRootElement(xml3));
        
        // Test case 4: XML with different root
        String xml4 = "<?xml version=\\"1.0\\"?><students><student><name>John</name></student></students>";
        System.out.println("Test 4 - Root element: " + getRootElement(xml4));
        
        // Test case 5: Simple root without declaration
        String xml5 = "<data><item>test</item></data>";
        System.out.println("Test 5 - Root element: " + getRootElement(xml5));
    }
}
