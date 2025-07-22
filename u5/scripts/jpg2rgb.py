from PIL import Image
import numpy as np
import sys

def jpeg_to_rgb888_bin(input_path, output_path, shape=None):
    # Open image and convert to RGB
    img = Image.open(input_path).convert("RGB")

    # Resize to desired size
    if shape:
        img = img.resize(shape, Image.LANCZOS)

    # Get raw RGB888 data
    rgb_data = np.array(img)  # Each pixel = 3 bytes (R, G, B)

    # Write to .bin file
    shape = rgb_data.shape
    with open(output_path, "wb") as f:
        f.write(rgb_data.tobytes())

    print(f"Converted and saved: {output_path} shape={shape}")

    h, w, _ = shape
    return (w, h)


if __name__ == '__main__':
    inp = sys.argv[1]
    outp = sys.argv[2]

    #shape = 96, 96
    shape = None
    shape = jpeg_to_rgb888_bin(inp, outp, shape)

    with open(outp, "rb") as f:
        data = f.read()

    img = Image.frombytes("RGB", shape, data)
    img.show()  # Or save to see the result