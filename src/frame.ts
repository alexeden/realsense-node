

/**
 * This class resprents a picture frame
 *
 * @property {Boolean} isValid - True if the frame is valid, otherwise false.
 * @property {Uint16Array|Uint8Array} data - A typed array representing the data.
 *  <br>The type of the typed array depends on the <code>format</code> specified in camera
 * configuration.
 * @property {Integer} width - The width of the frame.
 * @property {Integer} height - The height of the frame.
 * @property {Integer|Int64} frameNumber - An integer or an object representing the frame number.
 *  <br>If the frame number is less than 2^53, then the return value is an integer number;
 *  <br>Otherwise it will be an <code>Int64</code> object defined in npm module "node-int64"
 * @property {Number} timestamp - The timestamp of the frame.
 * @property {Integer} streamType - The stream type of the frame.
 * see <code>enum {@link stream}</code>
 * @property {Integer} bitsPerPixel - The number of bits per pixel
 * @property {Integer} timestampDomain - Get the domain (clock name) of timestamp value.
 */
class Frame {
  static _internalCreateFrame(cxxFrame) {
    if (!cxxFrame) return undefined;
    if (cxxFrame.isPoseFrame()) return new PoseFrame(cxxFrame);
    if (cxxFrame.isMotionFrame()) return new MotionFrame(cxxFrame);
    if (cxxFrame.isDisparityFrame()) return new DisparityFrame(cxxFrame);
    if (cxxFrame.isDepthFrame()) return new DepthFrame(cxxFrame);
    if (cxxFrame.isVideoFrame()) return new VideoFrame(cxxFrame);
    return new Frame(cxxFrame);
  }

  constructor(cxxFrame) {
    this.cxxFrame = cxxFrame || new RS2.RSFrame();
    this.updateProfile();
    internal.addObject(this);
    // called from native to reset this.arrayBuffer and this.typedArray when the
    // underlying frame was replaced. The arrayBuffer and typedArray must be reset
    // to avoid deprecated data to be used.
    const jsWrapper = this;
    this.cxxFrame._internalResetBuffer = function() {
      jsWrapper.typedArray = undefined;
      jsWrapper.arrayBuffer = undefined;
    };
  }

  updateProfile() {
    this.streamProfile = undefined;
    if (this.cxxFrame) {
      let cxxProfile = this.cxxFrame.getStreamProfile();
      if (cxxProfile) {
        this.streamProfile = StreamProfile._internalCreateStreamProfile(cxxProfile);
      }
    }
  }

  release() {
    if (this.cxxFrame) this.cxxFrame.destroy();
    if (this.streamProfile) this.streamProfile.destroy();
    this.arrayBuffer = undefined;
    this.typedArray = undefined;
  }

  /**
   * Destroy the frame and its resource
   */
  destroy() {
    this.release();
    this.cxxFrame = undefined;
    this.StreamProfile = undefined;
  }

  /**
   * Retrieve pixel format of the frame
   * @return {Integer} see enum {@link format} for available values
   */
  get format() {
    return this.streamProfile.format;
  }

  /**
   * Retrieve the origin stream type that produced the frame
   * @return {Integer} see {@link stream} for avaiable values
   */
  get streamType() {
    return this.streamProfile.streamType;
  }

  get profile() {
    return this.streamProfile;
  }

  get width() {
    return this.streamProfile.width ? this.streamProfile.width : this.cxxFrame.getWidth();
  }

  get height() {
    return this.streamProfile.height ? this.streamProfile.height : this.cxxFrame.getHeight();
  }

  /**
   * Check if the frame is valid
   * @return {Boolean}
   */
  get isValid() {
    return (this.cxxFrame && this.cxxFrame.isValid());
  }

  /**
   * Retrieve timestamp from the frame in milliseconds
   * @return {Integer}
   */
  get timestamp() {
    return this.cxxFrame.getTimestamp();
  }

  /**
   * Retrieve timestamp domain. timestamps can only be comparable if they are in common domain
   * (for example, depth timestamp might come from system time while color timestamp might come
   * from the device)
   * this method is used to check if two timestamp values are comparable (generated from the same
   * clock)
   * @return {Integer} see {@link timestamp_domain} for avaiable values
   */
  get timestampDomain() {
    return this.cxxFrame.getTimestampDomain();
  }

  /**
   * Retrieve the current value of a single frame metadata
   * @param {String|Number} metadata the type of metadata, see {@link frame_metadata} for avaiable
   * values
   * @return {Uint8Array} The metadata value, 8 bytes, byte order is bigendian.
   */
  frameMetadata(metadata) {
    const funcName = 'Frame.frameMetadata()';
    checkArgumentLength(1, 1, arguments.length, funcName);
    const m = checkArgumentType(arguments, constants.frame_metadata, 0, funcName);
    const array = new Uint8Array(8);
    return this.cxxFrame.getFrameMetadata(m, array) ? array : undefined;
  }

  /**
   * Determine if the device allows a specific metadata to be queried
   * @param {String|Number} metadata The type of metadata
   * @return {Boolean} true if supported, and false if not
   */
  supportsFrameMetadata(metadata) {
    return this.cxxFrame.supportsFrameMetadata(m);
  }

  /**
   * Retrieve frame number
   * @return {Integer}
   */
  get frameNumber() {
    return this.cxxFrame.getFrameNumber();
  }

  /**
   * Retrieve the frame data
   * @return {Float32Array|Uint16Array|Uint8Array|undefined}
   * if the frame is from the depth stream, the return value is Uint16Array;
   * if the frame is from the XYZ32F or MOTION_XYZ32F stream, the return value is Float32Array;
   * for other cases, return value is Uint8Array.
   */
  get data() {
    if (this.typedArray) return this.typedArray;

    if (!this.arrayBuffer) {
      this.arrayBuffer = this.cxxFrame.getData();
      this.typedArray = undefined;
    }

    if (!this.arrayBuffer) return undefined;

    this.updateProfile();
    switch (this.format) {
      case constants.format.FORMAT_Z16:
      case constants.format.FORMAT_DISPARITY16:
      case constants.format.FORMAT_Y16:
      case constants.format.FORMAT_RAW16:
        this.typedArray = new Uint16Array(this.arrayBuffer);
        return this.typedArray;
      case constants.format.FORMAT_YUYV:
      case constants.format.FORMAT_UYVY:
      case constants.format.FORMAT_RGB8:
      case constants.format.FORMAT_BGR8:
      case constants.format.FORMAT_RGBA8:
      case constants.format.FORMAT_BGRA8:
      case constants.format.FORMAT_Y8:
      case constants.format.FORMAT_RAW8:
      case constants.format.FORMAT_MOTION_RAW:
      case constants.format.FORMAT_GPIO_RAW:
      case constants.format.FORMAT_RAW10:
      case constants.format.FORMAT_ANY:
        this.typedArray = new Uint8Array(this.arrayBuffer);
        return this.typedArray;
      case constants.format.FORMAT_XYZ32F:
      case constants.format.FORMAT_MOTION_XYZ32F:
      case constants.format.FORMAT_6DOF:
      case constants.format.FORMAT_DISPARITY32:
        this.typedArray = new Float32Array(this.arrayBuffer);
        return this.typedArray;
    }
  }

  /**
   * Get the frame buffer data
   *  There are 2 acceptable forms of syntax:
   * <pre><code>
   *  Syntax 1. getData()
   *  Syntax 2. getData(ArrayBuffer)
   * </code></pre>
   *
   * @param {ArrayBuffer} [buffer] The buffer that will be written to.
   * @return {Float32Array|Uint16Array|Uint8Array|undefined}
   *  Returns a <code>TypedArray</code> or <code>undefined</code> for syntax 1,
   *   see {@link Frame#data};
   *  if syntax 2 is used, return value is not used (<code>undefined</code>).
   *
   * @see [VideoFrame.dataByteLength]{@link VideoFrame#dataByteLength} to determine the buffer size
   * in bytes.
   */
  getData(buffer) {
    const funcName = 'Frame.supportsFrameMetadata()';
    checkArgumentLength(0, 1, arguments.length, funcName);
    if (arguments.length === 0) {
      return this.data;
    } else if (arguments.length === 1) {
      checkArgumentType(arguments, 'ArrayBuffer', 0, funcName);
      return this.cxxFrame.writeData(buffer);
    }
  }

  /**
   * communicate to the library you intend to keep the frame alive for a while
   * this will remove the frame from the regular count of the frame pool
   * once this function is called, the SDK can no longer guarantee 0-allocations during frame
   * cycling
   * @return {undefined}
   */
  keep() {
    this.cxxFrame.keep();
  }
}

/**
 * This class resprents a video frame and is a subclass of Frame
 *
 * @property {Integer} width - The image width in pixels.
 * @property {Integer} height - The image height in pixels.
 * @property {Integer} dataByteLength - The length in bytes
 * @property {Integer} strideInBytes - The stride of the frame. The unit is number of bytes.
 * @property {Integer} bitsPerPixel - The number of bits per pixel
 * @property {Integer} bytesPerPixel - The number of bytes per pixel
 */
class VideoFrame extends Frame {
  constructor(frame) {
    super(frame);
  }

  /**
   * Get image width in pixels
   * @return {Integer}
   */
  get width() {
    return this.cxxFrame.getWidth();
  }

  /**
   * Get image height in pixels
   * @return {Integer}
   */
  get height() {
    return this.cxxFrame.getHeight();
  }

  /**
   * Get the data length in bytes
   * @return {Integer}
   */
  get dataByteLength() {
    return this.strideInBytes * this.height;
  }

  /**
   * Retrieve frame stride, the actual line width in bytes (not the logical image width)
   * @return {Integer}
   */
  get strideInBytes() {
    return this.cxxFrame.getStrideInBytes();
  }

  /**
   * Retrieve count of bits per pixel
   * @return {Integer}
   */
  get bitsPerPixel() {
    return this.cxxFrame.getBitsPerPixel();
  }

  /**
   * Retrieve bytes per pixel
   * @return {Integer}
   */
  get bytesPerPixel() {
    return this.cxxFrame.getBitsPerPixel()/8;
  }
}
