import os
import sys


def write_data_to_file(filename: str, adding: str, data: list) -> None:
    dest_file = ".".join(filename.split(".")[:-1]) + adding
    with open(dest_file, mode="w") as dest:
        dest.write(" ".join(data) + "\n")


def clear_file(filename: str) -> None:
    assert os.path.isfile(filename), f"'{filename}' is not a path to file"

    with open(filename, mode="r") as source:
        lines = source.readlines()
    
    new_data = []
    for line in lines:
        line = line.strip().lower()
        line = " ".join(line.split())
        line = "". join(filter(lambda sym: sym.isalnum() or sym == " ", line))

        new_data += line.split()
    
    write_data_to_file(filename, "_cleared.txt",            new_data)
    write_data_to_file(filename, "_cleared++.txt", list(set(new_data)))


if __name__ == "__main__":
    if len(sys.argv) > 1:
        filename = sys.argv[1]
    else:
        filename = input("Enter path to file\n").strip()
    
    clear_file(filename)
