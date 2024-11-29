from PIL import Image, ImageDraw, ImageFont


# Read the map and task files
def read_file(filename):
    with open(filename, "r") as file:
        return [line.strip() for line in file.readlines()]


# Mark tasks on the map
def mark_tasks_on_map(map_data, player_count, tasks):
    # Convert the map to a modifiable list
    map_grid = [list(row) for row in map_data]

    # Iterate through each player's tasks
    for player_id, task_coords in enumerate(tasks, start=1):
        for task_idx, coord in enumerate(task_coords, start=1):
            x, y = coord
            # Mark the task location
            if 0 <= x < len(map_grid) and 0 <= y < len(
                map_grid[0]
            ):  # Prevent out-of-bounds access
                map_grid[x][
                    y
                ] = f"{player_id}-{task_idx}"  # Mark as player-number-task-order

    # Return the modified map grid for drawing
    return map_grid


# Parse the task file
def parse_tasks(task_data):
    player_count = int(task_data[0])  # First line is the number of players
    tasks = []  # List of task coordinates for each player
    for i in range(1, len(task_data)):
        task_line = list(map(int, task_data[i].split()))
        task_count = task_line[0]  # Number of tasks
        task_coords = [
            (task_line[j], task_line[j + 1]) for j in range(1, len(task_line), 2)
        ]  # Extract coordinate pairs
        tasks.append(task_coords)
    return player_count, tasks


# Save the map as a PNG file
def save_map_as_png(map_data, filename, cell_size=40, player_colors=None):
    rows = len(map_data)
    cols = len(map_data[0])
    width = cols * cell_size
    height = rows * cell_size

    # Create the image
    img = Image.new("RGB", (width, height), "white")
    draw = ImageDraw.Draw(img)

    # Load the font
    try:
        # Try to load a system font, font size is dynamically set based on cell size
        font_size = cell_size // 2
        font = ImageFont.truetype("arial.ttf", font_size)
    except IOError:
        # If loading a font fails, use the default font
        font = ImageFont.load_default()

    # Set player colors
    if player_colors is None:
        player_colors = {
            "1": "red",
            "2": "green",
            "3": "blue",
            "4": "orange",
            "5": "purple",
            "6": "pink",
            "7": "cyan",
            "8": "yellow",
        }

    # Iterate through the map data and draw
    for i, row in enumerate(map_data):
        for j, cell in enumerate(row):
            x0, y0 = j * cell_size, i * cell_size
            x1, y1 = x0 + cell_size, y0 + cell_size

            # Draw the map background
            if cell == "#":
                draw.rectangle([x0, y0, x1, y1], fill="black")  # Wall
            elif cell == "o":
                draw.rectangle([x0, y0, x1, y1], fill="lightgray")  # Walkable area
            elif cell == "$":
                draw.rectangle([x0, y0, x1, y1], fill="gold")  # Special area
            elif cell == "P":
                draw.rectangle(
                    [x0, y0, x1, y1], fill="blue"
                )  # Player starting position
            elif "-" in cell:  # Task point marker
                player_id, task_idx = cell.split("-")
                player_color = player_colors.get(player_id, "red")  # Default to red
                draw.rectangle([x0, y0, x1, y1], fill=player_color)  # Task point color
                # Display task number (player-number-task-order)
                text = f"{player_id}-{task_idx}"
                text_bbox = draw.textbbox(
                    (0, 0), text, font=font
                )  # Get text bounding box
                text_width, text_height = (
                    text_bbox[2] - text_bbox[0],
                    text_bbox[3] - text_bbox[1],
                )
                text_x = x0 + (cell_size - text_width) // 2
                text_y = y0 + (cell_size - text_height) // 2
                draw.text((text_x, text_y), text, fill="white", font=font)

    # Save the image
    img.save(filename)


# Main function
def main():
    # File names
    map_filename = "map.txt"
    tasks_filename = "tasks.txt"
    original_map_output = "original_map.png"
    task_map_output = "task_map.png"

    # Read the map and task data
    map_data = read_file(map_filename)
    task_data = read_file(tasks_filename)

    # Parse the task data
    player_count, tasks = parse_tasks(task_data)

    # Save the original map as a PNG
    save_map_as_png(map_data, original_map_output)
    print(f"Original map saved as {original_map_output}")

    # Mark tasks on the map
    updated_map = mark_tasks_on_map(map_data, player_count, tasks)

    # Save the map with tasks as a PNG
    save_map_as_png(updated_map, task_map_output)
    print(f"Map with tasks saved as {task_map_output}")


# Run the main function
if __name__ == "__main__":
    main()
