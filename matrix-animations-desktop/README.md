# Desktop Matrix Animations

Cross-platform desktop app for downloading & viewing .mp4 animations from matrix timestamps

## Project Details

- Developed with Python version 3.9 
- Tkinter GUI
- Animation results persisted via .mp4

## Getting Started

- To execute these scripts you will need to install the ffmpeg package. To install ffmpeg with homebrew on macOS, execute in the terminal:

  ```
  $ brew install ffmpeg
  ```

- To install ffmpeg with Ubuntu/Debian:
  
 
  ```
  $ sudo apt-get install ffmpeg
  ```

## To Run

- Ensure Python is installed locally on your machine (including TKinter)
- To initialize a virtual enviroment, navigate to the directory of the application in the terminal and execute:

  ##### _Note: "python3" will depend on your version of Python_

  ```
  $ python3 -m venv venv
  ```

- Activate the virtual environment:

  ```
  $ source venv/bin/activate
  ```

- Install dependencies:

  ```
  $ pip install -r requirements.txt
  ```

- To Execute: 

  ```
  $ python3 main.py
  ```
