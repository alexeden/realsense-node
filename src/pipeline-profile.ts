import { deleteAutomatically } from 'addon';

/**
 * The pipeline profile includes a device and a selection of active streams, with specific profile.
 * The profile is a selection of the above under filters and conditions defined by the pipeline.
 * Streams may belong to more than one sensor of the device.
 */
export class PipelineProfile {
  constructor(profile) {
    this.cxxPipelineProfile = profile;
    deleteAutomatically(this);
  }

  /**
   * Return the selected streams profiles, which are enabled in this profile.
   *
   * @return {StreamProfile[]} an array of StreamProfile
   */
  getStreams() {
    let profiles = this.cxxPipelineProfile.getStreams();
    if (!profiles) return undefined;

    const array = [];
    profiles.forEach((profile) => {
      array.push(StreamProfile._internalCreateStreamProfile(profile));
    });
    return array;
  }

  /**
   * Return the selected stream profile, which are enabled in this profile.
   * @param {Integer|String} streamType the stream type of the desired profile,
   * see {@link stream} for avaiable values
   * @param {Integer} streamIndex stream index of the desired profile, -1 for any matching
   * @return {StreamProfile} the first matching stream profile
   */
  getStream(streamType, streamIndex = -1) {
    const funcName = 'PipelineProfile.getStream()';
    checkArgumentLength(1, 2, arguments.length, funcName);
    const s = checkArgumentType(arguments, constants.stream, 0, funcName);
    checkArgumentType(arguments, 'number', 1, funcName);
    let profiles = this.getStreams();
    if (!profiles) {
      return undefined;
    }
    for (let i = 0; i < profiles.length; i++) {
      if (profiles[i].streamType === s &&
          (streamIndex === -1 || (streamIndex === profiles[i].indexValue))) {
        return profiles[i];
      }
    }
    return undefined;
  }

  /**
   * Retrieve the device used by the pipeline.
   * The device class provides the application access to control camera additional settings -
   * get device information, sensor options information, options value query and set, sensor
   * specific extensions.
   * Since the pipeline controls the device streams configuration, activation state and frames
   * reading, calling the device API functions, which execute those operations, results in
   * unexpected behavior. The pipeline streaming device is selected during {@link Pipeline.start}.
   * Devices of profiles, which are not returned by
   * {@link Pipeline.start} or {@link Pipeline.getActiveProfile}, are not guaranteed to be used by
   * the pipeline.
   *
   * @return {Device} the pipeline selected device
   */
  getDevice() {
    return Device._internalCreateDevice(this.cxxPipelineProfile.getDevice());
  }

  /**
   * Destroy the resource associated with this object
   *
   * @return {undefined}
   */
  destroy() {
    if (this.cxxPipelineProfile) {
      this.cxxPipelineProfile.destroy();
      this.cxxPipelineProfile = undefined;
    }
  }
}
