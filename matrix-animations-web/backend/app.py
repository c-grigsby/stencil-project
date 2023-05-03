# @packages
from PIL import Image
import matplotlib
matplotlib.use('Agg')
import os
import tempfile
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from flask import Flask, request, send_file
from flask_cors import CORS
from moviepy.editor import *

app = Flask(__name__)
CORS(app)

# Add any other allowed file extensions here
app.config['UPLOAD_EXTENSIONS'] = ['.raw']  

@app.route("/", methods=["GET"])
def home():
    return "Hello from Stencil Player API"


@app.route('/animate-stencil', methods=['POST'])
def create_animation():
    # Check if the POST request contains the 'all-stacked.raw' file
    if 'stacked_raw_file' not in request.files:
        return 'Error: No stacked_raw_file in request', 400
    
    # Get the raw data file from the POST request
    raw_file = request.files['stacked_raw_file']

    # Check if the received file is a raw file
    filename = raw_file.filename
    if not filename.endswith(tuple(app.config['UPLOAD_EXTENSIONS'])):
      return 'Error: Invalid file extension', 400

    # Set the size of each matrix in the stacked raw data file
    num_rows = int(request.form['num_rows'])
    num_cols = int(request.form['num_columns'])
    
    if not num_rows:
        return 'Error: No rum_rows in request', 400
    if not num_cols:
        return 'Error: No rum_columns in request', 400

    # Save the raw data file to a temporary directory
    with tempfile.TemporaryDirectory() as tmp_dir:
        raw_file_path = os.path.join(tmp_dir, 'all-stacked.raw')
        raw_file.save(raw_file_path)

        # Load the raw data file
        data = np.fromfile(raw_file_path, dtype=np.float64)

        # Calculate the number of timesteps based on the size of the data array
        num_timesteps = data.size // (num_rows * num_cols)

        # Reshape the data into a 3D array of shape (num_timesteps, num_rows, num_cols)
        data = data.reshape((num_timesteps, num_rows, num_cols))

        # Create a function to update the animation at each frame
        def update(frame):
            plt.clf()
            plt.imshow(data[frame], cmap='viridis')
            plt.title(f'Timestep {frame}')
            plt.axis('off')

        # Create the animation
        fig = plt.figure()
        ani = FuncAnimation(fig, update, frames=num_timesteps, interval=50)

        # Save the animation as a GIF file
        gif_file_path = os.path.join(tmp_dir, 'animation.gif')
        ani.save(gif_file_path, writer='pillow', fps=30) 
        clip = VideoFileClip(gif_file_path)

         # Create the output MP4 file
        mp4_file_path = os.path.join(tmp_dir, 'animation.mp4')
        clip.write_videofile(mp4_file_path, fps=30)
    
        # Return the GIF file as a download
        return send_file(mp4_file_path, mimetype='video/mp4', as_attachment=True), 200, {'Access-Control-Allow-Origin': '*' }


if __name__ == "__main__":
    app.run(port=8000, debug=True)

