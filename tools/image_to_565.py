#! /usr/bin/env python
# -*- coding: utf-8 -*-

from PIL import Image
import struct, os, sys

def usage():
    # print('./png2rgb565.py HOGE.png') I need to print the current path
    sys.exit(1)
    
def error(msg):
    print(msg)
    sys.exit(-1)
    
def write_bin(f, pixel_list):
    for pix in pixel_list:
        # right shift by 3 to remove the 3 less useful bits
        # the 0x1F is just 11111 to check that everything worked correctly
        r = (pix[0] >> 3) & 0x1F
        # two less useful
        g = (pix[1] >> 2) & 0x3F
        # three less useful
            # this is because we need 565 bits 
        b = (pix[2] >> 3) & 0x1F
        # writes the bits in little endian left shifted by 11, 5 and 0
            # this is because we need to populate the 16 bit var
            # 5 red bits, 6 green, 5 blue
        f.write(struct.pack('<H', (r << 11) + (g << 5) + b))


if __name__ == '__main__':
    args = sys.argv
    if len(args) != 2: 
        usage()
    in_path = args[1]
    if os.path.exists(in_path) == False: 
        error('not exists: ' + in_path)
    
    body, _ = os.path.splitext(in_path)
    in_dir = os.path.dirname(in_path)      # folder the PNG lives in
    filename = os.path.basename(body)      # strip any directory from the input path
    out_dir = os.path.join(in_dir, "data") # "data" next to the PNG, not next to cwd
    os.makedirs(out_dir, exist_ok=True)    # creates it if it doesn't exist, no error if it does
    out_path = os.path.join(out_dir, filename + ".bin")

    img = Image.open(in_path).convert('RGB')
    pixels = list(img.getdata())
    # print pixels
    
    with open(out_path, 'wb') as f:
        write_bin(f, pixels)