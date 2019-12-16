// import * as addon from './addon';
// // import { RSContext, RSPipeline, RSFrameSet } from './types';
// import { RSStream } from './constants';

// /**
//  * Class containing a set of frames
//  *
//  * @property {Integer} size - count of frames.
//  * @property {DepthFrame|undefined} depthFrame - The depth frame in the frameset.
//  * @property {VideoFrame|undefined} colorFrame - The color frame in the frameset.
//  */
// export class FrameSet {
//   size: number;

//   constructor(
//     private readonly cxxFrameSet = addon.createFrameSet()
//   ) {
//     this.cache = [];
//     this.cacheMetadata = [];
//     // this.__update();
//   }

//   // /**
//   //  * Count of frames
//   //  *
//   //  * @return {Integer}
//   //  */
//   // get size() {
//   //   return this.cxxFrameSet.getSize();
//   // }

//   // /**
//   //  * Get the depth frame
//   //  *
//   //  * @return {DepthFrame|undefined}
//   //  */
//   // get depthFrame() {
//   //   return this.getFrame(RSStream.Depth, 0);
//   // }

//   // /**
//   //  * Get the color frame
//   //  *
//   //  * @return {VideoFrame|undefined}
//   //  */
//   // get colorFrame() {
//   //   return this.getFrame(RSStream.Color, 0);
//   // }

//   // /**
//   //  * Get the infrared frame
//   //  * @param {Integer} streamIndex index of the expected infrared stream
//   //  * @return {VideoFrame|undefined}
//   //  */
//   // getInfraredFrame(streamIndex = 0) {
//   //   return this.getFrame(RSStream.Infrared, streamIndex);
//   // }

//   /**
//    * Get the frame at specified index
//    *
//    * @param {Integer} index the index of the expected frame (Note: this is not
//    * stream index)
//    * @return {DepthFrame|VideoFrame|Frame|undefined}
//    */
//   at(index: number) {
//     return this.getFrame(
//       this.cxxFrameSet.indexToStream(index),
//       this.cxxFrameSet.indexToStreamIndex(index)
//     );
//   }

//   /**
//    * Run the provided callback function with each Frame inside the FrameSet
//    * @param {FrameCallback} callback the callback function to process each frame
//    * @return {undefined}
//    */
//   forEach(callback) {
//     const funcName = 'FrameSet.forEach()';
//     checkArgumentLength(1, 1, arguments.length, funcName);
//     checkArgumentType(arguments, 'function', 0, funcName);
//     const size = this.size;
//     for (let i = 0; i < size; i++) {
//       callback(this.at(i));
//     }
//   }

//   __internalGetFrame(stream, streamIndex) {
//     let cxxFrame = this.cxxFrameSet.getFrame(stream, streamIndex);
//     return (cxxFrame ? Frame._internalCreateFrame(cxxFrame) : undefined);
//   }

//   __internalFindFrameInCache(stream, streamIndex) {
//     if (stream === RSStream.Any) {
//       return (this.cacheMetadata.size ? 0 : undefined);
//     }

//     for (const [i, data] of this.cacheMetadata.entries()) {
//       if (data.stream !== stream) {
//         continue;
//       }
//       if (!streamIndex || (streamIndex && streamIndex === data.streamIndex)) {
//         return i;
//       }
//     }
//     return undefined;
//   }

//   __internalGetFrameCache(stream, streamIndex, callback) {
//     let idx = this.__internalFindFrameInCache(stream, streamIndex);
//     if (idx === undefined) {
//       let frame = callback(stream, streamIndex);
//       if (!frame) return undefined;

//       this.cache.push(frame);
//       // the stream parameter may be RSStream.Any, but when we store the frame in
//       // cache, we shall store its actual stream type.
//       this.cacheMetadata.push({stream: frame.streamType, streamIndex});
//       idx = this.cache.length - 1;
//     } else {
//       let frame = this.cache[idx];
//       if (!frame.cxxFrame) {
//         frame.cxxFrame = new RS2.RSFrame();
//       }

//       // as cache metadata entries always use actual stream type, we use the actual
//       // stream types to easy native from processing RSStream.Any
//       if (! this.cxxFrameSet.replaceFrame(
//           this.cacheMetadata[idx].stream, streamIndex, frame.cxxFrame)) {
//         this.cache[idx] = undefined;
//         this.cacheMetadata[idx] = undefined;
//       }
//     }
//     return this.cache[idx];
//   }

//   /**
//    * Get the frame with specified stream
//    *
//    * @param {Integer|String} stream stream type of the frame
//    * @param {Integer} streamIndex index of the stream, 0 means the first
//    * matching stream
//    * @return {DepthFrame|VideoFrame|Frame|undefined}
//    */
//   getFrame(stream: RSStream, streamIndex = 0) {
//     const s = checkArgumentType(arguments, constants.stream, 0, funcName);
//     if (arguments.length === 2) {
//       checkArgumentType(arguments, 'integer', 1, funcName);
//     }
//     return this.__internalGetFrameCache(s, streamIndex, this.__internalGetFrame.bind(this));
//   }

//   // __update() {
//   //   this.sizeValue = this.cxxFrameSet.getSize();
//   // }

//   releaseCache() {
//     this.cache.forEach((f) => {
//       if (f && f.cxxFrame) {
//         f.release();
//       }
//     });
//     this.cache = [];
//     this.cacheMetadata = [];
//   }

//   release() {
//     this.releaseCache();
//     if (this.cxxFrameSet) this.cxxFrameSet.destroy();
//   }

//   /**
//    * Release resources associated with this object
//    *
//    * @return {undefined}
//    */
//   destroy() {
//     this.release();
//     this.cxxFrameSet = undefined;
//   }
// }
