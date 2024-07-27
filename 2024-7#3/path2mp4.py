import os
import cv2
import asciiMap2Png
import numpy as np

map_file = "map.txt"
sa4lowerbound_path_file = "sa4lowerbound_path.txt"


# main
if __name__ == "__main__":
    # read map
    map_file_lines = []
    with open(map_file, "r") as file:
        map_file_lines = file.readlines()

    # read sa4lowerbound_path
    sa4lowerbound_path = []
    with open(sa4lowerbound_path_file, "r") as f:
        sa4lowerbound_path_lines = f.read().splitlines()
        for line in sa4lowerbound_path_lines:
            numbers = [int(x) for x in line.split()]
            positions = [
                (numbers[i], numbers[i + 1]) for i in range(0, len(numbers), 2)
            ]
            sa4lowerbound_path.append(positions)

    max_len = max(len(row) for row in sa4lowerbound_path)
    images = []
    for index in range(0, max_len):
        now_map = [list(row) for row in map_file_lines.copy()]
        for row in sa4lowerbound_path:
            agv = row[index] if index < len(row) else (0, 0)
            now_map[agv[0]][agv[1]] = "A"
        now_map = ["".join(row) for row in now_map]
        images.append(asciiMap2Png.ascii_to_png(now_map))

    # output to image
    # os.makedirs("output", exist_ok=True)

    nparr = np.frombuffer(images[0].getvalue(), np.uint8)
    frame = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
    height, width, layers = frame.shape

    fourcc = cv2.VideoWriter_fourcc(*"mp4v")
    video = cv2.VideoWriter("output.mp4", fourcc, 4, (width, height))

    for image in images:
        nparr = np.frombuffer(image.getvalue(), np.uint8)
        img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
        video.write(img)
    cv2.destroyAllWindows()
    video.release()
