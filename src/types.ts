type ErrorCallbackRegistration = <T extends object>(recv: T, fn: keyof T) => void;

export interface RealSenseAddon {
  cleanup(): void;
  getTime(): number;
  registerErrorCallback: ErrorCallbackRegistration;
}
