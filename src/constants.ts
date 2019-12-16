export enum RSCameraInfo {
  /** Friendly name */
  Name,
  /** Device serial number */
  SerialNumber,
  /** Primary firmware version */
  FirmwareVersion,
  /** Recommended firmware version */
  RecommendedFirmwareVersion,
  /** Unique identifier of the port the device is connected to (platform specific) */
  PhysicalPort,
  /** If device supports firmware logging, this is the command to send to get logs from firmware */
  DebugOpCode,
  /** True iff the device is in advanced mode */
  AdvancedMode,
  /** Product ID as reported in the USB descriptor */
  ProductId,
  /** True iff EEPROM is locked */
  CameraLocked,
  /** Designated USB specification: USB2/USB3 */
  UsbTypeDescriptor,
  /** Device product line D400/SR300/L500/T200 */
  ProductLine,
  /** ASIC serial number */
  AsicSerialNumber,
  /** Firmware update ID */
  FirmwareUpdateId,
}

export enum RSConfidence {
  Failed,
  Low,
  Medium,
  High,
}

export enum RSFormat {
  /** When passed to enable stream, librealsense will try to provide best suited format */
  Any,
  /** 16-bit linear depth values. The depth is meters is equal to depth scale * pixel value. */
  Z16,
  /** 16-bit float-point disparity values. Depth->Disparity conversion : Disparity = Baseline*FocalLength/Depth. */
  Disparity16,
  /** 32-bit floating point 3D coordinates. */
  XYZ32F,
  /**
   * 32-bit y0, u, y1, v data for every two pixels. Similar to YUV422 but packed in a different
   * order - https://en.wikipedia.org/wiki/YUV
   */
  YUYV,
  /** 8-bit red, green and blue channels */
  RGB8,
  /** 8-bit blue, green, and red channels -- suitable for OpenCV */
  BGR8,
  /** 8-bit red, green and blue channels + constant alpha channel equal to FF */
  RGBA8,
  /** 8-bit blue, green, and red channels + constant alpha channel equal to FF */
  BGRA8,
  /** 8-bit per-pixel grayscale image */
  Y8,
  /** 16-bit per-pixel grayscale image */
  Y16,
  /** Four 10 bits per pixel luminance values packed into a 5-byte macropixel */
  Raw10,
  /** 16-bit raw image */
  Raw16,
  /** 8-bit raw image */
  Raw8,
  /** Similar to the standard YUYV pixel format, but packed in a different order */
  UYVY,
  /** Raw data from the motion sensor */
  MotionRaw,
  /** Motion data packed as 3 32-bit float values, for X, Y, and Z axis */
  MotionXYZ32F,
  /** Raw data from the external sensors hooked to one of the GPIO's */
  GpioRaw,
  /**
   * Pose data packed as floats array, containing translation vector, rotation quaternion and
   * prediction velocities and accelerations vectors
   */
  SixDof,
  /** 32-bit float-point disparity values. Depth->Disparity conversion : Disparity = Baseline*FocalLength/Depth */
  Disparity32,
  /**
   * 16-bit per-pixel grayscale image unpacked from 10 bits per pixel packed ([8:8:8:8:2222])
   * grey-scale image. The data is unpacked to LSB and padded with 6 zero bits
   */
  Y10BPACK,
  /** 32-bit float-point depth distance value. */
  Distance,
  /** Bitstream encoding for video in which an image of each frame is encoded as JPEG-DIB  */
  Mjpeg,
  /** 8-bit per pixel interleaved. 8-bit left, 8-bit right. */
  Y8I,
  /** 12-bit per pixel interleaved. 12-bit left, 12-bit right. Each pixel is stored in a 24-bit word in little-endian order. */
  Y12I,
  /** multi-planar Depth 16bit + IR 10bit. */
  INZI,
  /** 8-bit IR stream. */
  INVI,
  /** Grey-scale image as a bit-packed array. 4 pixel data stream taking 5 bytes */
  W10,
}

export enum RSFrameMetadata {
  /** A sequential index managed per-stream. Integer value */
  FrameCounter,
  /** Timestamp set by device clock when data readout and transmit commence. usec */
  FrameTimestamp,
  /** Timestamp of the middle of sensor's exposure calculated by device. usec */
  SensorTimestamp,
  /** Sensor's exposure width. When Auto Exposure (AE) is on the value is controlled by firmware. usec */
  ActualExposure,
  /**
   * A relative value increasing which will increase the Sensor's gain factor.
   * When AE is set On, the value is controlled by firmware. Integer value
   */
  GainLevel,
  /** Auto Exposure Mode indicator. Zero corresponds to AE switched off. */
  AutoExposure,
  /** White Balance setting as a color temperature. Kelvin degrees */
  WhiteBalance,
  /** Time of arrival in system clock */
  TimeOfArrival,
  /** Temperature of the device, measured at the time of the frame capture. Celsius degrees */
  Temperature,
  /** Timestamp get from uvc driver. usec */
  BackendTimestamp,
  /** Actual fps */
  ActualFps,
  /** Laser power value 0-360. */
  FrameLaserPower,
  /**
   * @deprecated use RS2_FjRAME_METADATA_FRAME_EMITTER_MODE
   * Laser power mode. Zero corresponds to Laser power switched off and one for switched on.
   */
  FrameLaserPowerMode,
  /** Exposure priority. */
  ExposurePriority,
  /** Left region of interest for the auto exposure Algorithm. */
  ExposureRoiLeft,
  /** Right region of interest for the auto exposure Algorithm. */
  ExposureRoiRight,
  /** Top region of interest for the auto exposure Algorithm. */
  ExposureRoiTop,
  /** Bottom region of interest for the auto exposure Algorithm. */
  ExposureRoiBottom,
  /** Color image brightness. */
  Brightness,
  /** Color image contrast. */
  Contrast,
  /** Color image saturation. */
  Saturation,
  /** Color image sharpness. */
  Sharpness,
  /** Auto white balance temperature Mode indicator. Zero corresponds to automatic mode switched off. */
  AutoWhiteBalanceTemperature,
  /** Color backlight compensation. Zero corresponds to switched off. */
  BacklightCompensation,
  /** Color image hue. */
  Hue,
  /** Color image gamma. */
  Gamma,
  /** Color image white balance. */
  ManualWhiteBalance,
  /** Power Line Frequency for anti-flickering Off/50Hz/60Hz/Auto. */
  PowerLineFrequency,
  /** Color lowlight compensation. Zero corresponds to switched off. */
  LowLightCompensation,
  /** Emitter mode: 0 � all emitters disabled. 1 � laser enabled. 2 � auto laser enabled (opt). 3 � LED enabled (opt). */
  FrameEmitterMode,
  /** Led power value 0-360. */
  FrameLedPower,
}

export enum RSLogSeverity {
  /** Detailed information about ordinary operations */
  Debug,
  /** Terse information about ordinary operations */
  Info,
  /** Indication of possible failure */
  Warn,
  /** Indication of definite failure */
  Error,
  /** Indication of unrecoverable failure */
  Fatal,
  /** No logging will occur */
  None,
}

export enum RSNotificationCategory {
  /** Frames didn't arrived within 5 seconds */
  FramesTimeout,
  /** Received partial/incomplete frame */
  FrameCorrupted,
  /** Error reported from the device */
  HardwareError,
  /** General Hardeware notification that is not an error */
  HardwareEvent,
  /** Received unknown error from the device */
  UnknownError,
  /** Current firmware version installed is not the latest available */
  FirmwareUpdateRecommended,
  /** A relocalization event has updated the pose provided by a pose sensor */
  PoseRelocalization,
}

export enum RSOption {
  /** Enable / disable color backlight compensation */
  BacklightCompensation,
  /** Color image brightness */
  Brightness,
  /** Color image contrast */
  Contrast,
  /** Controls exposure time of color camera. Setting any value will disable auto exposure */
  Exposure,
  /** Color image gain */
  Gain,
  /** Color image gamma setting */
  Gamma,
  /** Color image hue */
  Hue,
  /** Color image saturation setting */
  Saturation,
  /** Color image sharpness setting */
  Sharpness,
  /** Controls white balance of color image. Setting any value will disable auto white balance */
  WhiteBalance,
  /** Enable / disable color image auto-exposure */
  EnableAutoExposure,
  /** Enable / disable color image auto-white-balance */
  EnableAutoWhiteBalance,
  /** Provide access to several recommend sets of option presets for the depth camera */
  VisualPreset,
  /** Power of the laser emitter, with 0 meaning projector off */
  LaserPower,
  /**
   * Set the number of patterns projected per frame.
   * The higher the accuracy value the more patterns projected.
   * Increasing the number of patterns help to achieve better accuracy.
   * Note that this control is affecting the Depth FPS
   */
  Accuracy,
  /**
   * Motion vs. Range trade-off, with lower values allowing for better motion sensitivity and
   * higher values allowing for better depth range
   */
  MotionRange,
  /** Set the filter to apply to each depth frame. Each one of the filter is optimized per the application requirements */
  FilterOption,
  /**
   * The confidence level threshold used by the Depth algorithm pipe to set whether a pixel will
   * get a valid range or will be marked with invalid range
   */
  ConfidenceThreshold,
  /** Emitter select: 0 � disable all emitters. 1 � enable laser. 2 � enable auto laser. 3 � enable LED. */
  EmitterEnabled,
  /** Number of frames the user is allowed to keep per stream. Trying to hold-on to more frames will cause frame-drops. */
  FramesQueueSize,
  /** Total number of detected frame drops from all streams */
  TotalFrameDrops,
  /** Auto-Exposure modes: Static, Anti-Flicker and Hybrid */
  AutoExposureMode,
  /** Power Line Frequency control for anti-flickering Off/50Hz/60Hz/Auto */
  PowerLineFrequency,
  /** Current Asic Temperature */
  AsicTemperature,
  /** disable error handling */
  ErrorPollingEnabled,
  /** Current Projector Temperature */
  ProjectorTemperature,
  /** Enable / disable trigger to be outputed from the camera to any external device on every depth frame */
  OutputTriggerEnabled,
  /** Current Motion-Module Temperature */
  MotionModuleTemperature,
  /** Number of meters represented by a single depth unit */
  DepthUnits,
  /** Enable/Disable automatic correction of the motion data */
  EnableMotionCorrection,
  /** Allows sensor to dynamically ajust the frame rate depending on lighting conditions */
  AutoExposurePriority,
  /** Color scheme for data visualization */
  ColorScheme,
  /** Perform histogram equalization post-processing on the depth data */
  HistogramEqualizationEnabled,
  /** Minimal distance to the target */
  MinDistance,
  /** Maximum distance to the target */
  MaxDistance,
  /** Texture mapping stream unique ID */
  TextureSource,
  /** The 2D-filter effect. The specific interpretation is given within the context of the filter */
  FilterMagnitude,
  /** 2D-filter parameter controls the weight/radius for smoothing. */
  FilterSmoothAlpha,
  /** 2D-filter range/validity threshold */
  FilterSmoothDelta,
  /** Enhance depth data post-processing with holes filling where appropriate */
  HolesFill,
  /** The distance in mm between the first and the second imagers in stereo-based depth cameras */
  StereoBaseline,
  /** Allows dynamically ajust the converge step value of the target exposure in Auto-Exposure algorithm */
  AutoExposureConvergeStep,
  /** Impose Inter-camera HW synchronization mode. Applicable for D400/Rolling Shutter SKUs */
  InterCamSyncMode,
  /** Select a stream to process */
  StreamFilter,
  /** Select a stream format to process */
  StreamFormatFilter,
  /** Select a stream index to process */
  StreamIndexFilter,
  /** When supported, this option make the camera to switch the emitter state every frame. 0 for disabled, 1 for enabled */
  EmitterOnOff,
  /** Zero order point x */
  ZeroOrderPointX,
  /** Zero order point y */
  ZeroOrderPointY,
  /** LLD temperature */
  LldTemperature,
  /** MC temperature */
  McTemperature,
  /** MA temperature */
  MaTemperature,
  /** Hardware stream configuration */
  HardwarePreset,
  /** disable global time  */
  GlobalTimeEnabled,
  /** APD temperature */
  ApdTemperature,
  /** Enable an internal map */
  EnableMapping,
  /** Enable appearance based relocalization */
  EnableRelocalization,
  /** Enable position jumping */
  EnablePoseJumping,
  /** Enable dynamic calibration */
  EnableDynamicCalibration,
  /** Offset from sensor to depth origin in millimetrers */
  DepthOffset,
  /** Power of the LED (light emitting diode), with 0 meaning LED off */
  LedPower,
  /** Toggle Zero-Order mode */
  ZeroOrderEnabled,
  /** Preserve previous map when starting */
  EnableMapPreservation,
}

export enum RSStreamType {
  Any,
  /** Native stream of depth data produced by RealSense device */
  Depth,
  /** Native stream of color data captured by RealSense device */
  Color,
  /** Native stream of infrared data captured by RealSense device */
  Infrared,
  /** Native stream of fish-eye (wide) data captured from the dedicate motion camera */
  Fisheye,
  /** Native stream of gyroscope motion data produced by RealSense device */
  Gyro,
  /** Native stream of accelerometer motion data produced by RealSense device */
  Accel,
  /** Signals from external device connected through GPIO */
  Gpio,
  /** 6 Degrees of Freedom pose data, calculated by RealSense device */
  Pose,
  /** 4 bit per-pixel depth confidence level */
  Confidence,
}
