import { RealSenseAddon } from './types';

export const addon: RealSenseAddon = require('bindings')('realsense_node');

addon.registerErrorCallback(
  {
    callback(error: Error) {
      console.error(`Native module error!`, error);
    },
  },
  'callback'
);

interface Destroyable { destroy(): unknown; }
const destroyables: Destroyable[] = [];

export const deleteAutomatically = <T extends Destroyable>(obj: T) => destroyables.push(obj);

export const cleanup = () => {
  destroyables.forEach(d => {
    console.log(`Destroying `, d);
    d.destroy();
  });

  addon.cleanup();
};
