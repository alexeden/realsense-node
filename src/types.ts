import {
  RSLogSeverity,
  RSNotificationCategory,
  RSOption,
} from './constants';

type ErrorCallbackRegistration = <T extends object>(recv: T, fn: keyof T) => void;

export interface RealSenseAddon {
  cleanup(): void;
  getTime(): number;
  RSContext: new () => RSContext;
  registerErrorCallback: ErrorCallbackRegistration;
}

type DevicesChangedCallback = (removed: RSDeviceList, added: RSDeviceList) => void;

// tslint:disable-next-line: no-empty-interface
export interface RSConfig {

}
export interface RSContext {
  createDeviceFromSensor(sensor: RSSensor): RSDevice;
  destroy(): void;
  loadDeviceFile(path: string): RSDevice;
  onDevicesChanged(devicesChangedCallback: DevicesChangedCallback): this;
  queryDevices(): RSDeviceList;
  unloadDeviceFile(path: string): void;
}

export interface RSDevice {
  destroy(): void;
  getCameraInfo(info: number): string;
  querySensors(): RSSensor[];
  reset(): this;
  supportsCameraInfo(info: number): boolean;
  triggerErrorForTest(): void;
}

export interface RSDeviceList {
  contains(device: RSDevice): boolean;
  destroy(): void;
  forEach(callback: (device: RSDevice, index: number) => void): void;
  getDevice(index: number): RSDevice;
  length(): number;
}

// tslint:disable-next-line: no-empty-interface
export interface RSFrame {

}

export interface RSFrameSet {
  destroy(): this;
  getFrame(stream: number, streamIndex: number): RSFrame;
  getSize(): number;
  indexToStream(index: number): number;
  indexToStreamIndex(index: number): number;
  replaceFrame(stream: number, streamIndex: number, frame: RSFrame): boolean;
}

export interface RSNotification {
  category: RSNotificationCategory;
  description: string;
  serializedData: string;
  severity: RSLogSeverity;
  timestamp: number;
}

export interface RSOptionRange {
  defaultValue: number;
  maxValue: number;
  minValue: number;
  step: number;
}

export interface RSPipeline {
  create(context: RSContext): this;
  destroy(): this;
  getActiveProfile(): RSPipelineProfile;
  pollForFrames(frameset: RSFrameSet): boolean;
  start(): RSPipelineProfile;
  startWithConfig(config: RSConfig): RSPipelineProfile;
  stop(): this;
  waitForFrames(frameset: RSFrameSet): boolean;
}

// tslint:disable-next-line: no-empty-interface
export interface RSPipelineProfile {

}
export interface RSRegionOfInterest {
  maxX: number;
  maxY: number;
  minX: number;
  minY: number;
}

export interface RSSensor {
  close(): this;
  destroy(): this;
  getCameraInfo(index: number): string;
  getDepthScale(): number;
  getOption(option: RSOption): number;
  getOptionDescription(option: RSOption): string;
  getOptionRange(option: RSOption): RSOptionRange;
  getOptionValueDescription(option: RSOption, value: number): string;
  getRegionOfInterest(): RSRegionOfInterest;
  getStreamProfiles(): RSStreamProfile;
  isDepthSensor(): boolean;
  isOptionReadonly(option: RSOption): boolean;
  isROISensor(): boolean;
  onNotification(callback: (notification: RSNotification) => void): this;
  openMultipleStream(streams: RSStreamProfile[]): this;
  openStream(stream: RSStreamProfile): this;
  setOption(option: RSOption, value: number): this;
  setRegionOfInterest(minx: number, miny: number, maxx: number, maxy: number): this;
  // startWithCallback
  startWithSyncer(syncer: RSSyncer): this;
  stop(): void;
  supportsCameraInfo(camera: number): boolean;
  supportsOption(option: RSOption): boolean;
}


// tslint:disable-next-line: no-empty-interface
export interface RSStreamProfile {
}

export interface RSSyncer {
  destroy(): this;
  pollForFrames(frameset: RSFrameSet): boolean;
  waitForFrames(frameset: RSFrameSet): boolean;
}
