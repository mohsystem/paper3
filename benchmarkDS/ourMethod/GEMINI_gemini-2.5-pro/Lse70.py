import os
import secrets
from flask import Flask, request, flash, redirect, url_for, send_from_directory
from werkzeug.utils import secure_filename
from werkzeug.exceptions import RequestEntityTooLarge

# --- Configuration ---
# It's recommended to load these from environment variables or a config file
# For this example, we define them here.

# Define the directory to store uploaded files. Use an absolute path.
# This ensures the path is not relative to the current working directory.
UPLOAD_FOLDER = os.path.abspath(os.path.join(os.path.dirname(__file__), 'uploads'))

# Define the allowed file extensions. Using an allow list is more secure.
ALLOWED_EXTENSIONS = {'txt', 'pdf', 'png', 'jpg', 'jpeg', 'gif'}

# Maximum file size (e.g., 16 MB). This helps prevent Denial of Service attacks.
MAX_FILE_SIZE = 16 * 1024 * 1024

# --- Flask App Initialization ---
app = Flask(__name__)

# Apply configurations
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER
app.config['MAX_CONTENT_LENGTH'] = MAX_FILE_SIZE
# A secret key is required for flashing messages.
# In a real application, this MUST be a securely generated, random string
# and should NOT be hardcoded. Load it from a secret manager or env variable.
app.config['SECRET_KEY'] = secrets.token_hex(16)

# --- Helper Function ---
def allowed_file(filename):
    """
    Checks if the file's extension is in the allowed list.
    """
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS

# --- Main Application Logic ---
def upload_file_logic(request_object):
    """
    Handles the file upload logic. Separated from the route for clarity.
    Accepts a Flask request object as a parameter.
    Returns a tuple (status_message, category) for flashing.
    """
    # Rule #5: Treat all input as untrusted.
    # Check if the post request has the file part
    if 'file' not in request_object.files:
        return 'No file part in the request.', 'error'

    file = request_object.files['file']

    # If the user does not select a file, the browser submits an
    # empty file without a filename.
    if file.filename == '':
        return 'No file selected.', 'warning'

    # Rule #1, #2: Validate input before use (filename and file type).
    if file and allowed_file(file.filename):
        # Rule #39, #40: Sanitize filename to prevent path traversal.
        # secure_filename handles '../' and other dangerous characters.
        filename = secure_filename(file.filename)
        
        # Rule #41: Ensure the path resolves within the expected directory.
        # os.path.join with an absolute UPLOAD_FOLDER and a sanitized
        # filename is a safe way to construct the final path.
        save_path = os.path.join(app.config['UPLOAD_FOLDER'], filename)

        try:
            # Save the file.
            file.save(save_path)
            return f'File "{filename}" uploaded successfully.', 'success'
        except Exception as e:
            # Rule #63: Log detailed errors, but return generic messages.
            app.logger.error(f"Error saving file: {e}")
            return 'An error occurred while saving the file.', 'error'
            
    else:
        return 'File type not allowed.', 'error'

@app.route('/', methods=['GET', 'POST'])
def upload_file_route():
    """
    Route to display the upload form and handle the POST request.
    """
    if request.method == 'POST':
        try:
            message, category = upload_file_logic(request)
            flash(message, category)
            return redirect(url_for('upload_file_route'))
        except RequestEntityTooLarge:
            flash(f'File is too large. Maximum size is {MAX_FILE_SIZE / 1024 / 1024:.0f} MB.', 'error')
            return redirect(url_for('upload_file_route'))
        except Exception as e:
            app.logger.error(f"An unexpected error occurred: {e}")
            flash('An unexpected server error occurred.', 'error')
            return redirect(url_for('upload_file_route'))
            
    # For GET request, display the upload form.
    return '''
    <!doctype html>
    <html lang="en">
    <head>
        <meta charset="utf-8">
        <title>Secure File Upload</title>
        <style>
            body { font-family: sans-serif; background-color: #f4f4f4; margin: 40px; }
            .container { background-color: #fff; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); max-width: 500px; margin: auto; }
            h1 { color: #333; }
            form { margin-top: 20px; }
            input[type="file"] { border: 1px solid #ccc; padding: 10px; border-radius: 4px; width: calc(100% - 24px); }
            input[type="submit"] { background-color: #007bff; color: white; padding: 10px 15px; border: none; border-radius: 4px; cursor: pointer; font-size: 16px; }
            input[type="submit"]:hover { background-color: #0056b3; }
            .flash { padding: 15px; margin-bottom: 20px; border: 1px solid transparent; border-radius: 4px; }
            .flash.success { color: #155724; background-color: #d4edda; border-color: #c3e6cb; }
            .flash.error { color: #721c24; background-color: #f8d7da; border-color: #f5c6cb; }
            .flash.warning { color: #856404; background-color: #fff3cd; border-color: #ffeeba; }
        </style>
    </head>
    <body>
        <div class="container">
            <h1>Upload a New File</h1>
            {% with messages = get_flashed_messages(with_categories=true) %}
              {% if messages %}
                {% for category, message in messages %}
                  <div class="flash {{ category }}">{{ message }}</div>
                {% endfor %}
              {% endif %}
            {% endwith %}
            <form method=post enctype=multipart/form-data>
              <input type=file name=file>
              <input type=submit value=Upload>
            </form>
            <p>Allowed file types: txt, pdf, png, jpg, jpeg, gif</p>
            <p>Max file size: 16 MB</p>
        </div>
    </body>
    </html>
    '''

# Added a route to view uploaded files for testing purposes.
# In a real application, you would want to secure this route.
@app.route('/uploads/<filename>')
def uploaded_file(filename):
    """
    Serves an uploaded file. This is for demonstration.
    Production systems should serve static files via a dedicated web server like Nginx.
    """
    # Sanitize the filename again before passing it to the file system
    safe_filename = secure_filename(filename)
    if safe_filename != filename:
        return "Invalid filename", 400
    try:
        return send_from_directory(app.config['UPLOAD_FOLDER'],
                                   safe_filename, as_attachment=False)
    except FileNotFoundError:
        return "File not found", 404


def main():
    """
    Main function to set up and run the web application.
    """
    # Create the upload directory if it doesn't exist.
    # Rule #45, #50: Avoid TOCTOU by attempting to create and handling failure.
    # os.makedirs with exist_ok=True is a safe way to do this.
    try:
        os.makedirs(app.config['UPLOAD_FOLDER'], exist_ok=True)
        print(f"Uploads directory is '{app.config['UPLOAD_FOLDER']}'")
    except OSError as e:
        print(f"Error creating directory {app.config['UPLOAD_FOLDER']}: {e}")
        return

    print("\n--- Manual Test Cases ---")
    print("Please run the application and test the following in your browser at http://127.0.0.1:5000")
    print("1. Test Case 1 (Success): Upload a valid file like 'test.txt' or 'image.jpg'.")
    print("2. Test Case 2 (Fail - Type): Upload a disallowed file like 'script.js' or 'program.exe'.")
    print("3. Test Case 3 (Fail - Size): Try to upload a file larger than 16 MB.")
    print("4. Test Case 4 (Fail - No File): Click 'Upload' without selecting a file.")
    print("5. Test Case 5 (Security - Path Traversal): Upload a file named '../../secret.txt'. It should be saved as 'secret.txt' inside the uploads directory.\n")

    # Running with debug=False is crucial for security in production.
    # The reloader and debugger exposed by debug mode can be exploited.
    app.run(host='0.0.0.0', port=5000, debug=False)


if __name__ == '__main__':
    main()