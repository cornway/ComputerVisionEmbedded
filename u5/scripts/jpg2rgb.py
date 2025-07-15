from PIL import Image
import sys

def jpeg_to_rgb888_bin(input_path, output_path, size=(96, 96)):
    # Open image and convert to RGB
    img = Image.open(input_path).convert("RGB")

    # Resize to desired size
    img = img.resize(size, Image.LANCZOS)

    # Get raw RGB888 data
    rgb_data = img.tobytes()  # Each pixel = 3 bytes (R, G, B)

    # Write to .bin file
    with open(output_path, "wb") as f:
        f.write(rgb_data)

    print(f"Converted and saved: {output_path}")


if __name__ == '__main__':
    inp = sys.argv[1]
    outp = sys.argv[2]

    jpeg_to_rgb888_bin(inp, outp)

    with open(outp, "rb") as f:
        data = f.read()

    img = Image.frombytes("RGB", (96, 96), data)
    img.show()  # Or save to see the result