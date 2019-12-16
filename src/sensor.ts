// import { RSOp } from './types';

/**
 * A sensor device in a RealSense camera
 */
export class Sensor {
  /**
   * Construct a Sensor object, representing a RealSense camera subdevice
   * By default, native resources associated with a Sensor object are freed
   * automatically during cleanup.
   */
  constructor(cxxSensor, autoDelete = true) {
    super(cxxSensor);
    this.cxxSensor = cxxSensor;
    this._events = new EventEmitter();
    if (autoDelete === true) {
      internal.addObject(this);
    }
  }


  /**
   * Open subdevice for exclusive access, by committing to a configuration.
   *  There are 2 acceptable forms of syntax:
   * <pre><code>
   *  Syntax 1. open(streamProfile)
   *  Syntax 2. open(profileArray)
   * </code></pre>
   *  Syntax 2 is for opening multiple profiles in one function call and should be used for
   * interdependent streams, such as depth and infrared, that have to be configured together.
   *
   * @param {StreamProfile} streamProfile configuration commited by the device
   * @param {StreamProfile[]} profileArray configurations array commited by the device
   * @see [Sensor.getStreamProfiles]{@link Sensor#getStreamProfiles} for a list of all supported
   * stream profiles
   */
  open(streamProfile) {
    if (Array.isArray(streamProfile) && streamProfile.length > 0) {
      let cxxStreamProfiles = [];
      for (let i = 0; i < streamProfile.length; i++) {
        if (!(streamProfile[i] instanceof StreamProfile)) {
          throw new TypeError(
              'Sensor.open() expects a streamProfile object or an array of streamProfile objects'); // eslint-disable-line
        }
        cxxStreamProfiles.push(streamProfile[i].cxxProfile);
      }
      this.cxxSensor.openMultipleStream(cxxStreamProfiles);
    } else {
      checkArgumentType(arguments, StreamProfile, 0, funcName);
      this.cxxSensor.openStream(streamProfile.cxxProfile);
    }
  }

  /**
   * Check if specific camera info is supported
   * @param {String|Integer} info - info type to query. See {@link camera_info} for available values
   * @return {Boolean|undefined} Returns undefined if an invalid info type was specified.
   * @see enum {@link camera_info}
   */
  supportsCameraInfo(info) {
    const funcName = 'Sensor.supportsCameraInfo()';
    checkArgumentLength(1, 1, arguments.length, funcName);
    const i = checkArgumentType(arguments, constants.camera_info, 0, funcName);
    return this.cxxSensor.supportsCameraInfo(i);
  }

  /**
   * Get camera information of the sensor
   *
   * @param {String|Integer} info - the camera_info type, see {@link camera_info} for available
   * values
   * @return {String|undefined}
   */
  getCameraInfo(info) {
    const funcName = 'Sensor.getCameraInfo()';
    checkArgumentLength(1, 1, arguments.length, funcName);
    const i = checkArgumentType(arguments, constants.camera_info, 0, funcName);
    return (this.cxxSensor.supportsCameraInfo(i) ? this.cxxSensor.getCameraInfo(i) : undefined);
  }

  /**
   * This callback is called when a frame is captured
   * @callback FrameCallback
   * @param {Frame} frame - The captured frame
   *
   * @see [Sensor.start]{@link Sensor#start}
   */

  /**
   * Start passing frames into user provided callback
   * There are 2 acceptable syntax:
   * <pre><code>
   *  Syntax 1. start(callback)
   *  Syntax 2. start(Syncer)
   * </code></pre>
   *
   * @param {FrameCallback} callback
   * @param {Syncer} syncer, the syncer to synchronize frames
   *
   * @example <caption>Simply do logging when a frame is captured</caption>
   *  sensor.start((frame) => {
   *    console.log(frame.timestamp, frame.frameNumber, frame.data);
   *  });
   *
   */
  start(callback) {
    const funcName = 'Sensor.start()';
    checkArgumentLength(1, 1, arguments.length, funcName);
    if (arguments[0] instanceof Syncer) {
      this.cxxSensor.startWithSyncer(arguments[0].cxxSyncer, false, 0);
    } else {
      checkArgumentType(arguments, 'function', 0, funcName);
      // create object to hold frames generated from native.
      this.frame = new Frame();
      this.depthFrame = new DepthFrame();
      this.videoFrame = new VideoFrame();
      this.disparityFrame = new DisparityFrame();
      this.motionFrame = new MotionFrame();
      this.poseFrame = new PoseFrame();

      let inst = this;
      this.cxxSensor.frameCallback = function() {
        // When the callback is triggered, the underlying frame bas been saved in the objects
        // created above, we need to update it and callback.
        if (inst.disparityFrame.isValid) {
          inst.disparityFrame.updateProfile();
          callback(inst.disparityFrame);
        } else if (inst.depthFrame.isValid) {
          inst.depthFrame.updateProfile();
          callback(inst.depthFrame);
        } else if (inst.videoFrame.isValid) {
          inst.videoFrame.updateProfile();
          callback(inst.videoFrame);
        } else if (inst.motionFrame.isValid) {
          inst.motionFrame.updateProfile();
          callback(inst.motionFrame);
        } else if (inst.poseFrame.isValid) {
          inst.poseFrame.updateProfile();
          callback(inst.poseFrame);
        } else {
          inst.frame.updateProfile();
          callback(inst.frame);
        }
      };
      this.cxxSensor.startWithCallback('frameCallback', this.frame.cxxFrame,
          this.depthFrame.cxxFrame, this.videoFrame.cxxFrame, this.disparityFrame.cxxFrame,
          this.motionFrame.cxxFrame, this.poseFrame.cxxFrame);
    }
  }

  /**
   * stop streaming
   * @return {undefined} No return value
   */
  stop() {
    if (this.cxxSensor) {
      this.cxxSensor.stop();
    }
    if (this.frame) this.frame.release();
    if (this.videoFrame) this.videoFrame.release();
    if (this.depthFrame) this.depthFrame.release();
  }

  /**
   * @typedef {Object} NotificationEventObject
   * @property {String} descr - The human readable literal description of the notification
   * @property {Float}  timestamp - The timestamp of the notification
   * @property {String} severity - The severity of the notification
   * @property {String} category - The category of the notification
   * @property {String} serializedData - The serialized data of the notification
   */

  /**
   * This callback is called when there is a device notification
   * @callback NotificationCallback
   * @param {NotificationEventObject} info
   * @param {String} info.descr - See {@link NotificationEventObject} for details
   * @param {Float}  info.timestamp - See {@link NotificationEventObject} for details
   * @param {String} info.severity - See {@link NotificationEventObject} for details
   * @param {String} info.category - See {@link NotificationEventObject} for details
   * @param {String} info.serializedData - See {@link NotificationEventObject} for details
   *
   * @see {@link NotificationEventObject}
   * @see [Sensor.setNotificationsCallback()]{@link Sensor#setNotificationsCallback}
   */

  /**
   * @event Sensor#notification
   * @param {NotificationEventObject} evt
   * @param {String} evt.descr - See {@link NotificationEventObject} for details
   * @param {Float}  evt.timestamp - See {@link NotificationEventObject} for details
   * @param {String} evt.severity - See {@link NotificationEventObject} for details
   * @param {String} evt.category - See {@link NotificationEventObject} for details
   * @param {String} evt.serializedData - See {@link NotificationEventObject} for details
   * @see {@link NotificationEventObject}
   * @see [Sensor.setNotificationsCallback()]{@link Sensor#setNotificationsCallback}
   */

  /**
   * register notifications callback
   * @param {NotificationCallback} callback The user-provied notifications callback
   * @see {@link NotificationEventObject}
   * @see [Sensor 'notification']{@link Sensor#event:notification} event
   * @return {undefined}
   */
  setNotificationsCallback(callback) {
    const funcName = 'Sensor.setNotificationsCallback()';
    checkArgumentLength(1, 1, arguments.length, funcName);
    checkArgumentType(arguments, 'function', 0, funcName);

    this._events.on('notification', (info) => {
      callback(info);
    });
    let inst = this;
    if (!this.cxxSensor.notificationCallback) {
      this.cxxSensor.notificationCallback = function(info) {
        // convert the severity and category properties from numbers to strings to be
        // consistent with documentation which are more meaningful to users
        info.severity = log_severity.logSeverityToString(info.severity);
        info.category = notification_category.notificationCategoryToString(info.category);
        inst._events.emit('notification', info);
      };
      this.cxxSensor.setNotificationCallback('notificationCallback');
    }
    return undefined;
  }

  /**
   * Get a list of stream profiles that given subdevice can provide. The returned profiles should be
   * destroyed by calling its destroy() method.
   *
   * @return {StreamProfile[]} all of the supported stream profiles
   * See {@link StreamProfile}
   */
  getStreamProfiles() {
    let profiles = this.cxxSensor.getStreamProfiles();
    if (profiles) {
      const array = [];
      profiles.forEach((profile) => {
        array.push(StreamProfile._internalCreateStreamProfile(profile));
      });
      return array;
    }
  }
}

/**
 * Sensor for managing region of interest.
 */
export class ROISensor extends Sensor {
  /**
   * Create a ROISensor out of another sensor
   * @param {Sensor} sensor a sensor object
   * @return {ROISensor|undefined} return a ROISensor if the sensor can be
   * treated as a ROISensor, otherwise return undefined.
   */
  static from(sensor) {
    if (sensor.cxxSensor.isROISensor()) {
      return new ROISensor(sensor.cxxSensor);
    }
    return undefined;
  }

  /**
   * Construct a ROISensor object, representing a RealSense camera subdevice
   * The newly created ROISensor object shares native resources with the sensor
   * argument. So the new object shouldn't be freed automatically to make
   * sure resources released only once during cleanup.
   */
   constructor(cxxSensor) {
    super(cxxSensor, false);
  }

  /**
   * @typedef {Object} RegionOfInterestObject
   * @property {Float32} minX - lower horizontal bound in pixels
   * @property {Float32} minY - lower vertical bound in pixels
   * @property {Float32} maxX - upper horizontal bound in pixels
   * @property {Float32} maxY - upper vertical bound in pixels
   * @see [Device.getRegionOfInterest()]{@link Device#getRegionOfInterest}
   */

  /**
   * Get the active region of interest to be used by auto-exposure algorithm.
   * @return {RegionOfInterestObject|undefined} Returns undefined if failed
   * @see {@link RegionOfInterestObject}
   */
  getRegionOfInterest() {
    return this.cxxSensor.getRegionOfInterest();
  }

  /**
   * Set the active region of interest to be used by auto-exposure algorithm
   * There are 2 acceptable forms of syntax:
   * <pre><code>
   *  Syntax 1. setRegionOfInterest(region)
   *  Syntax 2. setRegionOfInterest(minX, minY, maxX, maxY)
   * </code></pre>
   *
   * @param {RegionOfInterestObject} region - the region of interest to be used.
   * @param {Float32} region.minX - see {@link RegionOfInterestObject} for details.
   * @param {Float32} region.minY - see {@link RegionOfInterestObject} for details.
   * @param {Float32} region.maxX - see {@link RegionOfInterestObject} for details.
   * @param {Float32} region.maxY - see {@link RegionOfInterestObject} for details.
   *
   * @param {Float32} minX - see {@link RegionOfInterestObject} for details.
   * @param {Float32} minY - see {@link RegionOfInterestObject} for details.
   * @param {Float32} maxX - see {@link RegionOfInterestObject} for details.
   * @param {Float32} maxY - see {@link RegionOfInterestObject} for details.
   */
  setRegionOfInterest(region) {
    const funcName = 'ROISensor.setRegionOfInterest()';
    checkArgumentLength(1, 4, arguments.length, funcName);
    let minX;
    let minY;
    let maxX;
    let maxY;
    if (arguments.length === 1) {
      checkArgumentType(arguments, 'object', 0, funcName);
      minX = region.minX;
      minY = region.minY;
      maxX = region.maxX;
      maxY = region.maxY;
    } else if (arguments.length === 4) {
      checkArgumentType(arguments, 'number', 0, funcName);
      checkArgumentType(arguments, 'number', 1, funcName);
      checkArgumentType(arguments, 'number', 2, funcName);
      checkArgumentType(arguments, 'number', 3, funcName);
      minX = arguments[0];
      minY = arguments[1];
      maxX = arguments[2];
      maxY = arguments[3];
    } else {
      throw new TypeError(
          'setRegionOfInterest(region) expects a RegionOfInterestObject as argument');
    }
    this.cxxSensor.setRegionOfInterest(minX, minY, maxX, maxY);
  }
}

/**
 * Depth sensor
 */
export class DepthSensor extends Sensor {
  /**
   * Construct a device object, representing a RealSense camera
   */
  constructor(sensor) {
    super(sensor);
  }

  /**
   * Retrieves mapping between the units of the depth image and meters.
   *
   * @return {Float} depth in meters corresponding to a depth value of 1
   */
  get depthScale() {
    return this.cxxSensor.getDepthScale();
  }
}
