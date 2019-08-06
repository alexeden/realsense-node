const { addon } = require('../dist');

console.log('Creating context...');
const ctx = new addon.RSContext();
console.log(ctx);

console.log('Querying for devices...');
console.log(ctx.queryDevices());

console.log('Done!');
