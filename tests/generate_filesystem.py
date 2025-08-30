#!/usr/bin/env python3

import json
import sys
import os
import random
import hashlib
import hmac
import math
import subprocess
import re
import encodings.utf_8
import shutil
from pathlib import Path

def main():
    filesystem_sizes = {
        "very_small": 3,
        "small": 5,
        "normal": 7,
        "large": 10,
        "very_large": 15
    }

    if not len(sys.argv) in [2, 3] and (len(sys.argv) and sys.argv[2] in filesystem_sizes):
        print("Usage: ./generate_filesystem <output_dir> <filesystem_size>")
        print("\tFilesystem sizes: very_small, small, normal, large, very_large")
        os._exit(1)

    random.seed(None)

    os.mkdir(sys.argv[1])

    base_chance = filesystem_sizes[sys.argv[2] if len(sys.argv) == 3 else 'normal']

    cwd = os.getcwd()
    print("Generating random image data...")
    generate_directory(sys.argv[1] + "/imagedata", base_chance)
    os.chdir(cwd)

    print("Indexing image data...")
    size = index_image(sys.argv[1] + "/imagedata", sys.argv[1] + "/index.json")

    print("Generating the image...")
    generate_filesystem(sys.argv[1] + "/imagedata", sys.argv[1] + "/test.img", size)
    
    return

def generate_directory(path: str, chance: int):
    if chance <= 0:
        return

    os.mkdir(path)

    if random.randrange(0, 100) == 0:
        return

    os.chdir(path)

    directory_count = random.randrange(1, 10)
    file_count = random.randrange(0, 50)

    for i in range(0, file_count):
        path_to_file = random_path_component(random.randrange(2, 250))
        generate_random_file(path_to_file)

    for i in range(0, directory_count):
        path_to_dir = random_path_component(random.randrange(2, 250))
        generate_directory(path_to_dir, chance - (random.randrange(1, 4 if chance>4 else chance) if chance>1 else 1))

    os.chdir("..")
        
    return

def generate_random_file(path: str):
    size = random_file_size()
    file = open(path, "wb")

    while size > 0:
        bytes = random.randbytes(1024 if size > 1024 else size)
        file.write(bytes)
        size =- 1024

    file.close()


def index_image(path: str, output: str) -> int:
    out_directories = []
    out_files = []
    total_size = 0

    cwd = os.getcwd()
    os.chdir(path)

    for (root, dirs, files) in os.walk('.',topdown=True):
        root_index = len(out_directories)
        parent_name = re.sub(r'^\.\/?', '', str(Path(root).parent))
        parent = -1
        for i, d in enumerate(out_directories):
            if d["name"] == parent_name:
                parent = i
                break

        out_directories.append({"name": re.sub(r'^\.\/?', '', root), "parent_index": parent})

        for file in files:
            total_size += math.ceil(os.path.getsize(os.path.join(root, file))/4096)*4096
            with open(os.path.join(root, file), "rb") as f:
                digest = hashlib.file_digest(f, "md5")

            out_files.append({"file": file, "root_index": root_index, "md5": digest.hexdigest()})

    os.chdir(cwd)

    output_file = open(output, "w")
    output_file.write(json.dumps({"directories": out_directories, "files": out_files, "size": total_size}))

    return total_size

def generate_filesystem(path: str, output: str, size: int):
    mke2fs_bin = os.getenv('MKE2FS', default="mke2fs")
    size_multiplier = 5

    while True:
        image_size = max(math.ceil((size*size_multiplier) / (1024*1024*1024))+3, 10)
        cmd = "%s -F -L '' -O ^64bit -d %s -t ext2 %s %dG"%(mke2fs_bin, path, output, image_size)
        print("[Running %s]"%cmd)
        try:
            subprocess.run([mke2fs_bin, '-F', '-L', '\'\'', '-O', '^64bit', '-d', path, '-t', 'ext2', output, '%dG'%image_size], check=True)
            break
        except:
            if image_size > 800:
                print("[mke2fs failed, but the image size exceeded the limit. Restarting...]")
                shutil.rmtree(sys.argv[1])
                main()
                os._exit(0)
                
            print("[mke2fs failed, retrying with a bigger image size...]")
            size_multiplier += 1;
            continue

    return

def random_path_component(length) -> str:
    result = ""
    while True:
        char = random_utf8_char()
        new_result = result + char
        if (len(new_result.encode('utf-8')) > 255) or (len(new_result) > length):
            break

        result = new_result

    return result


def random_utf8_char() -> str:
    while True:
        code = random.randint(32, 0x10FFFF);
        if (code > 31 and     # Control chars
            code != 47 and    # /
            code != 127 and   # DEL
            not (0xD800 <= code <= 0xDFFFF)): # Surogate pairs
            return chr(code)

def random_utf8_char() -> str:
    while True:
        code = random.randint(32, 0x10FFFF);
        if (code > 31 and     # Control chars
            code != 47 and    # /
            code != 127 and   # DEL
            not (0xD800 <= code <= 0xDFFFF)): # Surogate pairs
            return chr(code)

def random_file_size() -> int:
    r = random.random()
    if r < 0.70: # Small files (0-100kb)
        return random.randrange(0, 100 * 1024)
    elif r < 0.90: # medium files (100kb-10MB)
        return random.randrange(100 * 1024, 10 * 1024 * 1024)
    elif r < 0.98: # Large files (10MB-4GB)
        return random.randrange(10 * 1024 * 1024, 4 * 1024 * 1024 * 1024)
    else: # Huge files (4GB-16GB)
        return random.randrange(4 * 1024 * 1024 * 1024, 16 * 1024 * 1024 * 1024)
    
if __name__ == "__main__":
    main()
