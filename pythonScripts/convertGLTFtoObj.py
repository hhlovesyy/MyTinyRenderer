"""Preprocess the Pony Cartoon model

The model is available for download from
    https://sketchfab.com/models/885d9f60b3a9429bb4077cfac5653cf9

The Python Imaging Library is required
    pip install pillow
"""

from __future__ import print_function

import json
import os
import zipfile

from PIL import Image

from utils.gltf import dump_obj_data

SRC_FILENAME = "./models/starModel.gltf"
BIN_FILENAME = "./models/starModel.bin"
DST_DIRECTORY = "./models/outputs/"

OBJ_FILENAMES = [
    "starModel.obj",
]

def process_meshes(zip_file):
    # gltf = json.loads(zip_file.read("scene.gltf"))
    # buffer = zip_file.read("scene.bin")

    gltf = json.loads(open(SRC_FILENAME).read())
    buffer = open(BIN_FILENAME, "rb").read()

    for mesh_index, filename in enumerate(OBJ_FILENAMES):
        if filename:
            obj_data = dump_obj_data(
                gltf, buffer, mesh_index, with_tangent=True
            )
            filepath = os.path.join(DST_DIRECTORY, filename)
            with open(filepath, "w") as f:
                f.write(obj_data)


def main():
    if not os.path.exists(DST_DIRECTORY):
        os.makedirs(DST_DIRECTORY)

    process_meshes(SRC_FILENAME)


if __name__ == "__main__":
    main()
