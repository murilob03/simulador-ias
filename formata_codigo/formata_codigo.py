import sys

def format_instructions(input_file, output_file):
    with open(input_file, 'r') as f:
        lines = f.readlines()
    
    instructions = [line.replace('\t', '\n') for line in lines]
    formatted_instructions = [instruction.lower() for instruction in instructions]
    

    with open(output_file, 'w') as f:
        f.writelines(formatted_instructions)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python script.py <input_file> <output_file>")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    
    format_instructions(input_file, output_file)
