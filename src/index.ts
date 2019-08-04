import { RealSenseAddon } from './types';

const { getTime, registerErrorCallback }: RealSenseAddon = require('bindings')('realsense_node');

export {
  getTime,
  registerErrorCallback,
};
