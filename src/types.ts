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

// tslint:disable-next-line: no-empty-interface
export interface RSSensor {
  close(): void;
  destroy(): void;
  // getCameraInfo
  // getDepthScale
  // getOption
  // getOptionDescription
  // getOptionRange
  // getOptionValueDescription
  // getRegionOfInterest
  // getStreamProfiles
  // isDepthSensor
  // isOptionReadonly
  // isROISensor
  // openMultipleStream
  // openStream
  // setNotificationCallback
  // setOption
  // setRegionOfInterest
  // startWithCallback
  // startWithSyncer
  // stop
  // supportsCameraInfo
  // supportsOption

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
