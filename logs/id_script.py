import os
import sys
import argparse

import random as rd


def get_new_id(file_tmp: str, dir_path) -> int:
    assert os.path.isdir(dir_path), f"{dir_path} is not a path to dir"

    files = list(filter(lambda file: file.startswith(file_tmp), os.listdir(dir_path)))

    if not files:
        return 0
    
    ids = list(map(int, filter(lambda id: all([sym.isdigit() for sym in id]), map(lambda file: file.split(".")[-1], files))))

    return max(ids) + 1


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Scripts returns number of lines in --file')

    parser.add_argument("src",   type=str, default="None", nargs="?")
    parser.add_argument("--dir", type=str, default="./")

    args = parser.parse_args()

    if args.src == "None":
        args.src = input("Enter file_tmp\n").strip()

    sys.stdout.write(str(get_new_id(args.src + ".", args.dir)))
