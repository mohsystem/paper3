
BUFFER_SIZE = 100

def handle_input(input_str):
    """\n    Safely handles user input into a fixed-size buffer\n    :param input_str: The input string to be stored\n    :return: The safely stored string (truncated if necessary)\n    """
    if input_str is None:
        return ""
    
    # Ensure input doesn't exceed buffer size\n    if len(input_str) > BUFFER_SIZE:\n        return input_str[:BUFFER_SIZE]\n    \n    return input_str\n\ndef store_in_buffer(input_str):\n    """\n    Stores input into a character buffer safely\n    :param input_str: The input string\n    :return: Byte array buffer with safe content\n    """\n    buffer = bytearray(BUFFER_SIZE)\n    \n    if input_str is None or len(input_str) == 0:\n        return buffer\n    \n    length = min(len(input_str), BUFFER_SIZE)\n    input_bytes = input_str.encode('utf-8')[:length]\n    buffer[:len(input_bytes)] = input_bytes\n    \n    return buffer\n\ndef main():\n    print("Testing Fixed-Size Buffer Input Handler\\n")\n    \n    # Test case 1: Normal input\n    test1 = "Hello, World!"\n    print("Test 1 - Normal input:")\n    print(f"Input: {test1}")\n    print(f"Output: {handle_input(test1)}")\n    print(f"Buffer: {store_in_buffer(test1).decode('utf-8', errors='ignore').rstrip(chr(0))}")\n    print()\n    \n    # Test case 2: Empty input\n    test2 = ""\n    print("Test 2 - Empty input:")\n    print('Input: ""')\n    print(f"Output: {handle_input(test2)}")\n    print()\n    \n    # Test case 3: Input exceeding buffer size\n    test3 = "A" * 150\n    print("Test 3 - Input exceeding buffer size (150 chars):")\n    print(f"Input length: {len(test3)}")\n    result3 = handle_input(test3)\n    print(f"Output length: {len(result3)}")\n    print(f"Truncated: {len(test3) > len(result3)}")\n    print()\n    \n    # Test case 4: Input at buffer boundary\n    test4 = "B" * 100\n    print("Test 4 - Input at buffer boundary (100 chars):")\n    print(f"Input length: {len(test4)}")\n    result4 = handle_input(test4)\n    print(f"Output length: {len(result4)}")\n    print()\n    \n    # Test case 5: Null input\n    test5 = None\n    print("Test 5 - Null input:")\n    print("Input: None")\n    print(f'Output: "{handle_input(test5)}"')

if __name__ == "__main__":
    main()
