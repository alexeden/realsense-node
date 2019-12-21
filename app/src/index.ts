import { app, BrowserWindow } from 'electron';
declare const MAIN_WINDOW_WEBPACK_ENTRY: any;

if (require('electron-squirrel-startup')) { // eslint-disable-line global-require
  // Handle creating/removing shortcuts on Windows when installing/uninstalling.
  app.quit();
}

let mainWindow: Electron.BrowserWindow | null;

const createWindow = () => {
  mainWindow = new BrowserWindow({ fullscreen: true });

  mainWindow.loadURL(MAIN_WINDOW_WEBPACK_ENTRY);

  mainWindow.on('closed', () => {
    mainWindow = null;
  });
};

app.on('ready', createWindow);

app.on('window-all-closed', () => app.quit());

app.on('activate', () => {
  // Re-create a window in the app when the dock icon is clicked and there are no other windows open
  if (mainWindow === null) {
    createWindow();
  }
});
