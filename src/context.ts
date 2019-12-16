
export class Context {
  /**
   * There are only one acceptable form of syntax to create a Context for users:
   * <pre><code>
   *  new Context();
   * </code></pre>
   * other forms are reserved for internal use only.
   */
  constructor(cxxCtx) {
    const funcName = 'Context.constructor()';
    // Internal code will create Context with cxxObject or other params
    checkDiscreteArgumentLength([0, 1, 3, 4], arguments.length, funcName);
    this._events = new EventEmitter();
    if (arguments.length === 0) {
      this.cxxCtx = new RS2.RSContext();
      this.cxxCtx.create();
    } else if (arguments.length === 1) {
      checkArgumentType(arguments, RS2.RSContext, 0, funcName);
      this.cxxCtx = cxxCtx;
    } else {
      checkArgumentType(arguments, 'string', 0, funcName);
      checkDiscreteArgumentValue(arguments, 0, ['recording', 'playback'], funcName);
      this.cxxCtx = new (Function.prototype.bind.apply(
          RS2.RSContext, [null].concat(Array.from(arguments))))();
      this.cxxCtx.create();
    }
    this.cxxCtx._events = this._events;
    internal.addContext(this);
  }

  /**
   * Cleanup underlying C++ context, and release all resources that were created by this context.
   * The JavaScript Context object(s) will not be garbage-collected without call(s) to this function
   */
  destroy() {
    if (this.cxxCtx) {
      this.cxxCtx.destroy();
      this.cxxCtx = undefined;
      internal.cleanupContext();
    }
  }

  /**
   * Get the events object of EventEmitter
   * @return {EventEmitter}
   */
  get events() {
    return this._events;
  }

  /**
  * Create a static snapshot of all connected devices at the time of the call
  * @return {DeviceList|undefined} connected devices at the time of the call
  */
  queryDevices() {
    let list = this.cxxCtx.queryDevices();
    return (list ? new DeviceList(list) : undefined);
  }

  /**
  * Generate an array of all available sensors from all RealSense devices
  * @return {Sensor[]|undefined}
  */
  querySensors() {
    let devList = this.queryDevices();
    if (!devList) {
      return undefined;
    }
    let devices = devList.devices;
    if (devices && devices.length) {
      const array = [];
      devices.forEach((dev) => {
        const sensors = dev.querySensors();
        sensors.forEach((sensor) => {
          array.push(sensor);
        });
      });
      return array;
    }
    return undefined;
  }

  /**
   * Get the device from one of its sensors
   *
   * @param {Sensor} sensor
   * @return {Device|undefined}
   */
  getSensorParent(sensor) {
    const funcName = 'Context.getSensorParent()';
    checkArgumentLength(1, 1, arguments.length, funcName);
    checkArgumentType(arguments, Sensor, 0, funcName);

    let cxxDev = this.cxxCtx.createDeviceFromSensor(sensor.cxxSensor);
    if (!cxxDev) {
      return undefined;
    }
    return Device._internalCreateDevice(cxxDev);
  }

  /**
   * When one or more devices are plugged or unplugged into the system
   * @event Context#device-changed
   * @param {DeviceList} removed - The devices removed from the system
   * @param {DeviceList} added - The devices added to the system
   */

  /**
   * This callback is called when number of devices is changed
   * @callback devicesChangedCallback
   * @param {DeviceList} removed - The devices removed from the system
   * @param {DeviceList} added - The devices added to the system
   *
   * @see [Context.setDevicesChangedCallback]{@link Context#setDevicesChangedCallback}
   */

  /**
   * Register a callback function to receive device-changed notification
   * @param {devicesChangedCallback} callback - devices changed callback
   */
  setDevicesChangedCallback(callback) {
    const funcName = 'Context.setDevicesChangedCallback()';
    checkArgumentLength(1, 1, arguments.length, funcName);
    checkArgumentType(arguments, 'function', 0, funcName);

    this._events.on('device-changed', (removed, added) => {
      callback(removed, added);
    });
    let inst = this;
    if (!this.cxxCtx.deviceChangedCallback) {
      this.cxxCtx.deviceChangedCallback = function(removed, added) {
        let rmList = (removed ? new DeviceList(removed) : undefined);
        let addList = (added ? new DeviceList(added) : undefined);
        inst._events.emit('device-changed', rmList, addList);
      };
      this.cxxCtx.setDevicesChangedCallback('deviceChangedCallback');
    }
  }
