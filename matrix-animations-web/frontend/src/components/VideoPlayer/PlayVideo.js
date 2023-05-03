// @packages
import React from 'react';
import ReactPlayer from 'react-player';
// @scripts
import classes from './PlayVideo.module.css';

const PlayVideo = ({ videoURL }) => {
  return (
    <div>
      <ReactPlayer
        className={classes.video_player}
        url={videoURL}
        playing={true}
        controls={true}
        width={640}
      />
    </div>
  );
};

export default PlayVideo;
