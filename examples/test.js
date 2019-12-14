const { addon } = require('../dist');

process
  .once('SIGHUP', () => {
    console.log('RESTART!');
  })
  .once('SIGUSR2', () => {
    console.log('SIGUSR2!');
    addon.cleanup();
  });

console.log('Creating context...');
const ctx = new addon.RSContext();
console.log(ctx);

console.log('Querying for devices...');
const deviceList = ctx.queryDevices()
console.log(`Device list length: ${deviceList.length()}`);

ctx.onDevicesChanged((rmList, addList) => {
  if (rmList && typeof rmList.length === 'function') {
    console.log(`Devices removed: `, rmList.length());
  }

  if (addList && typeof addList.length === 'function') {
    console.log(`Devices added: `, addList.length());
  }
});
