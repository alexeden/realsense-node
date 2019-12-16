import { deleteAutomatically } from './addon';
import { RSPipelineProfile } from './types';
import { RSStreamType } from './constants';

/**
 * The pipeline profile includes a device and a selection of active streams, with specific profile.
 * The profile is a selection of the above under filters and conditions defined by the pipeline.
 * Streams may belong to more than one sensor of the device.
 */
export class PipelineProfile {
  constructor(
    readonly cxxPipelineProfile: RSPipelineProfile
  ) {
    deleteAutomatically(this);
  }

  /**
   * Return the selected stream profile, which are enabled in this profile.
   * @param {Integer|String} streamType the stream type of the desired profile,
   * see {@link stream} for avaiable values
   * @param {Integer} streamIndex stream index of the desired profile, -1 for any matching
   * @return {StreamProfile} the first matching stream profile
   */
  getStream(streamType: RSStreamType, streamIndex = -1) {
    const profiles = this.cxxPipelineProfile.getStreams();

    if (!profiles) {
      return undefined;
    }
    // tslint:disable-next-line: prefer-for-of
    for (let i = 0; i < profiles.length; i++) {
      if (profiles[i].streamType === streamType && (streamIndex === -1 || (streamIndex === profiles[i].index))) {
        return profiles[i];
      }
    }

    return undefined;
  }

  // /**
  //  * Retrieve the device used by the pipeline.
  //  * The device class provides the application access to control camera additional settings -
  //  * get device information, sensor options information, options value query and set, sensor
  //  * specific extensions.
  //  * Since the pipeline controls the device streams configuration, activation state and frames
  //  * reading, calling the device API functions, which execute those operations, results in
  //  * unexpected behavior. The pipeline streaming device is selected during {@link Pipeline.start}.
  //  * Devices of profiles, which are not returned by
  //  * {@link Pipeline.start} or {@link Pipeline.getActiveProfile}, are not guaranteed to be used by
  //  * the pipeline.
  //  *
  //  * @return {Device} the pipeline selected device
  //  */
  // getDevice() {
  //   return Device._internalCreateDevice(this.cxxPipelineProfile.getDevice());
  // }

  /**
   * Destroy the resource associated with this object
   *
   * @return {undefined}
   */
  destroy() {
    this.cxxPipelineProfile.destroy();
  }
}
