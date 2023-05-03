# @packages
import matplotlib.pyplot as plt
import numpy as np
import os 
import tkinter as tk
import threading
from matplotlib.animation import FuncAnimation
from tkinter import *
from tkinter import filedialog
from tkinter import ttk
from PIL import Image

class MatrixAnimationsApp(tk.Frame):
    def __init__(self, master=None):
        super().__init__(master)
        self.master = master
        self.master.title("Matrix Animations")
        self.pack()
        self.create_widgets()
        self.data = None
        self.filename = None
        
    def create_widgets(self):
        # Custom style for the entry widget
        style1 = ttk.Style()
        style1.theme_use('default')
        style1.configure('RoundedEntry.TEntry', 
            foreground='black',
            borderwidth='5',
            relief='flat',
            padding=8,
            font=('Arial', 14),
            bordercolor='#1E88E5',
            focuscolor='#1E88E5',
            roundness=50)

        # Create label and button to upload .raw file
        self.file_label = tk.Label(self, text="Upload .raw File:", font=("Helvetica", 12))
        self.file_label.grid(row=0, column=0, padx=8, pady=5)
        self.file_button = tk.Button(self, text="Choose File", font=("Helvetica", 11), command=self.open_file)
        self.file_button.grid(row=0, column=1, padx=8, pady=5)
        
        # Create entry widgets for matrix dimensions
        self.rows_label = tk.Label(self, text="Matrix Rows:", font=("Helvetica", 12))
        self.rows_label.grid(row=1, column=0, padx=8, pady=5)
        self.rows_entry = ttk.Entry(self, style='RoundedEntry.TEntry')
        self.rows_entry.grid(row=1, column=1, padx=8, pady=5)
        self.cols_label = tk.Label(self, text="Matrix Columns:", font=("Helvetica", 12))
        self.cols_label.grid(row=2, column=0, padx=8, pady=5)
        self.cols_entry = ttk.Entry(self, style='RoundedEntry.TEntry')
        self.cols_entry.grid(row=2, column=1, padx=8, pady=5)

        # Create button to start animation & download .mp4
        self.start_button = tk.Button(self, text="Start Animation", command=self.start_animation_functions)
        self.start_button.grid(row=3, column=1, padx=8, pady=5)
        
        # Create label to display file name
        self.filename_label = tk.Label(self, text="")
        self.filename_label.grid(row=4, column=0, columnspan=2, padx=5, pady=5)
        
        # Create the progress bar
        self.progress_bar = ttk.Progressbar(self, orient=HORIZONTAL, mode='determinate', length=180)
        self.progress_bar.grid(row=5, column=0, columnspan=2, padx=5, pady=5)

    def open_file(self):
        # Open file dialog to select .raw file
        self.filename = filedialog.askopenfilename(initialdir="./", title="Select file", filetypes=[("Raw files", "*.raw")])
        # Load the raw data file
        self.data = np.fromfile(self.filename, dtype=np.float64)
        # Update the file name label
        self.filename = os.path.basename(self.filename)
        self.filename_label.config(text=self.filename)
        
    def start_progress_bar(self):
        self.progress_bar.start(8)
        
    def stop_progress_bar(self):
        self.progress_bar.stop()
        
    def start_animation_functions(self):
        # Create a new thread to run the start_animation() function
        thread = threading.Thread(target=self.start_animation)
        thread.start()
        # Start the progress bar in the main thread
        self.start_progress_bar()
        

    def start_animation(self):
      try:
        # Ensure file upload
        if (self.filename == None):
            self.stop_progress_bar()
            self.filename_label.config(text="Please upload .raw file", font=("Helvetica", 10))
            return
            
        # Retrieve rows and columns from entry widgets
        if (self.rows_entry.get() == '' or self.cols_entry.get()==''):
            self.stop_progress_bar()
            self.filename_label.config(text="Please enter rows and columns for matrix", font=("Helvetica", 10))
            return
        num_rows = int(self.rows_entry.get())
        num_cols = int(self.cols_entry.get())
        if (num_rows <= 0 or num_cols <= 0):
          self.stop_progress_bar()
          self.filename_label.config(text="Values must be greater than zero", font=("Helvetica", 10))
          return
        
        # Calculate # of timesteps based on the size of matrix
        num_timesteps = self.data.size // (num_rows * num_cols)

        # Reshape the data into a 3D array of shape (num_timesteps, num_rows, num_cols)
        self.data = self.data.reshape((num_timesteps, num_rows, num_cols))

        # Create a function to update the animation at each frame
        def update(frame):
            plt.clf()
            plt.imshow(self.data[frame], cmap='viridis')
            plt.title(f'Timestep {frame}')
            plt.axis('off')
            # Save the current frame as a PIL Image object
            fig.canvas.draw()
            img = Image.frombytes('RGB', fig.canvas.get_width_height(), fig.canvas.tostring_rgb())
            return img
        
        # Create the animation
        fig = plt.figure()
        ani = FuncAnimation(fig, update, frames=num_timesteps, interval=50)

        # Save the animation as an MP4 file
        ani.save(self.filename[:-4]+'-animation.mp4', writer='ffmpeg', fps=50)
        self.filename_label.config(text='Downloaded: '+self.filename[:-4]+'-animation.mp4', font=("Helvetica", 11))

        self.stop_progress_bar()
        # Display the animation
        plt.show()
        
      except Exception as e:
          self.filename_label.config(text="Error: Open console to review", font=("Helvetica", 10))
          print("Error: ", e)
        
        
if __name__ == "__main__":
    root = tk.Tk()
    app = MatrixAnimationsApp(master=root)
    app.mainloop()

