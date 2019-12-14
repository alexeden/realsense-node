type ErrorCallbackRegistration = <T extends object>(recv: T, fn: keyof T) => void;

export interface RealSenseAddon {
  cleanup(): void;
  getTime(): number;
  registerErrorCallback: ErrorCallbackRegistration;
}

// tslint:disable-next-line: no-empty-interface
export interface RSDevice {

}

export interface RSDeviceList {
  destroy(): void;
  contains(device: RSDevice): boolean;
  length(): number;
  getDevice(index: number): RSDevice;
}
