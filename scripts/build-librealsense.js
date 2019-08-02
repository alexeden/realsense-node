const { spawn } = require('child_process');
const os = require('os');
const path = require('path');

function buildNativeLib() {
  console.log('Building librealsense C++ API. It takes time...');
  const type = os.type();
  if (type === 'Windows_NT') {
    buildNativeLibWindows();
  }
  else if (type === 'Linux') {
    buildNativeLibLinux();
  }
  else if (type === 'Darwin') {
    buildNativeLibMac();
  }
  else {
    throw new TypeError('Not supported build platform!');
  }
}

function buildNativeLibMac() {
  const child = spawn(path.resolve(__dirname, './build-dist-mac.sh'));
  child.stderr.on('data', data => console.error(`${data}`));
  child.stdout.on('data', data => console.log(`${data}`));
  child.on('close', code => {
    if (code) throw new Error(`Failed to build librealsense, build-dist-mac.sh exited with code ${code}`);
  });
}

function buildNativeLibLinux() {
  const child = spawn(path.resolve(__dirname, './build-dist-linux.sh'));
  child.stderr.on('data', data => console.error(`${data}`));
  child.stdout.on('data', data => console.log(`${data}`));
  child.on('close', code => {
    if (code) throw new Error(`Failed to build librealsense, build-dist.sh exited with code ${code}`);
  });
}

function buildNativeLibWindows() {
  const cmakeGenPlatform = process.arch;
  const msBuildPlatform = process.arch === 'ia32' ? 'Win32' : process.arch;
  const child = spawn('cmd', [
    '/c',
    '.\\build-dist-win.bat',
    cmakeGenPlatform,
    msBuildPlatform,
  ]);

  child.stderr.on('data', data => console.error(`${data}`));
  child.stdout.on('data', data => console.log(`${data}`));
  child.on('close', code => {
    if (code) throw new Error(`Failed to build librealsense, build-dist.bat exited with code ${code}`);
  });
}
buildNativeLib();


// ( async () => {
//   await genPackageJson();
//   callDistScript();
// })();

// function genPackageJson() {
//   return new Promise((resolve, reject) => {
//     exec('cp -f ./package.json scripts/npm_dist/package.json', (error, stdout, stderr) => {
//       if (error) {
//         console.log('fail to generate package.json, error:', error);
//         reject(error);
//       }

//       let distPackage = require('../scripts/npm_dist/package.json');
//       // Add a new key 'preinstall'
//       distPackage.scripts.preinstall = 'node scripts/npm_dist/build-librealsense.js';
//       jf.writeFileSync('./scripts/npm_dist/package.json', distPackage, {spaces: 2});
//       resolve();
//     });
//   });
// }

// function callDistScript() {
//   exec('./scripts/npm_dist/gen-dist.sh ', (error, stdout, stderr) => {
//     if (error) {
//       console.log('fail to generate dist package, error:', error);
//       throw error;
//     }

//     if (stdout) {
//       process.stdout.write(stdout);
//     }

//     if (stderr) {
//       process.stderr.write(stderr);
//     }
//   });
// }

// genDistPackage();
