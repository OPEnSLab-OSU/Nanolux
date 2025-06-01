# AudioLux WebApp

Presents a graphical user interface allowing the configuration of the device. 
The application is served by a web server in the firmware. Once running, the
application will communicate with the embedded server through an API.

## Architecture

The application is very simple and based on [Preact.js](https://preactjs.com/), a minified version of React. 
Even though it is very small, Preact.js provides all the main functionality of its big
sibling. There is not enough complexity in the application to merit it being implemented
in a MVVM or MVC pattern. There is no model. Nonetheless, a modest effort is invested in
providing some decoupling and modularization.

This is a high level view of the current implementation:

![webapp breakdown](https://github.com/user-attachments/assets/0d963b15-a872-48d3-adab-40014a351759)

>Coloring Legend:
>
>BLUE: An end-user interactable component of the project.
>
>PURPLE: A web app subdirectory.
>
>GREEN: A specific file or related group of files (related .js and .css files).
>
>ORANGE: A labeled category showing the breakdown of a specific file.

## Building the Web App

To get started, you will need node and npm installed in your system. Currently, the build
process is based on Node.js version 14.17.1 and npm version 6.14.13. If you don't have node 
in your system consult the 
[official installing guide](https://docs.npmjs.com/downloading-and-installing-node-js-and-npm). It 
is highly suggested that you use a node version manager like 
[nvm](https://github.com/creationix/nvm) (or [nvm-windows](https://github.com/coreybutler/nvm-windows)).
Check the [install guide](https://www.freecodecamp.org/news/node-version-manager-nvm-install-guide/) for
more information.

Once you have node and npm in your system, clone the repo, go to the WebApp folder and use the CLI commands below to install, build, run and test the application.


### CLI Commands

```bash
# install dependencies
npm install

# serve with hot reload at localhost:8080
npm run dev

# build for production with minification
npm run build

# WINDOWS ONLY: remove unnecessary files from the production build
npm run clean

# test the production build locally
npm run serve

# run tests with jest and enzyme
npm run test

# run static tests on source code with babel and jsx-a11y
npm run lint

# run tests on live app with lighthouse
npm run lhci:collect
npm run lhci:assert
```

For detailed explanation on how things work, checkout the [CLI Readme](https://github.com/developit/preact-cli/blob/master/README.md).

For more details on testing the web app check [Testing the web app](https://github.com/OPEnSLab-OSU/Nanolux/wiki/The-Web-App#testing-the-web-app)

## Uploading the Web App to the SD card

The Web App is separate from the firmware that runs on the AudioLux device. Once the Web App is built, it must be transferred to an SD card that can be connected to the AudioLux device.

The web app is built for deployment using the `npm run build` command, as mentioned above. Once done, the app files will be in a `build` directory of the Web App folder. The following files and folders must be copied to the SD card:

 - The `assets` folder and its contents.
 - All the `.html`, `.js`, and `.json` files.
 - All the `.css` files.

Do _not_ copy the map files. Do _not_ copy the ssr-build folder. These can be removed from the `build` directory with the `npm run clean` command, as mentioned above.

Check the wiki for 
[instructions on how to upload the web app to the SD card](https://github.com/OPEnSLab-OSU/Nanolux/wiki/The-Web-App#setting-up-the-sd-card).

>NOTE: Replacing the contents of the SD card will delete any state saved to the card. In other words, all settings will revert to defaults and the device will lose any local network configuration it might have saved.
