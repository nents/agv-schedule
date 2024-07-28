import os
import cv2
import asciiMap2Png
import numpy as np
from PIL import Image, ImageDraw, ImageFont
from io import BytesIO
from moviepy.editor import VideoFileClip, clips_array, CompositeVideoClip

map_file = "map.txt"
sa4lowerbound_path_file = "sa4lowerbound_path.txt"
greedy4simulate_path_file = "greedy4simulate_path.txt"
sa4lowerbound_name = "sa4lowerbound"
greedy4simulate_name = "greedy4simulate"
FPS = 6

defalut_position = (1, 1)


def create_mp4(map_file, path_file, output_name, fps):
    # read map
    map_lines = []
    with open(map_file, "r") as file:
        map_lines = file.readlines()

    # read path
    paths = []
    with open(path_file, "r") as file:
        path_lines = file.read().splitlines()
        for line in path_lines:
            numbers = [int(x) for x in line.split()]
            positions = [
                (numbers[i], numbers[i + 1]) for i in range(0, len(numbers), 2)
            ]
            paths.append(positions)
    max_line = max(len(row) for row in paths)

    # transfer to images
    images = []
    for index in range(0, max_line):
        now_map = [list(row) for row in map_lines.copy()]
        for row in paths:
            agv = row[index] if index < len(row) else defalut_position
            now_map[agv[0]][agv[1]] = "A"
        now_map = ["".join(row) for row in now_map]
        # append a title
        titled_image = Image.open(asciiMap2Png.ascii_to_png(now_map))
        new_height = titled_image.height + 30
        new_image = Image.new("RGB", (titled_image.width, new_height), color="black")
        new_image.paste(titled_image, (0, 30))
        draw = ImageDraw.Draw(new_image)
        font = ImageFont.load_default(size=20)
        draw.text((10, 10), f"{output_name} - {index}", font=font, fill="white")
        output = BytesIO()
        new_image.save(output, format="PNG")
        output.seek(0)
        images.append(output)

    # ouput to mp4
    os.makedirs("output", exist_ok=True)

    nparr = np.frombuffer(images[0].getvalue(), np.uint8)
    frame = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
    height, width, layers = frame.shape

    fourcc = cv2.VideoWriter_fourcc(*"mp4v")
    video = cv2.VideoWriter(f"output/{output_name}.mp4", fourcc, fps, (width, height))
    video.set(cv2.VIDEOWRITER_PROP_QUALITY, 100)
    for image in images:
        nparr = np.frombuffer(image.getvalue(), np.uint8)
        img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
        video.write(img)
    cv2.destroyAllWindows()
    video.release()


# main
if __name__ == "__main__":
    create_mp4(map_file, sa4lowerbound_path_file, sa4lowerbound_name, fps=FPS)
    create_mp4(map_file, greedy4simulate_path_file, greedy4simulate_name, fps=FPS)

    sa_video = VideoFileClip(f"output/{sa4lowerbound_name}.mp4")
    greedy_viedo = VideoFileClip(f"output/{greedy4simulate_name}.mp4")

    output_video = clips_array([[greedy_viedo, sa_video]])
    output_video.write_videofile(f"output/compare.mp4", fps=FPS, bitrate=None)

    # remove temp files
    os.remove(f"output/{sa4lowerbound_name}.mp4")
    os.remove(f"output/{greedy4simulate_name}.mp4")
