
def is_pangram(input_str: str) -> bool:
    """\n    Check if a string is a pangram (contains all 26 letters of the alphabet).\n    \n    Args:\n        input_str: The string to check\n        \n    Returns:\n        True if the string is a pangram, False otherwise\n    """
    # Validate input
    if input_str is None:
        return False
    
    if not isinstance(input_str, str):
        return False
    
    # Track unique letters seen
    letters = set()
    
    for char in input_str:
        # Convert to lowercase and check if it's a letter\n        if 'A' <= char <= 'Z':\n            letters.add(chr(ord(char) + 32))  # Convert to lowercase\n        elif 'a' <= char <= 'z':\n            letters.add(char)\n    \n    # A pangram must contain all 26 letters\n    return len(letters) == 26\n\n\nif __name__ == "__main__":\n    # Test case 1: Classic pangram\n    test1 = "The quick brown fox jumps over the lazy dog"\n    print(f'Test 1: "{test1}" -> {is_pangram(test1)}')\n    \n    # Test case 2: Pangram with numbers and punctuation\n    test2 = "Pack my box with five dozen liquor jugs!!! 123"\n    print(f'Test 2: "{test2}" -> {is_pangram(test2)}')\n    \n    # Test case 3: Not a pangram\n    test3 = "This is not a pangram"\n    print(f'Test 3: "{test3}" -> {is_pangram(test3)}')\n    \n    # Test case 4: Empty string\n    test4 = ""\n    print(f'Test 4: "{test4}" -> {is_pangram(test4)}')\n    \n    # Test case 5: All letters present with special characters\n    test5 = "abcdefghijklmnopqrstuvwxyz!@#$%^&*()"\n    print(f'Test 5: "{test5}" -> {is_pangram(test5)}')
