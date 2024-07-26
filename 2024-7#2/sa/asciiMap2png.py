from PIL import Image, ImageDraw

input_file = "map.txt"
output_file = "map.png"


def ascii_to_png(input_file, output_file):
    with open(input_file, "r") as file:
        lines = file.readlines()

    width = max(len(line) for line in lines)
    height = len(lines)
    scale = 10

    image = Image.new("RGB", ((width + 1) * scale, (height + 2) * scale), (0, 0, 0))
    draw = ImageDraw.Draw(image)

    colors_map = {"X": (240, 0, 0), "A": (0, 0, 240), "o": (255, 255, 255)}

    for y, line in enumerate(lines):
        for x, char in enumerate(line.strip()):
            color = colors_map.get(char, (0, 0, 0))
            draw.rectangle(
                ((x + 1) * scale, (y + 1) * scale, (x + 2) * scale, (y + 2) * scale),
                fill=color,
            )

    image.save(output_file)


# main
if __name__ == "__main__":
    ascii_to_png(input_file, output_file)
