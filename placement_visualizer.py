import matplotlib.pyplot as plt
import sys

def plot_points(file_path):
    # Lists to store x and y coordinates
    x_coords = []
    y_coords = []

    valid = 0
    total = 0

    # Read points from the file
    with open(file_path, 'r') as file:
        for line in file:
            # Split the line into ID, x, and y
            _, x, y = map(str, line.strip().split())
            x_coords.append(float(x))
            y_coords.append(float(y))

            total += 1

            if float(x) >= 0 and float(x) <= 101 and float(y) >= 0 and float(y) <= 99:
                valid += 1

    print(f'Total cells (including I/O pads): {total}, valid: {valid}')

    # Plot the points
    plt.scatter(x_coords, y_coords, label='Cells/IO Pads', s=1)

    # Add labels and title
    plt.xlabel('X')
    plt.ylabel('Y')
    plt.title(f'Placement: {file_path}')

    # Add legend
    plt.legend()

    # Show the plot
    plt.show()

if __name__ == "__main__":
    if (len(sys.argv) != 2):
        print("Usage: placement_visualizer [kiaPad file]")
    else:
        plot_points(sys.argv[1])
