import { addon } from './addon';
// import { RSContext, RSPipeline, RSFrameSet } from './types';
import { RSStreamType } from './constants';
import { RSFrame } from './types';

interface CachedMetadata {
  stream: RSStreamType;
  streamIndex: number;
}

/**
 * Class containing a set of frames
 *
 * @property {Integer} size - count of frames.
 * @property {DepthFrame|undefined} depthFrame - The depth frame in the frameset.
 * @property {VideoFrame|undefined} colorFrame - The color frame in the frameset.
 */
export class FrameSet {
  private cache: Array<RSFrame | undefined>;
  private cacheMetadata: Array<CachedMetadata | undefined>;

  constructor(
    private readonly cxxFrameSet = new addon.RSFrameSet()
  ) {
    this.cache = [];
    this.cacheMetadata = [];
    // this.__update();
  }

  /**
   * Count of frames
   */
  get size() {
    return this.cxxFrameSet.getSize();
  }

  // /**
  //  * Get the depth frame
  //  *
  //  * @return {DepthFrame|undefined}
  //  */
  // get depthFrame() {
  //   return this.getFrame(RSStreamType.Depth, 0);
  // }

  // /**
  //  * Get the color frame
  //  *
  //  * @return {VideoFrame|undefined}
  //  */
  // get colorFrame() {
  //   return this.getFrame(RSStreamType.Color, 0);
  // }

  // /**
  //  * Get the infrared frame
  //  * @param {Integer} streamIndex index of the expected infrared stream
  //  * @return {VideoFrame|undefined}
  //  */
  // getInfraredFrame(streamIndex = 0) {
  //   return this.getFrame(RSStreamType.Infrared, streamIndex);
  // }

  /**
   * Get the frame at specified index
   *
   * @param {Integer} index the index of the expected frame (Note: this is not
   * stream index)
   * @return {DepthFrame|VideoFrame|Frame|undefined}
   */
  at(index: number) {
    return this.getFrame(
      this.cxxFrameSet.indexToStream(index),
      this.cxxFrameSet.indexToStreamIndex(index)
    );
  }

  private findCachedFrameIndex(
    stream: RSStreamType,
    streamIndex: number
  ) {
    if (stream === RSStreamType.Any) {
      return (this.cacheMetadata.length ? 0 : undefined);
    }

    for (const [i, data] of [...this.cacheMetadata.entries()].filter(([, metadata]) => !!metadata)) {
      if (data!.stream !== stream) {
        continue;
      }
      if (!streamIndex || (streamIndex && streamIndex === data!.streamIndex)) {
        return i;
      }
    }

    return undefined;
  }

  /**
   * Get the frame with specified stream
   *
   * @param {Integer|String} stream stream type of the frame
   * @param {Integer} streamIndex index of the stream, 0 means the first
   * matching stream
   * @return {DepthFrame|VideoFrame|Frame|undefined}
   */
  getFrame(stream: RSStreamType, streamIndex = 0) {
    let idx = this.findCachedFrameIndex(stream, streamIndex);
    if (idx === undefined) {
      const frame = this.cxxFrameSet.getFrame(stream, streamIndex);
      if (!frame) return undefined;

      this.cache.push(frame);
      // the stream parameter may be RSStreamType.Any, but when we store the frame in
      // cache, we shall store its actual stream type.
      this.cacheMetadata.push({
        stream: frame.getStreamProfile().streamType,
        streamIndex,
      });

      idx = this.cache.length - 1;
    }
    else {
      const frame = this.cache[idx]!;

      // as cache metadata entries always use actual stream type, we use the actual
      // stream types to easy native from processing RSStreamType.Any
      if (!this.cxxFrameSet.replaceFrame(this.cacheMetadata[idx]!.stream, streamIndex, frame)) {
        this.cache[idx] = undefined;
        this.cacheMetadata[idx] = undefined;
      }
    }

    return this.cache[idx];
  }

  destroy() {
    this.clearCache();
    if (this.cxxFrameSet) this.cxxFrameSet.destroy();
  }

  releaseAndReturn() {
    this.destroy();

    return this.cxxFrameSet;
  }


  private clearCache() {
    this.cache.forEach(f => {
      if (f) {
        f.getStreamProfile().destroy();
        f.destroy();
      }
    });
    this.cache = [];
    this.cacheMetadata = [];
  }
}
