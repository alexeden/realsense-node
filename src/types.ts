import {
  RSLogSeverity,
  RSNotificationCategory,
  RSOption,
  RSFormat,
  RSStreamType,
  RSFrameMetadata,
  RSConfidence,
} from './constants';

export interface RealSenseAddon {
  cleanup(): void;
  getTime(): number;
  registerErrorCallback: ErrorCallbackRegistration;
  RSAlign: new () => RSAlign;
  RSColorizer: new () => RSColorizer;
  RSConfig: new () => RSConfig;
  RSContext: new () => RSContext;
  RSDevice: new () => RSDevice;
  RSDeviceList: new () => RSDeviceList;
  RSFrame: new () => RSFrame;
  RSFrameSet: new () => RSFrameSet;
  RSPipeline: new () => RSPipeline;
  RSPipelineProfile: new () => RSPipelineProfile;
  RSSensor: new () => RSSensor;
  RSStreamProfile: new () => RSStreamProfile;
  RSSyncer: new () => RSSyncer;
}

type DevicesChangedCallback = (removed: RSDeviceList, added: RSDeviceList) => void;
type ErrorCallbackRegistration = <T extends object>(recv: T, fn: keyof T) => void;

export interface XYZ {
  x: number;
  y: number;
  z: number;
}

export interface XYZW {
  w: number;
  x: number;
  y: number;
  z: number;
}

export interface RSAlign {
  destroy(): this;
  process(frameset1: RSFrameSet, frameset2: RSFrameSet): boolean;
  waitForFrames(): RSFrameSet;
}

export interface RSColorizer {
  destroy(): this;
}

export interface RSConfig {
  destroy(): this;
  disableAllStreams(): this;
  disableStream(stream: number): this;
  enableAllStreams(): this;
  enableDevice(device: number): this;
  enableDeviceFromFile(file: string): this;
  enableDeviceFromFileRepeatOption(file: string, repeat: boolean): this;
  enableRecordToFile(file: string): this;
  enableStream(
    stream: number,
    index: number,
    width: number,
    height: number,
    format: number,
    framerate: number
  ): this;
}

export interface RSContext {
  createDeviceFromSensor(sensor: RSSensor): RSDevice;
  destroy(): this;
  loadDeviceFile(path: string): RSDevice;
  onDevicesChanged(devicesChangedCallback: DevicesChangedCallback): this;
  queryDevices(): RSDeviceList;
  unloadDeviceFile(path: string): void;
}

export interface RSDevice {
  destroy(): this;
  getCameraInfo(info: number): string;
  querySensors(): RSSensor[];
  reset(): this;
  supportsCameraInfo(info: number): boolean;
  triggerErrorForTest(): void;
}

export interface RSDeviceList {
  contains(device: RSDevice): boolean;
  destroy(): this;
  forEach(callback: (device: RSDevice, index: number) => void): void;
  getDevice(index: number): RSDevice;
  length(): number;
}

export interface RSExtrinsics {
  rotation: [number, number, number, number, number, number, number, number, number];
  translation: [number, number, number];
}

export interface RSFrame {
  canGetPoints(): boolean;
  destroy(): this;
  exportToPly(filename: string, frame: RSFrame): this;
  getBaseLine(): number;
  getBitsPerPixel(): number;
  getData(): Uint8Array;
  getDistance(x: number, y: number): number;
  getFrameMetadata(metadata: RSFrameMetadata, data: Uint8Array): boolean;
  getFrameNumber(): number;
  getHeight(): number;
  getMotionData(xyz: XYZ): this;
  getPointsCount(): number;
  getPoseData(data: RSPose): boolean;
  getStreamProfile(): RSStreamProfile;
  getStrideInBytes(): number;
  getTexCoordBufferLen(): number;
  getTextureCoordinates(): Float32Array;
  getTimestamp(): number;
  getTimestampDomain(): number;
  getVertices(): Float32Array;
  getVerticesBufferLen(): number;
  getWidth(): number;
  isDepthFrame(): boolean;
  isDisparityFrame(): boolean;
  isMotionFrame(): boolean;
  isPoseFrame(): boolean;
  isValid(): boolean;
  isVideoFrame(): boolean;
  keep(): this;
  supportsFrameMetadata(metadata: RSFrameMetadata): boolean;
  writeData(data: ArrayBuffer): this;
  writeTextureCoordinates(coords: ArrayBuffer): boolean;
  writeVertices(vertices: ArrayBuffer): boolean;
}

export interface RSFrameSet {
  destroy(): this;
  getFrame(stream: RSStreamType, streamIndex: number): RSFrame;
  getSize(): number;
  indexToStream(index: number): RSStreamType;
  indexToStreamIndex(index: number): number;
  replaceFrame(stream: RSStreamType, streamIndex: number, frame: RSFrame): boolean;
}

export interface RSIntrinsics {
  coeffs: [number, number, number, number, number];
  fx: number;
  fy: number;
  height: number;
  model: number;
  ppx: number;
  ppy: number;
  width: number;
}

export interface RSMotionIntrinsics {
  biasVariances: [number, number, number];
  data: [number, number, number];
  noiseVariances: [number, number, number];
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
  create(context?: RSContext): this;
  destroy(): this;
  getActiveProfile(): RSPipelineProfile;
  pollForFrames(frameset: RSFrameSet): boolean;
  start(config?: RSConfig): RSPipelineProfile;
  stop(): this;
  waitForFrames(frameset: RSFrameSet, timeout?: number): boolean;
}

export interface RSPipelineProfile {
  destroy(): this;
  getDevice(): RSDevice;
  getStreams(): RSStreamProfile[];
}

export interface RSPose {
  acceleration: XYZ;
  angularAcceleration: XYZ;
  angularVelocity: XYZ;
  mapperConfidence: RSConfidence;
  rotation: XYZW;
  trackerConfidence: RSConfidence;
  translation: XYZ;
  velocity: XYZ;
}

export interface RSRegionOfInterest {
  maxX: number;
  maxY: number;
  minX: number;
  minY: number;
}

export interface RSSensor {
  readonly isDepthSensor: boolean;
  readonly isROISensor: boolean;
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
  isOptionReadonly(option: RSOption): boolean;
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


export interface RSStreamProfile {
  destroy(): this;
  format: RSFormat;
  fps: number;
  getExtrinsicsTo(profile: RSStreamProfile): RSExtrinsics;
  getMotionIntrinsics(): RSMotionIntrinsics;
  getVideoStreamIntrinsics(): RSIntrinsics;
  height: number;
  index: number;
  isDefault: number;
  isMotionProfile: boolean;
  isVideoProfile: boolean;
  streamType: RSStreamType;
  uniqueId: number;
  width: number;
}

export interface RSSyncer {
  destroy(): this;
  pollForFrames(frameset: RSFrameSet): boolean;
  waitForFrames(frameset: RSFrameSet): boolean;
}
