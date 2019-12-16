/**
 * Class represents a stream configuration
 */
export class StreamProfile {
  constructor(cxxProfile) {
    this.cxxProfile = cxxProfile;
    this.streamValue = this.cxxProfile.stream();
    this.formatValue = this.cxxProfile.format();
    this.fpsValue = this.cxxProfile.fps();
    this.indexValue = this.cxxProfile.index();
    this.uidValue = this.cxxProfile.uniqueID();
    this.isDefaultValue = this.cxxProfile.isDefault();
  }

  /**
   * Extrinsics:
   * @typedef {Object} ExtrinsicsObject
   * @property {Float32[]} rotation - Array(9), Column-major 3x3 rotation matrix
   * @property {Float32[]} translation - Array(3), Three-element translation vector, in meters
   * @see [StreamProfile.getExtrinsicsTo()]{@link StreamProfile#getExtrinsicsTo}
   */

  destroy() {
    if (this.cxxProfile) {
      this.cxxProfile.destroy();
      this.cxxProfile = undefined;
    }
  }

  static _internalCreateStreamProfile(cxxProfile) {
    if (cxxProfile.isMotionProfile()) {
      return new MotionStreamProfile(cxxProfile);
    } else if (cxxProfile.isVideoProfile()) {
      return new VideoStreamProfile(cxxProfile);
    } else {
      return new StreamProfile(cxxProfile);
    }
  }
}

class VideoStreamProfile extends StreamProfile {
  /**
   * Construct a device object, representing a RealSense camera
   */
  constructor(cxxProfile) {
    super(cxxProfile);

    // TODO(tinshao): determine right width and height value
    this.widthValue = this.cxxProfile.width();
    this.heightValue = this.cxxProfile.height();
  }

  /**
   * Width in pixels of the video stream
   *
   * @return {Integer}
   */
  get width() {
    return this.widthValue;
  }


  /**
   * height in pixels of the video stream
   *
   * @return {Integer}
   */
  get height() {
    return this.heightValue;
  }

  /**
   * Stream intrinsics:
   * @typedef {Object} IntrinsicsObject
   * @property {Integer} width - Width of the image in pixels
   * @property {Integer} height - Height of the image in pixels
   * @property {Float32} ppx - Horizontal coordinate of the principal point of the image, as a
   * pixel offset from the left edge
   * @property {Float32} ppy - Vertical coordinate of the principal point of the image, as a pixel
   * offset from the top edge
   * @property {Float32} fx - Focal length of the image plane, as a multiple of pixel width
   * @property {Float32} fy - Focal length of the image plane, as a multiple of pixel height
   * @property {Integer} model - Distortion model of the image, see
   * @property {Float32[]} coeffs - Array(5), Distortion coefficients
   * @see [StreamProfile.getIntrinsics()]{@link StreamProfile#getIntrinsics}
   */

  /**
   * When called on a VideoStreamProfile, returns the intrinsics of specific stream configuration
   * @return {IntrinsicsObject|undefined}
   */
  getIntrinsics() {
    return this.cxxProfile.getVideoStreamIntrinsics();
  }
}
