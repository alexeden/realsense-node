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

}
// tslint:disable-next-line: no-empty-interface
export interface RSDevice {

}

export interface RSDeviceList {
  contains(device: RSDevice): boolean;
  destroy(): void;
  getDevice(index: number): RSDevice;
  length(): number;
}
