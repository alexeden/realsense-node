/**
 * A RealSense camera
 */
class Device {
  constructor(cxxDev, autoDelete = true) {
    this.cxxDev = cxxDev;
    if (autoDelete) {
      internal.addObject(this);
    }
  }

  /**
   * Check if everything is OK, e.g. if the device object is connected to underlying hardware
   * @return {Boolean}
   */
  get isValid() {
    return (this.cxxDev !== null);
  }

  /**
   * get an array of adjacent sensors, sharing the same physical parent composite device
   * @return {Sensor[]}
   */
  querySensors() {
    let sensors = this.cxxDev.querySensors();
    if (!sensors) return undefined;

    const array = [];
    sensors.forEach((s) => {
      if (s.isDepthSensor()) {
        array.push(new DepthSensor(s));
      } else {
        array.push(new Sensor(s));
      }
    });
    return array;
  }

  /**
   * Get the first sensor
   * @return {Sensor|undefined}
   */
  get first() {
    let sensors = this.querySensors();
    if (sensors && sensors.length > 0) {
      return sensors[0];
    }
    return undefined;
  }

  /**
   * Information that can be queried from the device.
   * Not all information attributes are available on all camera types.
   * This information is mainly available for camera debug and troubleshooting and should not be
   * used in applications.
   * @typedef {Object} CameraInfoObject
   * @property {String|undefined} name - Device friendly name. <br> undefined is not
   * supported.
   * @property {String|undefined} serialNumber - Device serial number. <br> undefined is not
   * supported.
   * @property {String|undefined} firmwareVersion - Primary firmware version.
   * <br> undefined is not supported.
   * @property {String|undefined} physicalPort - Unique identifier of the port the device is
   * connected to (platform specific). <br> undefined is not supported.
   * @property {String|undefined} debugOpCode - If device supports firmware logging, this is the
   * command to send to get logs from firmware. <br> undefined is not supported.
   * @property {String|undefined} advancedMode - True if the device is in advanced mode.
   * <br> undefined is not supported.
   * @property {String|undefined} productId - Product ID as reported in the USB descriptor.
   * <br> undefined is not supported.
   * @property {Boolean|undefined} cameraLocked - True if EEPROM is locked. <br> undefined is not
   * supported.
   * @property {String|undefined} usbTypeDescriptor - Designated USB specification: USB2/USB3.
   * <br> undefined is not supported.
   * @property {String|undefined} recommendedFirmwareVersion - Latest firmware version.
   * <br> undefined is not supported.
   * @see [Device.getCameraInfo()]{@link Device#getCameraInfo}
   */

  /**
   * Get camera information
   * There are 2 acceptable forms of syntax:
   * <pre><code>
   *  Syntax 1. getCameraInfo()
   *  Syntax 2. getCameraInfo(info)
   * </code></pre>
   *
   * @param {String|Integer} [info] - the camera_info type, see {@link camera_info} for available
   * values
   * @return {CameraInfoObject|String|undefined} if no argument is provided, {CameraInfoObject} is
   * returned. If a camera_info is provided, the specific camera info value string is returned.
   */
  getCameraInfo(info) {
    const funcName = 'Device.getCameraInfo()';
    checkArgumentLength(0, 1, arguments.length, funcName);
    if (arguments.length === 0) {
      let result = {};
      if (this.cxxDev.supportsCameraInfo(camera_info.CAMERA_INFO_NAME)) {
        result.name = this.cxxDev.getCameraInfo(camera_info.CAMERA_INFO_NAME);
      }
      if (this.cxxDev.supportsCameraInfo(camera_info.CAMERA_INFO_SERIAL_NUMBER)) {
        result.serialNumber = this.cxxDev.getCameraInfo(camera_info.CAMERA_INFO_SERIAL_NUMBER);
      }
      if (this.cxxDev.supportsCameraInfo(camera_info.CAMERA_INFO_FIRMWARE_VERSION)) {
        result.firmwareVersion = this.cxxDev.getCameraInfo(
            camera_info.CAMERA_INFO_FIRMWARE_VERSION);
      }
      if (this.cxxDev.supportsCameraInfo(camera_info.CAMERA_INFO_PHYSICAL_PORT)) {
        result.physicalPort = this.cxxDev.getCameraInfo(camera_info.CAMERA_INFO_PHYSICAL_PORT);
      }
      if (this.cxxDev.supportsCameraInfo(camera_info.CAMERA_INFO_DEBUG_OP_CODE)) {
        result.debugOpCode = this.cxxDev.getCameraInfo(camera_info.CAMERA_INFO_DEBUG_OP_CODE);
      }
      if (this.cxxDev.supportsCameraInfo(camera_info.CAMERA_INFO_ADVANCED_MODE)) {
        result.advancedMode = this.cxxDev.getCameraInfo(camera_info.CAMERA_INFO_ADVANCED_MODE);
      }
      if (this.cxxDev.supportsCameraInfo(camera_info.CAMERA_INFO_PRODUCT_ID)) {
        result.productId = this.cxxDev.getCameraInfo(camera_info.CAMERA_INFO_PRODUCT_ID);
      }
      if (this.cxxDev.supportsCameraInfo(camera_info.CAMERA_INFO_CAMERA_LOCKED)) {
        result.cameraLocked = this.cxxDev.getCameraInfo(camera_info.CAMERA_INFO_CAMERA_LOCKED);
      }
      if (this.cxxDev.supportsCameraInfo(camera_info.CAMERA_INFO_USB_TYPE_DESCRIPTOR)) {
        result.usbTypeDescriptor = this.cxxDev.getCameraInfo(
            camera_info.CAMERA_INFO_USB_TYPE_DESCRIPTOR);
      }
      if (this.cxxDev.supportsCameraInfo(camera_info.CAMERA_INFO_RECOMMENDED_FIRMWARE_VERSION)) {
        result.recommendedFirmwareVersion = this.cxxDev.getCameraInfo(
            camera_info.CAMERA_INFO_RECOMMENDED_FIRMWARE_VERSION);
      }
      return result;
    } else {
      const val = checkArgumentType(arguments, constants.camera_info, 0, funcName);
      return (this.cxxDev.supportsCameraInfo(val) ? this.cxxDev.getCameraInfo(val) : undefined);
    }
  }

  get cameraInfo() {
    return this.getCameraInfo();
  }

  /**
   * Check if specific camera info is supported.
   * @param {String|Integer} info - info type to query. See {@link camera_info} for available values
   * @return {Boolean|undefined} Returns undefined if an invalid info type was specified.
   * @see enum {@link camera_info}
   * @example <caption>Example of 3 equivalent calls of the same query</caption>
   * device.supportsCameraInfo('name');
   * device.supportsCameraInfo(realsense2.camera_info.camera_info_name);
   * device.supportsCameraInfo(realsense2.camera_info.CAMERA_INFO_NAME);
   */
  supportsCameraInfo(info) {
    const funcName = 'Device.supportsCameraInfo()';
    checkArgumentLength(1, 1, arguments.length, funcName);
    const i = checkArgumentType(arguments, constants.camera_info, 0, funcName);
    return this.cxxDev.supportsCameraInfo(i);
  }

  /**
   * Send hardware reset request to the device.
   * @return {undefined}
   */
  reset() {
    this.cxxDev.reset();
  }

  /**
   * Release resources associated with the object
   */
  destroy() {
    if (this.cxxDev) {
      this.cxxDev.destroy();
      this.cxxDev = undefined;
    }
    this._events = undefined;
  }

  static _internalCreateDevice(cxxDevice) {
    return cxxDevice.isTm2() ? new Tm2(cxxDevice) : new Device(cxxDevice);
  }
}
