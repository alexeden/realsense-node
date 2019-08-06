const { addon } = require('../dist');

console.log('Creating context...');
const ctx = new addon.RSContext();
console.log(ctx);

console.log('Querying for devices...');
const deviceList = ctx.queryDevices()
console.log('Got device list');
console.log('Getting device count...')
console.log(deviceList.size());

addon.cleanup();
console.log('Done!');
