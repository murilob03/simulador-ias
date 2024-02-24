# Simple script to randomize the lines of a file
# Usage: python randomize.py <input_file> <output_file>
# Example: python randomize.py input.txt output.txt

import sys
import random

def randomize(input_file, output_file):
    with open(input_file, 'r') as f:
        lines = f.readlines()
        random.shuffle(lines)
        with open(output_file, 'w') as f:
            f.writelines(lines)

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print('Usage: python randomize.py <input_file> <output_file>')
        sys.exit(1)
    randomize(sys.argv[1], sys.argv[2])
    print('Randomized lines written to', sys.argv[2])