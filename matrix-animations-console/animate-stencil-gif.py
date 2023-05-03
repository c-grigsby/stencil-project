import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from PIL import Image


try: 
    # Get filename
    filename = input("\n>>> Hello, enter .raw file name for timestamp animation: ")

    # Load the raw data file
    data = np.fromfile(filename, dtype=np.float64)
    
    # Get size of each matrix in the stacked raw data file
    num_rows = int(input("\nThanks, enter the number of matrix rows: "))
    num_cols = int(input("Great, now enter the numbers of matrix columns: "))

    print("\n>>> Creating animation... ")

    # Calculate # of timesteps based on the size of the matrix
    num_timesteps = data.size // (num_rows * num_cols)

    # Reshape the data into a 3D array of shape (num_timesteps, num_rows, num_cols)
    data = data.reshape((num_timesteps, num_rows, num_cols))

    # Create a function to update the animation at each frame
    def update(frame):
        plt.clf()
        plt.imshow(data[frame], cmap='viridis')
        plt.title(f'Timestep {frame}')
        plt.axis('off')
        # Save the current frame as a PIL Image object
        fig.canvas.draw()
        img = Image.frombytes('RGB', fig.canvas.get_width_height(), fig.canvas.tostring_rgb())
        return img

    # Create the animation
    fig = plt.figure()
    ani = FuncAnimation(fig, update, frames=num_timesteps, interval=50)

    # Save the animation as a GIF file
    ani.save(filename[:-4]+'-animation.gif', writer='pillow', fps=50)

    # Display the animation
    plt.show()

    print("\n>>> Success! A gif of your animation has been generated")

except FileNotFoundError:
    print("Error: The specified file could not be found.")
except IOError:
    print("Error: An error occurred while reading the file.")
except Exception as e:
    print("Error: An unexpected error occurred: ", e)
