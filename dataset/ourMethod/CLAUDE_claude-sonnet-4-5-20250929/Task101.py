
import os
import tempfile
from pathlib import Path
from typing import Optional

def create_executable_script(base_directory: str, filename: str) -> bool:
    if not base_directory or not filename:
        return False
    
    if ".." in filename or "/" in filename or "\\\\" in filename:\n        return False\n    \n    try:\n        base_path = Path(base_directory).resolve()\n        file_path = (base_path / filename).resolve()\n        \n        if not str(file_path).startswith(str(base_path)):\n            return False\n        \n        if file_path.exists():\n            if not file_path.is_file() or file_path.is_symlink():\n                return False\n        \n        fd, temp_path = tempfile.mkstemp(suffix=".sh", dir=base_path, text=False)\n        try:\n            os.write(fd, b"#!/bin/bash\
")\n            os.close(fd)\n            \n            os.chmod(temp_path, 0o755)\n            \n            os.replace(temp_path, str(file_path))\n            \n            return True\n        except Exception:\n            try:\n                os.close(fd)\n            except Exception:\n                pass\n            try:\n                os.unlink(temp_path)\n            except Exception:\n                pass\n            raise\n    except (OSError, ValueError):\n        return False\n\ndef main() -> None:\n    test_dir = tempfile.gettempdir()\n    \n    print(f"Test 1 (valid): {create_executable_script(test_dir, 'script.sh')}")\n    print(f"Test 2 (valid different name): {create_executable_script(test_dir, 'test_script.sh')}")\n    print(f"Test 3 (path traversal attempt): {create_executable_script(test_dir, '../script.sh')}")\n    print(f"Test 4 (null filename): {create_executable_script(test_dir, '')}")\n    print(f"Test 5 (invalid char): {create_executable_script(test_dir, 'dir/script.sh')}")\n\nif __name__ == "__main__":
    main()
