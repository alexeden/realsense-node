type ErrorCallbackRegistration = <T extends object>(recv: T, fn: keyof T) => void;

export interface RealSenseAddon {
  cleanup(): void;
  getTime(): number;
  RSContext: new () => RSContext;
  registerErrorCallback: ErrorCallbackRegistration;
}

type DevicesChangedCallback = (removed: RSDeviceList, added: RSDeviceList) => void;

export interface RSContext {
  createDeviceFromSensor(sensor: RSSensor): RSDevice;
  destroy(): void;
  loadDeviceFile(path: string): RSDevice;
  onDevicesChanged(devicesChangedCallback: DevicesChangedCallback): this;
  queryDevices(): RSDeviceList;
  unloadDeviceFile(path: string): void;
}

export interface RSOptionRange {
  defaultValue: number;
  maxValue: number;
  minValue: number;
  step: number;
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
  getOption(option: number): number;
  getOptionDescription(option: number): string;
  getOptionRange(option: number): RSOptionRange;
  getOptionValueDescription(option: number, value: number): string;
  getRegionOfInterest(): RSRegionOfInterest;
  getStreamProfiles(): RSStreamProfile;
  isDepthSensor(): boolean;
  isOptionReadonly(option: number): boolean;
  isROISensor(): boolean;
  openMultipleStream(streams: RSStreamProfile[]): this;
  openStream(stream: RSStreamProfile): this;
  // setNotificationCallback
  setOption(option: number, value: number): this;
  setRegionOfInterest(minx: number, miny: number, maxx: number, maxy: number): this;
  // startWithCallback
  startWithSyncer(syncer: RSSyncer): this;
  stop(): void;
  supportsCameraInfo(camera: number): boolean;
  supportsOption(option: number): boolean;
}


// tslint:disable-next-line: no-empty-interface
export interface RSStreamProfile {
}

// tslint:disable-next-line: no-empty-interface
export interface RSSyncer {

}
// tslint:disable-next-line: no-empty-interface
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
