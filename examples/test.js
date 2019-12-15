const { addon } = require('../dist');

process
  .once('SIGHUP', () => {
    console.log('RESTART!');
  })
  .once('SIGUSR2', () => {
    console.log('SIGUSR2!');
    addon.cleanup();
    process.exit(0);
  });

console.log('Creating context...');
const ctx = new addon.RSContext();

console.log('Querying for devices...');
const deviceList = ctx.queryDevices()
console.log(`Device list length: ${deviceList.length()}`);

deviceList.forEach((dev, i) => {
  console.log(`Device #${i + 1}: `, dev);
});

ctx.onDevicesChanged((rmList, addList) => {
  if (rmList && typeof rmList.length === 'function') {
    console.log(`Devices removed: `, rmList.length());
  }

  if (addList && typeof addList.length === 'function') {
    console.log(`Devices added: `, addList.length());
  }
});
