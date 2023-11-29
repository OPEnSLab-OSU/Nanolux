import axios from 'redaxios';

// const BASE_URL = 'https://audioluxmockapi.azurewebsites.net';
// const BASE_URL = 'http://localhost:8000';
const BASE_URL = 'http://192.168.4.1';


// Load the API url from a static file. This file will be updated
// by the AudioLux firmware on boot or when a local WiFi network
// is joined.
//
let base_url;
async function get_base_url() {
    const load_url = async () => {
        try {
            const response = await fetch('../assets/url.json');
            const url_data = await response.json();
            return url_data.url;
        } catch (error) {
            return BASE_URL;
        }
    }

    base_url = await load_url()
}
get_base_url().then(_ => console.log(`base_url: ${base_url}`));


const getSettings = () =>
    getData('settings');

const getPatternList = () =>
    getData('patterns');


const getPattern = () =>
    getData('pattern');

const getSecondaryPattern = () =>
    getData('pattern2');

const getNoise = () =>
    getData('noise');

const getExclusiveMode = () =>
    getData('mode');

const getWiFiList = () =>
    getData('wifis');

const getWiFi = () =>
    getData('wifi')

const getWiFiJoinStatus = () =>
    getData('wifi_status')

const getHostname = () =>
    getData('hostname');

const getHistory = () =>
    getData('history')


const getData = (path) =>
    axios.get(`${base_url}/api/${path}`, {headers: {'Access-Control-Allow-Origin': '*'}})
        .then(response => response.data);

const savePattern = (patternIndex) =>
    axios.put(`${base_url}/api/pattern`,{index: patternIndex});

const saveSecondaryPattern = (patternIndex) =>
    axios.put(`${base_url}/api/pattern2`,{index: patternIndex});


const saveNoise = (noise) =>
    axios.put(`${base_url}/api/noise`,{noise});

const saveExclusiveMode = (mode) =>
    axios.put(`${base_url}/api/mode`,{mode});


const saveSettings = (settings) =>
    axios.put(`${base_url}/api/settings`, {...settings}, );

const joinWiFi = (wifi) =>
    axios.put(`${base_url}/api/wifi`,{...wifi});

const saveHostname = (hostname) =>
    axios.put(`${base_url}/api/hostname`,{hostname});

export {
    getSettings,
    saveSettings,
    getPatternList,
    getPattern,
    savePattern,
    getSecondaryPattern,
    saveSecondaryPattern,
    getNoise,
    saveNoise,
    getWiFiList,
    getWiFi,
    joinWiFi,
    getWiFiJoinStatus,
    getHostname,
    saveHostname,
    getHistory,
    getExclusiveMode,
    saveExclusiveMode,
    base_url
};