# Stencil Animation Server

This application provides an API for the stencil animation app

## Project Details

- Developed with Python version 3.10 and Flask micro-framework
- Matplotlib for processing .raw matrix & returning .mp4 animations
- Deployed to Heroku

## Getting Started

- Ensure Python is installed locally on your machine
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

## Run the Development Server

- Navigate to the project in the terminal and activate the virtual environment:

  ```
  $ source venv/bin/activate
  ```

- Run the development server from the 'src' directory:

  ```
  $ flask run
  ```

---
