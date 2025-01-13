import argparse
import random
import cv2 as cv
import numpy as np
from PIL import Image
import copy

circle = [
    [0, 0, 0, 0, 0, 0, 0],
    [0, 0, 1, 1, 1, 0, 0],
    [0, 1, 0, 0, 0, 1, 0],
    [0, 1, 0, 0, 0, 1, 0],
    [0, 1, 0, 0, 0, 1, 0],
    [0, 0, 1, 1, 1, 0, 0],
    [0, 0, 0, 0, 0, 0, 0],
]

triangle = [
    [0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 1, 0, 0, 0],
    [0, 0, 1, 0, 1, 0, 0],
    [0, 1, 0, 0, 0, 1, 0],
    [1, 0, 0, 0, 0, 0, 1],
    [1, 1, 1, 1, 1, 1, 1],
    [0, 0, 0, 0, 0, 0, 0],
]

square = [
    [0, 0, 0, 0, 0, 0, 0],
    [0, 1, 1, 1, 1, 1, 0],
    [0, 1, 0, 0, 0, 1, 0],
    [0, 1, 0, 0, 0, 1, 0],
    [0, 1, 0, 0, 0, 1, 0],
    [0, 1, 1, 1, 1, 1, 0],
    [0, 0, 0, 0, 0, 0, 0],
]

def add_noize(const_arr, number_of_bits):
    arr = copy.deepcopy(const_arr)
    inv = random.sample(range(0, 48), number_of_bits)
    for b in inv:
        arr[b // 7][b % 7] = int(arr[b // 7][b % 7] ^ 1)
    return arr


def geometra(noize_rate):
    tdata = []

    which_figure = random.randrange(3)

    if which_figure == 2:
        tset = []
        circle_noized = add_noize(circle, noize_rate)
        for r in range(7):
            for c in range(7):
                tset.append(int(circle_noized[r][c]))
        tset.extend([1.0, 0.0, 0.0])
        tdata.append(tset)
        show_figure_from_train_set(tset, True)
    elif which_figure == 1:
        tset = []
        triangle_noized = add_noize(triangle, noize_rate)
        for r in range(7):
            for c in range(7):
                tset.append(int(triangle_noized[r][c]))
        tset.extend([0.0, 1.0, 0.0])
        tdata.append(tset)
        show_figure_from_train_set(tset, True)
    else:
        tset = []
        square_noized = add_noize(square, noize_rate)
        for r in range(7):
            for c in range(7):
                tset.append(int(square_noized[r][c]))
        tset.extend([0.0, 0.0, 1.0])
        tdata.append(tset)
        show_figure_from_train_set(tset, True)

    return tdata


def write_geometra_file(fname, tdata):
    f = open(fname, mode="w", encoding="UTF-8")
#    f.write("%d %d %d\n" % (49, 3, len(tdata)))
    for t in tdata:
        for v in t:
            f.write("%d" % v)
        f.write("\n")
    f.close()


def show_figure_from_train_set(sample, wait: bool):
    print(sample)
    result = sample[-3:]
    answer = ""
    if result[0] == 1:
        answer = "Circle"
    elif result[1] == 1:
        answer = "Triangle"
    elif result[2] == 1:
        answer = "Square"
    else:
        answer = "Unknown"
    print(answer)

    a = np.array(sample[:-3])
    img = np.reshape(a, (-1, 7))
    img = np.array(img, dtype=np.double)
    img = cv.resize(img, (100, 100), fx=0, fy=0, interpolation=cv.INTER_NEAREST)
    cv.imshow(answer, img)
    if wait:
        cv.waitKey(0)
    else:
        cv.waitKey(5)
    cv.destroyWindow('i')

def show_figure_from_file(fname: str):
    with open(fname, encoding="UTF-8") as img:
        l = img.readline().split()
        img = []
        for i in l:
            img.append(int(i))
        
        show_figure_from_train_set(img,True)

def write_figure_to_file(fname: str):
    tdat = geometra(1)
    write_geometra_file('generated/' + fname, tdat)
    pass

if __name__ == '__main__':
    parser = argparse.ArgumentParser("Image reader and writer")
    parser.add_argument("-f", "--file", help="File name of figure to read or to write", type=str, required=True)
    parser.add_argument("--write", help="", action='store_true', required = False)
    args = parser.parse_args()

    fname = args.file

    if args.write:
        write_figure_to_file(fname)
    else:
        show_figure_from_file(fname)
