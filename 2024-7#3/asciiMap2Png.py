from PIL import Image, ImageDraw
from io import BytesIO


def ascii_to_png(lines):

    width = max(len(line) for line in lines)
    height = len(lines)
    scale = 15

    image = Image.new("RGB", ((width + 1) * scale, (height + 2) * scale), (0, 0, 0))
    draw = ImageDraw.Draw(image)

    colors_map = {
        "X": (240, 0, 0),
        "A": (0, 200, 240),
        "o": (240, 240, 240),
        ".": (0, 0, 0),
        "P": (255, 128, 50),
    }

    for y, line in enumerate(lines):
        for x, char in enumerate(line.strip()):
            color = colors_map.get(char, (0, 0, 0))
            draw.rectangle(
                ((x + 1) * scale, (y + 1) * scale, (x + 2) * scale, (y + 2) * scale),
                fill=color,
                outline=(224, 224, 224),
                width=1,
            )

    output = BytesIO()
    image.save(output, format="PNG")
    output.seek(0)

    return output


# main
if __name__ == "__main__":
    # files
    input_file = "map.txt"
    output_file = "map.png"

    with open(input_file, "r") as file:
        lines = file.readlines()

    #
    output = ascii_to_png(lines)

    # output
    with open(output_file, "wb") as file:
        file.write(output.getvalue())
