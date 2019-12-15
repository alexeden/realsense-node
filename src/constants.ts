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
  /** Provide access to several recommend sets of option presets for the depth camera  */
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
  /** Total number of detected frame drops from all streams  */
  TotalFrameDrops,
  /** Auto-Exposure modes: Static, Anti-Flicker and Hybrid  */
  AutoExposureMode,
  /** Power Line Frequency control for anti-flickering Off/50Hz/60Hz/Auto  */
  PowerLineFrequency,
  /** Current Asic Temperature  */
  AsicTemperature,
  /** disable error handling  */
  ErrorPollingEnabled,
  /** Current Projector Temperature  */
  ProjectorTemperature,
  /** Enable / disable trigger to be outputed from the camera to any external device on every depth frame  */
  OutputTriggerEnabled,
  /** Current Motion-Module Temperature  */
  MotionModuleTemperature,
  /** Number of meters represented by a single depth unit  */
  DepthUnits,
  /** Enable/Disable automatic correction of the motion data  */
  EnableMotionCorrection,
  /** Allows sensor to dynamically ajust the frame rate depending on lighting conditions  */
  AutoExposurePriority,
  /** Color scheme for data visualization  */
  ColorScheme,
  /** Perform histogram equalization post-processing on the depth data  */
  HistogramEqualizationEnabled,
  /** Minimal distance to the target  */
  MinDistance,
  /** Maximum distance to the target  */
  MaxDistance,
  /** Texture mapping stream unique ID  */
  TextureSource,
  /** The 2D-filter effect. The specific interpretation is given within the context of the filter  */
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
  /** Impose Inter-camera HW synchronization mode. Applicable for D400/Rolling Shutter SKUs  */
  InterCamSyncMode,
  /** Select a stream to process  */
  StreamFilter,
  /** Select a stream format to process  */
  StreamFormatFilter,
  /** Select a stream index to process  */
  StreamIndexFilter,
  /** When supported, this option make the camera to switch the emitter state every frame. 0 for disabled, 1 for enabled  */
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
  /** Hardware stream configuration  */
  HardwarePreset,
  /** disable global time   */
  GlobalTimeEnabled,
  /** APD temperature */
  ApdTemperature,
  /** Enable an internal map  */
  EnableMapping,
  /** Enable appearance based relocalization  */
  EnableRelocalization,
  /** Enable position jumping  */
  EnablePoseJumping,
  /** Enable dynamic calibration  */
  EnableDynamicCalibration,
  /** Offset from sensor to depth origin in millimetrers */
  DepthOffset,
  /** Power of the LED (light emitting diode), with 0 meaning LED off */
  LedPower,
  /** Toggle Zero-Order mode  */
  ZeroOrderEnabled,
  /** Preserve previous map when starting  */
  EnableMapPreservation,
}
