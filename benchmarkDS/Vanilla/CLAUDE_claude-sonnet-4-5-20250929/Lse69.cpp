
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

class Lse69 {
private:
    static const string base64_chars;
    
    static string base64_encode(const vector<unsigned char>& data) {
        string ret;
        int i = 0;
        int j = 0;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];
        int in_len = data.size();
        int idx = 0;

        while (in_len--) {
            char_array_3[i++] = data[idx++];
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for (i = 0; i < 4; i++)
                    ret += base64_chars[char_array_4[i]];
                i = 0;
            }
        }

        if (i) {
            for (j = i; j < 3; j++)
                char_array_3[j] = '\\0';

            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

            for (j = 0; j < i + 1; j++)
                ret += base64_chars[char_array_4[j]];

            while (i++ < 3)
                ret += '=';
        }

        return ret;
    }

public:
    static void uploadImage(const string& inputImagePath, const string& outputFilePath) {
        try {
            // Read the image file
            ifstream imageFile(inputImagePath, ios::binary);
            if (!imageFile.is_open()) {
                cerr << "Error: Could not open input file" << endl;
                return;
            }

            vector<unsigned char> imageData((istreambuf_iterator<char>(imageFile)),
                                           istreambuf_iterator<char>());
            imageFile.close();

            // Encode the image using base64
            string encodedImage = base64_encode(imageData);

            // Write the encoded image to a file
            ofstream outputFile(outputFilePath);
            if (!outputFile.is_open()) {
                cerr << "Error: Could not open output file" << endl;
                return;
            }
            outputFile << encodedImage;
            outputFile.close();

            cout << "Image uploaded and encoded successfully!" << endl;
        } catch (const exception& e) {
            cerr << "Error uploading image: " << e.what() << endl;
        }
    }
};

const string Lse69::base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int main() {
    // Test cases
    cout << "Test Case 1:" << endl;
    Lse69::uploadImage("test_image1.jpg", "encoded_image1.txt");

    cout << "\\nTest Case 2:" << endl;
    Lse69::uploadImage("test_image2.png", "encoded_image2.txt");

    cout << "\\nTest Case 3:" << endl;
    Lse69::uploadImage("test_image3.gif", "encoded_image3.txt");

    cout << "\\nTest Case 4:" << endl;
    Lse69::uploadImage("test_image4.bmp", "encoded_image4.txt");

    cout << "\\nTest Case 5:" << endl;
    Lse69::uploadImage("test_image5.jpeg", "encoded_image5.txt");

    return 0;
}
