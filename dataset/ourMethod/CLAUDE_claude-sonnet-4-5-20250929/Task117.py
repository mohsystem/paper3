
import os
import zipfile
import tarfile
from pathlib import Path
from typing import Optional

MAX_FILE_SIZE = 100 * 1024 * 1024  # 100MB
MAX_TOTAL_SIZE = 500 * 1024 * 1024  # 500MB
MAX_FILES = 10000
BUFFER_SIZE = 8192


def extract_archive(archive_path: str, dest_dir: str) -> bool:
    """\n    Extracts ZIP or TAR archive contents to destination directory.\n    \n    Args:\n        archive_path: Path to the archive file\n        dest_dir: Destination directory for extraction\n        \n    Returns:\n        True if extraction successful, False otherwise\n    """
    if not archive_path or not dest_dir:
        return False
    
    try:
        archive_path_obj = Path(archive_path)
        dest_path_obj = Path(dest_dir)
        
        if not archive_path_obj.exists() or not archive_path_obj.is_file():
            return False
        
        dest_path_obj.mkdir(parents=True, exist_ok=True)
        dest_normalized = dest_path_obj.resolve()
        
        archive_lower = archive_path.lower()
        if archive_lower.endswith('.zip'):
            return extract_zip(archive_path_obj, dest_normalized)
        elif archive_lower.endswith('.tar'):
            return extract_tar(archive_path_obj, dest_normalized)
        
        return False
    except (OSError, ValueError):
        return False


def extract_zip(archive_path: Path, dest_dir: Path) -> bool:
    """Extract ZIP archive with security validation."""
    total_extracted = 0
    file_count = 0
    
    try:
        with zipfile.ZipFile(archive_path, 'r') as zf:
            for member in zf.infolist():
                if file_count >= MAX_FILES:
                    return False
                
                name = member.filename
                if not name or '..' in name or name.startswith('/') or name.startswith('\\\\'):\n                    continue\n                \n                target_path = (dest_dir / name).resolve()\n                \n                if not str(target_path).startswith(str(dest_dir)):\n                    continue\n                \n                if member.is_dir():\n                    target_path.mkdir(parents=True, exist_ok=True)\n                    continue\n                \n                if member.file_size > MAX_FILE_SIZE:\n                    continue\n                \n                target_path.parent.mkdir(parents=True, exist_ok=True)\n                \n                temp_path = dest_dir / f".tmp_{os.urandom(8).hex()}"\n                bytes_written = 0\n                \n                try:\n                    with zf.open(member) as source, open(temp_path, 'wb') as target:\n                        while True:\n                            chunk = source.read(BUFFER_SIZE)\n                            if not chunk:\n                                break\n                            \n                            bytes_written += len(chunk)\n                            total_extracted += len(chunk)\n                            \n                            if bytes_written > MAX_FILE_SIZE or total_extracted > MAX_TOTAL_SIZE:\n                                temp_path.unlink()\n                                return False\n                            \n                            target.write(chunk)\n                        \n                        target.flush()\n                        os.fsync(target.fileno())\n                    \n                    temp_path.replace(target_path)\n                    file_count += 1\n                    \n                except Exception:\n                    if temp_path.exists():\n                        temp_path.unlink()\n                    raise\n                \n        return True\n    except (zipfile.BadZipFile, OSError, ValueError):\n        return False\n\n\ndef extract_tar(archive_path: Path, dest_dir: Path) -> bool:\n    """Extract TAR archive with security validation."""\n    total_extracted = 0\n    file_count = 0\n    \n    try:\n        with tarfile.open(archive_path, 'r') as tf:\n            for member in tf.getmembers():\n                if file_count >= MAX_FILES:\n                    return False\n                \n                name = member.name\n                if not name or '..' in name or name.startswith('/') or member.size > MAX_FILE_SIZE:\n                    continue\n                \n                target_path = (dest_dir / name).resolve()\n                \n                if not str(target_path).startswith(str(dest_dir)):\n                    continue\n                \n                if member.isdir():\n                    target_path.mkdir(parents=True, exist_ok=True)\n                    continue\n                \n                if not member.isfile():\n                    continue\n                \n                target_path.parent.mkdir(parents=True, exist_ok=True)\n                \n                temp_path = dest_dir / f".tmp_{os.urandom(8).hex()}"\n                bytes_written = 0\n                \n                try:\n                    with tf.extractfile(member) as source, open(temp_path, 'wb') as target:
                        if source is None:
                            continue
                        
                        while True:
                            chunk = source.read(BUFFER_SIZE)
                            if not chunk:
                                break
                            
                            bytes_written += len(chunk)
                            total_extracted += len(chunk)
                            
                            if total_extracted > MAX_TOTAL_SIZE:
                                temp_path.unlink()
                                return False
                            
                            target.write(chunk)
                        
                        target.flush()
                        os.fsync(target.fileno())
                    
                    temp_path.replace(target_path)
                    file_count += 1
                    
                except Exception:
                    if temp_path.exists():
                        temp_path.unlink()
                    raise
                
        return True
    except (tarfile.TarError, OSError, ValueError):
        return False


def main():
    print("Test 1 - Extract valid ZIP:")
    print(extract_archive("test.zip", "output1"))
    
    print("\\nTest 2 - Extract valid TAR:")
    print(extract_archive("test.tar", "output2"))
    
    print("\\nTest 3 - Invalid archive path:")
    print(extract_archive("nonexistent.zip", "output3"))
    
    print("\\nTest 4 - Empty parameters:")
    print(extract_archive("", "output4"))
    
    print("\\nTest 5 - None parameters:")
    print(extract_archive(None, "output5"))


if __name__ == "__main__":
    main()
