// @packages
import axios from 'axios';
import { BeatLoader } from 'react-spinners';
import { Fragment, React, useEffect, useRef, useState } from 'react';
import { animateScroll as scroll } from 'react-scroll';
// @scripts
import classes from './RawFileUpload.module.css';
import PlayVideo from '../VideoPlayer/PlayVideo';

const RawFileUpload = ({ setAppInPlay }) => {
  const [error, setError] = useState(false);
  const [fileUploaded, setFileUploaded] = useState(null);
  const [fileName, setFileName] = useState();
  const [mp4Url, setMp4Url] = useState(null);
  const [loading, setLoading] = useState(false);
  const [numRows, setNumRows] = useState(0);
  const [numCols, setNumCols] = useState(0);
  const fileInputRef = useRef();

  useEffect(() => {
    if (fileUploaded) {
      // Reset state
      setMp4Url(null);
      setNumRows(0);
      setNumCols(0);
      setAppInPlay(true);

      const reader = new FileReader();
      reader.readAsDataURL(fileUploaded);
    }
  },[setAppInPlay, fileUploaded]);

  const fileSelectedHandler = (e) => {
    let file = e.target.files[0];
    if (file) {
      // If file size is greater than 25MB
      if (file.size > 1024 * 1024 * 25) {
        alert("File size must be less than 25MB.\n");
        return;
      } else {
        setFileUploaded(file);
        setFileName(file.name);
      }
      scroll.scrollToBottom({
        smooth: true,
      });
    }
  };

  const uploadRequest = (e) => {
    e.preventDefault();
    fileInputRef.current.click();
  };

  const downloadMP4 = async () => {
    const link = document.createElement('a');
    link.href = mp4Url;
    link.download = `${fileName.slice(0, -4)}-animation.mp4`;
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
  };

  const getAnimation_MP4 = async (fileUploaded) => {
    try {
      scroll.scrollToBottom({
        smooth: true,
      });
      setLoading(true);
      if (fileUploaded != null) {
        setLoading(true);
        // Send .raw file to backend for conversion
        const formData = new FormData();
        formData.append('stacked_raw_file', fileUploaded);
        formData.append('num_rows', numRows);
        formData.append('num_columns', numCols);

        const response = await axios.post(
          `${process.env.REACT_APP_BACKEND_API}/animate-stencil`,
          formData,
          {
            headers: {
              'Content-Type': 'multipart/form-data',
            },
            responseType: 'blob',
          }
        );
        // Convert the response data to a Blob object
        const mp4Blob = new Blob([response.data], { type: 'video/mp4' });
        // Create a URL for the Blob object
        const createMp4Url = URL.createObjectURL(mp4Blob);
        // Set the URL of the MP4 file in state
        setMp4Url(createMp4Url);
      }
    } catch (error) {
      alert("Error with request to the server. Please ensure that you have entered the corect information for rows & columns and timestamp iterations is below 300.", error);
      console.error(error);
    }
    setLoading(false);
  };

  const downloadMp4Handler = async (e) => {
    e.preventDefault();
    if (numCols <= 0 || numRows <= 0) {
      setError(true);
      return;
    }
    setError(false);
    if (mp4Url != null) { 
      console.log("here")
      await downloadMP4();
    } else {
      // Call server to convert .raw to .mp4 animation
      await getAnimation_MP4(fileUploaded);
      await downloadMP4();
    }
    scroll.scrollToBottom({
      smooth: true,
    });
  };

  const playAnimationHandler = async (e) => {
    e.preventDefault();
    if (numCols <= 0 || numRows <= 0) {
      setError(true);
      return;
    }
    setError(false);
    if (mp4Url != null) return;
    // Call server to convert .raw to .mp4 animation
    await getAnimation_MP4(fileUploaded);
    scroll.scrollToBottom({
      smooth: true,
    });
  };

  return (
    <Fragment>
      <form className={classes.form}>
        {fileName && (
          <div className={classes.filename}> {fileName} uploaded</div>
        )}
        <button onClick={uploadRequest}>Select .raw File</button>
        <input
          accept='.raw'
          type='file'
          style={{ display: 'none' }}
          ref={fileInputRef}
          onChange={fileSelectedHandler}
        />
        {fileName && (
          <div className={classes.input_labels_container}>
            <label className={classes.input_labels}>
              Number of Matrix Rows:
              <input
                type='number'
                value={numRows}
                onChange={(e) => setNumRows(e.target.value)}
                className={error ? classes.input_box_error : classes.input_box}
              />
            </label>
            <br />
            <label className={classes.input_labels}>
              Number of Matrix Columns:
              <input
                type='number'
                value={numCols}
                onChange={(e) => setNumCols(e.target.value)}
                className={error ? classes.input_box_error : classes.input_box}
              />
            </label>
          </div>
        )}
      </form>
      <form>
        {fileUploaded && (
          <button onClick={downloadMp4Handler}>Download .mp4</button>
        )}
        {fileUploaded && (
          <button onClick={playAnimationHandler}>Play Animation</button>
        )}
      </form>
      <div className={classes.spinnerDiv}>
        {loading && (
          <div className={classes.loading_text}>
            This could take a moment...
          </div>
        )}
        <BeatLoader
          className={classes.spinner}
          color='white'
          loading={loading}
          size={23}
        />
      </div>
      {mp4Url && <PlayVideo videoURL={mp4Url} />}
    </Fragment>
  );
};

export default RawFileUpload;
