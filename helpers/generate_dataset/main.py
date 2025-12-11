import os
import random
import glob
import numpy as np
from PIL import Image, ImageDraw, ImageFont, ImageOps
from tqdm import tqdm

OUTPUT_DIR = "./dataset"
FONT_DIR = "./fonts"
IMAGE_SIZE = (28, 28)
SAMPLES_PER_LETTER = 100
LETTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

NOISE_HOLES_PROB = 0.05  # Probability that a pixel of the letter becomes white (hole)
NOISE_BORDER_PROB = 0.02  # Probability of black pixels on the borders
BORDER_WIDTH = 4  # Width of the "dirty" area on the left and right


def create_directory_structure():
    if not os.path.exists(OUTPUT_DIR):
        os.makedirs(OUTPUT_DIR)
    for char in LETTERS:
        dir_path = os.path.join(OUTPUT_DIR, char)
        if not os.path.exists(dir_path):
            os.makedirs(dir_path)


def get_fonts():
    fonts = glob.glob(os.path.join(FONT_DIR, "*.ttf"))
    if not fonts:
        print("Error: No fonts found in ./fonts. Did you run 'nix develop'?")
        exit(1)
    return fonts


def add_noise(image_array):
    height, width = image_array.shape

    mask_ink = image_array < 128

    random_matrix = np.random.rand(height, width)

    holes_mask = mask_ink & (random_matrix < NOISE_HOLES_PROB)
    image_array[holes_mask] = 255

    noise_matrix = np.random.rand(height, width)

    left_mask = np.zeros_like(image_array, dtype=bool)
    left_mask[:, :BORDER_WIDTH] = True

    right_mask = np.zeros_like(image_array, dtype=bool)
    right_mask[:, -BORDER_WIDTH:] = True

    border_mask = (
        (left_mask | right_mask)
        & (image_array > 128)
        & (noise_matrix < NOISE_BORDER_PROB)
    )

    image_array[border_mask] = 0

    return image_array


def generate_image(char, font_path, output_path):
    img = Image.new("L", IMAGE_SIZE, color=255)
    draw = ImageDraw.Draw(img)

    font_size = random.randint(18, 24)
    try:
        font = ImageFont.truetype(font_path, font_size)
    except OSError:
        return

    try:
        bbox = draw.textbbox((0, 0), char, font=font)

        text_width = bbox[2] - bbox[0]
        text_height = bbox[3] - bbox[1]

        if text_width == 0 or text_height == 0:
            return

        x = (IMAGE_SIZE[0] - text_width) / 2 - bbox[0]
        y = (IMAGE_SIZE[1] - text_height) / 2 - bbox[1]

        draw.text((x, y), char, font=font, fill=0)

    except (OSError, ValueError):
        return

    try:
        img_array = np.array(img)
        noisy_array = add_noise(img_array)

        final_img = Image.fromarray(noisy_array)
        final_img.save(output_path)
    except Exception as e:
        print(f"Error saving image: {e}")


def main():
    create_directory_structure()
    fonts = get_fonts()

    print(
        f"Generating dataset: {len(LETTERS)} classes, {SAMPLES_PER_LETTER} images/class"
    )
    print(f"Using {len(fonts)} different fonts.")

    total_images = len(LETTERS) * SAMPLES_PER_LETTER

    with tqdm(total=total_images) as pbar:
        for char in LETTERS:
            for i in range(SAMPLES_PER_LETTER):
                # Choose a random font
                font = random.choice(fonts)
                filename = f"{char}_{i}.png"
                path = os.path.join(OUTPUT_DIR, char, filename)

                generate_image(char, font, path)
                pbar.update(1)

    print("\nGeneration finished in ./dataset")


if __name__ == "__main__":
    main()
