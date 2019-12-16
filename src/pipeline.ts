import * as addon from './addon';
import { RSContext, RSPipeline, RSFrameSet } from './types';

export class Pipeline {
  /**
   * Construct a Pipeline object
   * There are 2 acceptable syntax
   *
   * <pre><code>
   *  Syntax 1. new Pipeline()
   *  Syntax 2. new Pipeline(context)
   * </code></pre>
   * Syntax 1 uses the default context.
   * Syntax 2 used the context created by application
   * @param {Context} [context] - the {@link Context} that is being used by the pipeline
   */
  private readonly ctx: RSContext;
  private readonly pipeline: RSPipeline;
  private readonly frameSet: RSFrameSet;
  private started: boolean;

  constructor() {
    this.ctx = addon.createContext();
    this.pipeline = addon.createPipeline();
    this.frameSet = addon.createFrameSet();
    this.pipeline.create(this.ctx);
    this.started = false;

    // internal.addObject(this);
  }

  /**
   * Destroy the resource associated with this pipeline
   */
  destroy() {
    this.stop();
    this.pipeline.destroy();
    this.ctx.destroy();
    this.frameSet.destroy();
  }

  /**
   * Start streaming
   * There are 2 acceptable syntax
   * Syntax 1 uses the default configuration.
   * Syntax 2 used the configured streams and or device of the config parameter
   *
   * @param {Config} [config] - the {@link Config} object to use for configuration
   * @return {@link PipelineProfile}
   */
  start() {
    const funcName = 'Pipeline.start()';
    checkArgumentLength(0, 1, arguments.length, funcName);
    if (this.started === true) return undefined;

    if (arguments.length === 0) {
      this.started = true;
      return new PipelineProfile(this.cxxPipeline.start());
    } else {
      checkArgumentType(arguments, Config, 0, funcName);
      this.started = true;
      return new PipelineProfile(this.cxxPipeline.startWithConfig(arguments[0].cxxConfig));
    }
  }

  /**
   * Stop streaming
   *
   * @return {undefined}
   */
  stop() {
    if (this.started === false) return;

    this.pipeline.stop();
    this.started = false;
    this.frameSet.release();
  }

  /**
   * Wait until a new set of frames becomes available.
   * The returned frameset includes time-synchronized frames of each enabled stream in the pipeline.
   * In case of different frame rates of the streams, the frames set include a matching frame of the
   * slow stream, which may have been included in previous frames set.
   * The method blocks the calling thread, and fetches the latest unread frames set.
   * Device frames, which were produced while the function wasn't called, are dropped.
   * To avoid frame drops, this method should be called as fast as the device frame rate.
   *
   * @param {Integer} timeout - max time to wait, in milliseconds, default to 5000 ms
   * @return {FrameSet|undefined} a FrameSet object or Undefined
   * @see See [Pipeline.latestFrame]{@link Pipeline#latestFrame}
   */
  waitForFrames(timeout = 5000) {
    const funcName = 'Pipeline.waitForFrames()';
    checkArgumentLength(0, 1, arguments.length, funcName);
    checkArgumentType(arguments, 'number', 0, funcName);
    this.frameSet.release();
    if (this.cxxPipeline.waitForFrames(this.frameSet.cxxFrameSet, timeout)) {
      this.frameSet.__update();
      return this.frameSet;
    }
    return undefined;
  }

  get latestFrame() {
    return this.frameSet;
  }

  /**
   * Check if a new set of frames is available and retrieve the latest undelivered set.
   * The frameset includes time-synchronized frames of each enabled stream in the pipeline.
   * The method returns without blocking the calling thread, with status of new frames available
   * or not. If available, it fetches the latest frames set.
   * Device frames, which were produced while the function wasn't called, are dropped.
   * To avoid frame drops, this method should be called as fast as the device frame rate.
   *
   * @return {FrameSet|undefined}
   */
  pollForFrames() {
    this.frameSet.release();
    if (this.cxxPipeline.pollForFrames(this.frameSet.cxxFrameSet)) {
      this.frameSet.__update();
      return this.frameSet;
    }
    return undefined;
  }

  /**
   * Return the active device and streams profiles, used by the pipeline.
   * The pipeline streams profiles are selected during {@link Pipeline.start}. The method returns a
   * valid result only when the pipeline is active -
   * between calls to {@link Pipeline.start} and {@link Pipeline.stop}.
   * After {@link Pipeline.stop} is called, the pipeline doesn't own the device, thus, the pipeline
   * selected device may change in
   * subsequent activations.
   *
   * @return {PipelineProfile} the actual pipeline device and streams profile, which was
   * successfully configured to the streaming device on start.
   */
  getActiveProfile() {
    if (this.started === false) return undefined;

    return new PipelineProfile(this.cxxPipeline.getActiveProfile());
  }
}
