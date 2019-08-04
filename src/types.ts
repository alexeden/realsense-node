type ErrorCallbackRegistration = <T extends object>(recv: T, fn: keyof T) => void;

export interface RealSenseAddon {
  getTime(): number;

  registerErrorCallback: ErrorCallbackRegistration;
}
