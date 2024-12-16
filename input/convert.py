import sys

def replace_tabs_with_spaces(file_path):
    try:
        with open(file_path, 'r') as file:
            content = file.read()
        
        content = content.replace('\t', ' ')
        
        with open(file_path, 'w') as file:
            file.write(content)
        
        print(f"Successfully replaced all tabs with spaces in {file_path}")
    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python convert.py <file_path>")
    else:
        replace_tabs_with_spaces(sys.argv[1])