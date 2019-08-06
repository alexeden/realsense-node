import { RealSenseAddon } from './types';

const addon: RealSenseAddon = require('bindings')('realsense_node');

addon.registerErrorCallback(
  {
    callback(error: Error) {
      console.error(`Native module error!`, error);
    },
  },
  'callback'
);

export {
  addon,
};
