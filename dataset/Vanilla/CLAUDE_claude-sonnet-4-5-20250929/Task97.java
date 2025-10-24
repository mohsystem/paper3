
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathFactory;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import java.io.File;

public class Task97 {
    public static String executeXPath(String xpathValue, String xmlFileName) {
        try {
            File xmlFile = new File(xmlFileName);
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();
            Document document = builder.parse(xmlFile);
            document.getDocumentElement().normalize();
            
            XPathFactory xPathFactory = XPathFactory.newInstance();
            XPath xpath = xPathFactory.newXPath();
            
            String xpathQuery = String.format("//*[@id='%s']", xpathValue);
            NodeList nodeList = (NodeList) xpath.evaluate(xpathQuery, document, XPathConstants.NODESET);
            
            StringBuilder result = new StringBuilder();
            for (int i = 0; i < nodeList.getLength(); i++) {
                Node node = nodeList.item(i);
                result.append(node.getNodeName()).append(": ").append(node.getTextContent());
                if (i < nodeList.getLength() - 1) {
                    result.append("\\n");
                }
            }
            
            return result.toString();
        } catch (Exception e) {
            return "Error: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println("Test Case 1:");
        System.out.println(executeXPath("1", "test1.xml"));
        System.out.println();
        
        // Test case 2
        System.out.println("Test Case 2:");
        System.out.println(executeXPath("2", "test2.xml"));
        System.out.println();
        
        // Test case 3
        System.out.println("Test Case 3:");
        System.out.println(executeXPath("3", "test3.xml"));
        System.out.println();
        
        // Test case 4
        System.out.println("Test Case 4:");
        System.out.println(executeXPath("item1", "test4.xml"));
        System.out.println();
        
        // Test case 5
        System.out.println("Test Case 5:");
        System.out.println(executeXPath("main", "test5.xml"));
    }
}
