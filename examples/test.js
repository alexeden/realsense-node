const { addon: rs, RSOption, Pipeline, RSStream } = require('../dist');

process
  .once('SIGHUP', () => {
    console.log('RESTART!');
  })
  .once('SIGUSR2', () => {
    console.log('SIGUSR2!');
    rs.cleanup();
    // process.exit(0);
  });

process.on('beforeExit', () => {
  rs.cleanup();
});
const pipeline = new Pipeline();

const profile = pipeline.start();

const device = profile.cxxPipelineProfile.getDevice();
console.log(device);
profile.cxxPipelineProfile.getStreams().forEach(stream => {
  console.log(`${RSStream[stream.stream()]} stream ${stream.index()}`)
});

console.log(pipeline.waitForFrames());
console.log(pipeline.waitForFrames());

console.log('Creating context...');
const ctx = new rs.RSContext();

console.log('Querying for devices...');
const deviceList = ctx.queryDevices();
console.log(`Device list length: ${deviceList.length()}`);

deviceList.forEach((dev, i) => {
  console.log(`Device #${i + 1}: `, dev);
  dev.querySensors().forEach(sensor => {
    console.log(`I'm a sensor!`, sensor);
    console.log('isDepthSensor: ', sensor.isDepthSensor);
    console.log('isROISensor: ', sensor.isROISensor);
    console.log('getCameraInfo: ', sensor.getCameraInfo(0));
    console.log('getStreamProfiles length: ', sensor.getStreamProfiles().length);
    console.log('supports RSOption.LedPower: ', sensor.supportsOption(RSOption.LedPower));

    sensor.onNotification(notification => {
      console.log('notification!', notification);
    });
  });
});

ctx.onDevicesChanged((rmList, addList) => {
  if (rmList && typeof rmList.length === 'function') {
    console.log(`Devices removed: `, rmList.length());
  }

  if (addList && typeof addList.length === 'function') {
    console.log(`Devices added: `, addList.length());
  }
});
