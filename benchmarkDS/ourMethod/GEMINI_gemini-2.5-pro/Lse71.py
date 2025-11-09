import os
import uuid
from flask import Flask, request, redirect, url_for, flash, render_template_string
from werkzeug.utils import secure_filename

# Define a safe directory for uploads, created in the current working directory.
# In a real application, this path should be carefully configured and outside the web root.
UPLOAD_FOLDER = 'uploads'
# Rule #6: Use allow lists over block lists.
ALLOWED_EXTENSIONS = {'txt', 'pdf', 'png', 'jpg', 'jpeg', 'gif'}
# Define a maximum file size to prevent denial-of-service (16 MB)
MAX_FILE_SIZE = 16 * 1024 * 1024

app = Flask(__name__)

# Rule #11, #63: Never hard code secrets. Read from environment or a secret manager.
# This key is used for flashing messages.
app.config['SECRET_KEY'] = os.environ.get('FLASK_SECRET_KEY', 'a-very-secret-key-for-development')

if app.config['SECRET_KEY'] == 'a-very-secret-key-for-development' and os.environ.get('WERKZEUG_RUN_MAIN') == 'true':
    print("WARNING: Using a default secret key. Set the FLASK_SECRET_KEY environment variable for production.")

app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER
app.config['MAX_CONTENT_LENGTH'] = MAX_FILE_SIZE


def allowed_file(filename):
    """Check if the file has an allowed extension."""
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS


@app.route('/', methods=['GET'])
def upload_form():
    """Serves the file upload form."""
    return render_template_string('''
    <!doctype html>
    <html lang="en">
    <head>
        <meta charset="utf-8">
        <title>Upload new File</title>
    </head>
    <body>
        <h1>Upload new File</h1>
        <form method=post enctype=multipart/form-data action="{{ url_for('upload_file') }}">
          <p>
            <input type=file name=file>
            <input type=submit value=Upload>
          </p>
        </form>
        {% with messages = get_flashed_messages() %}
          {% if messages %}
            <ul class=flashes>
            {% for message in messages %}
              <li>{{ message }}</li>
            {% endfor %}
            </ul>
          {% endif %}
        {% endwith %}
    </body>
    </html>
    ''')


# Rule #5, #8, #9: Validate all inputs at trust boundaries.
@app.route('/upload', methods=['POST'])
def upload_file():
    """Handles the file upload logic."""
    if 'file' not in request.files:
        flash('No file part in the request.')
        return redirect(url_for('upload_form'))
    
    file = request.files['file']

    if file.filename == '':
        flash('No file selected.')
        return redirect(url_for('upload_form'))

    # Rule #62: Validate file type and name.
    if file and allowed_file(file.filename):
        # Rule #7, #43: Sanitize filename to prevent path traversal attacks.
        original_filename = secure_filename(file.filename)
        
        # In case secure_filename returns an empty string (e.g., filename is just ".")
        if not original_filename:
            flash('Invalid filename.')
            return redirect(url_for('upload_form'))

        file_extension = original_filename.rsplit('.', 1)[1].lower()
        
        # Generate a unique filename to prevent overwrites and filename guessing.
        unique_filename = f"{uuid.uuid4().hex}.{file_extension}"
        
        upload_folder_abs = os.path.abspath(app.config['UPLOAD_FOLDER'])
        save_path = os.path.join(upload_folder_abs, unique_filename)
        
        # Rule #45: Defense-in-depth check to ensure the path is within the upload folder.
        if not os.path.abspath(save_path).startswith(upload_folder_abs):
            # This case should be virtually unreachable due to secure_filename
            app.logger.error(f"Path traversal attempt detected: {original_filename}")
            flash('Invalid file path.')
            return redirect(url_for('upload_form'))

        try:
            # Rule #59: Handle exceptions during file operations.
            file.save(save_path)
            # Rule #71: Return generic success message.
            flash(f'File "{original_filename}" uploaded successfully.')
            return redirect(url_for('upload_form'))
        except Exception as e:
            # Rule #12, #71: Log detailed errors, return generic messages.
            app.logger.error(f"Failed to save file '{unique_filename}': {e}")
            flash('An error occurred on the server while saving the file.')
            return redirect(url_for('upload_form'))
    else:
        # Rule #6: Reject on validation failure.
        flash('File type not allowed.')
        return redirect(url_for('upload_form'))


def main():
    """
    Main function to set up and run the web application.
    Includes instructions for manual testing.
    """
    # Create upload directory if it doesn't exist.
    if not os.path.exists(UPLOAD_FOLDER):
        try:
            os.makedirs(UPLOAD_FOLDER)
        except OSError as e:
            print(f"Error creating directory {UPLOAD_FOLDER}: {e}")
            return
    
    print("Starting Flask web server...")
    print(f"File uploads will be saved to the '{os.path.abspath(UPLOAD_FOLDER)}' directory.")
    print("Access the application at http://127.0.0.1:5000")
    print("\n--- Manual Test Cases ---")
    print("1. Success Case: Upload a valid file (e.g., a .png or .txt file).")
    print("2. Invalid Extension: Try to upload a file with a disallowed extension (e.g., .exe).")
    print("3. No File Selected: Click the 'Upload' button without selecting a file.")
    print(f"4. File Too Large: Try to upload a file larger than {MAX_FILE_SIZE / 1024 / 1024} MB.")
    print("5. Malicious Filename: Try to upload a file named '../../secret.txt'. The filename should be sanitized.")
    
    # For production, use a production-ready WSGI server like Gunicorn or uWSGI.
    # Do not use debug=True in a production environment.
    app.run(host='127.0.0.1', port=5000, debug=False)


if __name__ == '__main__':
    main()