// @packages
import { Fragment, React } from 'react';
// @scripts
import logo from '../../public/logo.svg';
import classes from './WelcomeMsg.module.css';

const WelcomeMsg = () => {
  return (
    <Fragment>
      <img src={logo} className="App-logo" alt="logo" />
      <h1 className={classes.welcome_header}>Matrix Timestamp Animations</h1>
      <p className={classes.welcome_text}>
        This application will accept a .raw file representing a series of matrix timestamps  <br/>
        and transform them into a .mp4 file
      </p>
    </Fragment>
  );
};

export default WelcomeMsg;
