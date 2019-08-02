import { RealSenseAddon } from './types';

const addon: RealSenseAddon = require('bindings')('realsense_node');

console.log('addon!', addon);
console.log('time: ', addon.getTime());

export {
  addon,
};
