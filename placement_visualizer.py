import sys
import matplotlib.pyplot as plt

def plot_points(file_path):
    # Separate lists for regular and 'p' prefixed coordinates
    x_cells, y_cells = [], []
    x_pads, y_pads = [], []

    valid = 0
    total = 0

    with open(file_path, 'r') as file:
        for line in file:
            id_str, x_str, y_str = line.strip().split()
            x = float(x_str)
            y = float(y_str)

            if id_str.startswith('p'):
                x_pads.append(x)
                y_pads.append(y)
            else:
                x_cells.append(x)
                y_cells.append(y)

            total += 1

            if 0 <= x <= 101 and 0 <= y <= 99:
                valid += 1

    print(f'Total cells (including I/O pads): {total}, valid: {valid}')

    # Plot cells
    plt.scatter(x_cells, y_cells, c='blue', label='Cells', s=35)
    # Plot I/O pads
    plt.scatter(x_pads, y_pads, c='red', label='I/O Pads', s=35)

    plt.xlabel('X')
    plt.ylabel('Y')
    plt.title(f'Placement: {file_path}')
    plt.legend()
    plt.show()

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: placement_visualizer [kiaPad file]")
    else:
        plot_points(sys.argv[1])
