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

export const RSAlign = () => {
  const rsAlign = new addon.RSAlign();
  destroyables.push(rsAlign);

  return rsAlign;
};

export const RSColorizer = () => {
  const rsColorizer = new addon.RSColorizer();
  destroyables.push(rsColorizer);

  return rsColorizer;
};

export const RSConfig = () => {
  const rsConfig = new addon.RSConfig();
  destroyables.push(rsConfig);

  return rsConfig;
};

export const RSContext = () => {
  const rsContext = new addon.RSContext();
  destroyables.push(rsContext);

  return rsContext;
};

export const RSDevice = () => {
  const rsDevice = new addon.RSDevice();
  destroyables.push(rsDevice);

  return rsDevice;
};

export const RSDeviceList = () => {
  const rsDeviceList = new addon.RSDeviceList();
  destroyables.push(rsDeviceList);

  return rsDeviceList;
};

export const RSFrame = () => {
  const rsFrame = new addon.RSFrame();
  destroyables.push(rsFrame);

  return rsFrame;
};

export const RSFrameSet = () => {
  const rsFrameSet = new addon.RSFrameSet();
  destroyables.push(rsFrameSet);

  return rsFrameSet;
};

export const RSPipeline = () => {
  const rsPipeline = new addon.RSPipeline();
  destroyables.push(rsPipeline);

  return rsPipeline;
};

export const RSPipelineProfile = () => {
  const rsPipelineProfile = new addon.RSPipelineProfile();
  destroyables.push(rsPipelineProfile);

  return rsPipelineProfile;
};

export const RSSensor = () => {
  const rsSensor = new addon.RSSensor();
  destroyables.push(rsSensor);

  return rsSensor;
};

export const RSStreamProfile = () => {
  const rsStreamProfile = new addon.RSStreamProfile();
  destroyables.push(rsStreamProfile);

  return rsStreamProfile;
};

export const RSSyncer = () => {
  const rsSyncer = new addon.RSSyncer();
  destroyables.push(rsSyncer);

  return rsSyncer;
};
