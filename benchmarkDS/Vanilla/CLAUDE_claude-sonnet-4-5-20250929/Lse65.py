
from http.server import HTTPServer, BaseHTTPRequestHandler
import cgi
import os
from io import BytesIO

class Lse65:
    @staticmethod
    def handle_file_upload(file_content, filename):
        # Create uploads directory if it doesn't exist\n        if not os.path.exists('uploads'):\n            os.makedirs('uploads')\n        \n        # Save the file\n        filepath = os.path.join('uploads', filename)\n        with open(filepath, 'wb') as f:\n            f.write(file_content)\n        \n        return filepath\n\nclass FileUploadHandler(BaseHTTPRequestHandler):\n    def do_GET(self):\n        if self.path == '/':\n            html = '''
            <html>
            <body>
                <form action="/upload" method="post" enctype="multipart/form-data">
                    <input type="file" name="pdfFile" accept=".pdf">
                    <input type="submit" value="Upload PDF">
                </form>
            </body>
            </html>
            '''\n            self.send_response(200)\n            self.send_header('Content-type', 'text/html')\n            self.end_headers()\n            self.wfile.write(html.encode())\n    \n    def do_POST(self):\n        if self.path == '/upload':\n            content_type = self.headers['Content-Type']\n            \n            if 'multipart/form-data' in content_type:\n                form = cgi.FieldStorage(\n                    fp=self.rfile,\n                    headers=self.headers,\n                    environ={\n                        'REQUEST_METHOD': 'POST',\n                        'CONTENT_TYPE': content_type,\n                    }\n                )\n                \n                if 'pdfFile' in form:\n                    fileitem = form['pdfFile']\n                    if fileitem.filename:\n                        filename = os.path.basename(fileitem.filename)\n                        file_content = fileitem.file.read()\n                        \n                        Lse65.handle_file_upload(file_content, filename)\n                        \n                        response = b'File uploaded successfully!'\n                        self.send_response(200)\n                        self.send_header('Content-type', 'text/html')\n                        self.end_headers()\n                        self.wfile.write(response)\n                        return\n            \n            self.send_response(400)\n            self.end_headers()\n            self.wfile.write(b'Bad Request')\n\ndef main():\n    server_address = ('', 8000)\n    httpd = HTTPServer(server_address, FileUploadHandler)\n    print('Server started on port 8000')\n    print('Open http://localhost:8000 in your browser')\n    httpd.serve_forever()\n\nif __name__ == '__main__':
    main()
