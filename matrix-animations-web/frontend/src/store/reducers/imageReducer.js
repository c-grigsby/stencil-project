// @scripts
import { SELECTING_IMAGE } from '../actions/imageTypes';

const initialState = {
  selectingImage: false,
};

const imageReducer = (state = initialState, action) => {
  switch (action.type) {
    case SELECTING_IMAGE:
      return {
        selectingImage: !state.selectingImage,
      };
    default:
      return state;
  }
};

export default imageReducer;
