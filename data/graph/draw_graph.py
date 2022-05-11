import os
import sys
import argparse

from math import ceil

import numpy as np
import matplotlib.pyplot as plt

NCOL = 3


def draw_graph(src: str, dest: str) -> None:
    assert os.path.isfile(src),  f"'{src}'  is not a path to file"

    with open(src, mode="r") as src_file:
        data        = src_file.readlines()
        rows, cols  = ceil(len(data) / NCOL) + 1, NCOL

    fig = plt.figure(figsize=(20, 5 * rows))

    coefs  = [ ]
    for i, note in enumerate(data, 1):
        name, values, coef = note.split(";")

        values = np.array(list(map(int, values.split())))

        coefs.append(float(coef))

        ax = fig.add_subplot(rows, cols, i)
        ax.set_title(f"{i}. " + name, size=25)

        plt.bar(np.arange(0, len(values)), values, width=1)
    
    ax = fig.add_subplot(rows, cols, rows * cols - 1)
    ax.set_title("Collision coefficient", size=25)

    plt.bar(range(len(coefs)), coefs, width=1)
    plt.xticks(range(len(coefs)), range(1, len(coefs) + 1))

    for i in range(len(coefs)):
        plt.text(i, coefs[i], f"{coefs[i]:.2f}")

    plt.savefig(dest)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Scripts returns number of lines in --file')

    parser.add_argument("src",   type=str, default="None", nargs="?")
    parser.add_argument("dest",  type=str, default="None", nargs="?")
    parser.add_argument("--dir", type=str, default="./")

    args = parser.parse_args()
    print(args)

    src  = args.dir + args.src
    dest = args.dir + args.dest

    if not os.path.isfile(src):
        if args.src  != "None": print(f"'{src}'  is not a path to file.")
        src  = input("Enter path to src  file\n").strip()
    
    if args.dest == "None":
        print(f"'{dest}' is not a path to file.")
        dest = input("Enter path to dest file\n").strip()
    
    draw_graph(src, dest)
