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

interface Destroyable { destroy(): this; }
const destroyables: Destroyable[] = [];

export const getTime = addon.getTime;

export const cleanup = () => {
  destroyables.forEach(d => {
    console.log(`Destroying `, d);
    d.destroy();
  });

  addon.cleanup();
};

export const createAlign = () => {
  const rsAlign = new addon.RSAlign();
  destroyables.push(rsAlign);

  return rsAlign;
};

export const createColorizer = () => {
  const rsColorizer = new addon.RSColorizer();
  destroyables.push(rsColorizer);

  return rsColorizer;
};

export const createConfig = () => {
  const rsConfig = new addon.RSConfig();
  destroyables.push(rsConfig);

  return rsConfig;
};

export const createContext = () => {
  const rsContext = new addon.RSContext();
  destroyables.push(rsContext);

  return rsContext;
};

export const createDevice = () => {
  const rsDevice = new addon.RSDevice();
  destroyables.push(rsDevice);

  return rsDevice;
};

export const createDeviceList = () => {
  const rsDeviceList = new addon.RSDeviceList();
  destroyables.push(rsDeviceList);

  return rsDeviceList;
};

export const createFrame = () => {
  const rsFrame = new addon.RSFrame();
  destroyables.push(rsFrame);

  return rsFrame;
};

export const createFrameSet = () => {
  const rsFrameSet = new addon.RSFrameSet();
  destroyables.push(rsFrameSet);

  return rsFrameSet;
};

export const createPipeline = () => {
  const rsPipeline = new addon.RSPipeline();
  destroyables.push(rsPipeline);

  return rsPipeline;
};

export const createPipelineProfile = () => {
  const rsPipelineProfile = new addon.RSPipelineProfile();
  destroyables.push(rsPipelineProfile);

  return rsPipelineProfile;
};

export const createSensor = () => {
  const rsSensor = new addon.RSSensor();
  destroyables.push(rsSensor);

  return rsSensor;
};

export const createStreamProfile = () => {
  const rsStreamProfile = new addon.RSStreamProfile();
  destroyables.push(rsStreamProfile);

  return rsStreamProfile;
};

export const createSyncer = () => {
  const rsSyncer = new addon.RSSyncer();
  destroyables.push(rsSyncer);

  return rsSyncer;
};
