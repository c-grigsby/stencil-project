// @packages
import { Provider } from 'react-redux';
import { React, useState } from 'react';
// @scripts
import './styles/App.css';
import WelcomeMsg from './components/Welcome/WelcomeMsg';
import RawFileUpload from './components/FileUpload/RawFileUpload';
import store from './store/store';

function App() {
  const [appInPlay, setAppInPlay] = useState(false);

  return (
    <Provider store={store}>
      <div className="App">
        <div className="App-header">
          {!appInPlay && <WelcomeMsg />}
          <RawFileUpload setAppInPlay={setAppInPlay}/>
        </div>
      </div>
    </Provider>
  );
}

export default App;
