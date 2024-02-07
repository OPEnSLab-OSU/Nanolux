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

const getAlpha = () =>
    getData('alpha');

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

const getSmoothing = () =>
    getData('smoothing')

const getBrightness = () =>
    getData('brightness')

const getLength = () =>
    getData('len')

const getMs = () =>
    getData('ms')

const getDebug = () =>
    getData('debug')

const getLoadedSubpattern = (subpattern) =>
    axios.get(`${base_url}/api/loadedSubpatterns?sp=${subpattern}`, {headers: {'Access-Control-Allow-Origin': '*'}})
        .then(response => response.data);

const getLoadedSubpatternCount = () =>
    axios.get(`${base_url}/api/loadedSubpatternCount`, {headers: {'Access-Control-Allow-Origin': '*'}})
        .then(response => response.data);

const getLoadedPatternSettings = () =>
    axios.get(`${base_url}/api/loadedPatternSettings`, {headers: {'Access-Control-Allow-Origin': '*'}})
        .then(response => response.data);

const getData = (path) =>
    axios.get(`${base_url}/api/${path}`, {headers: {'Access-Control-Allow-Origin': '*'}})
        .then(response => response.data);

const saveNoise = (noise) =>
    axios.put(`${base_url}/api/noise`,{noise});

const saveAlpha = (alpha) =>
    axios.put(`${base_url}/api/alpha`,{alpha});

const saveInSlot = (slot) =>
    axios.put(`${base_url}/api/save`,{slot});

const loadFromSlot = (slot) =>
    axios.put(`${base_url}/api/load`,{slot});

const saveExclusiveMode = (mode) =>
    axios.put(`${base_url}/api/mode`,{mode});


const saveSettings = (settings) =>
    axios.put(`${base_url}/api/settings`, {...settings}, );

const joinWiFi = (wifi) =>
    axios.put(`${base_url}/api/wifi`,{...wifi});

const saveHostname = (hostname) =>
    axios.put(`${base_url}/api/hostname`,{hostname});

const updateLoadedSubpattern = (subpattern, data) =>
    axios.put(`${base_url}/api/updateLoadedSubpattern?subpattern=${subpattern}`, {data})

const updateLoadedPattern = (data) =>
    axios.put(`${base_url}/api/updateLoadedPattern`, {data})

const modifyLoadedSubpatternCount = (data) =>
    axios.put(`${base_url}/api/modifyLoadedSubpatternCount`, {data})

export {
    getSettings,
    saveSettings,
    getPatternList,
    getPattern,
    getSecondaryPattern,
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
    saveAlpha,
    getAlpha,
    saveInSlot,
    loadFromSlot,
    getBrightness,
    getSmoothing,
    getLength,
    getDebug,
    getMs,
    getLoadedSubpattern,
    getLoadedSubpatternCount,
    getLoadedPatternSettings,
    updateLoadedSubpattern,
    updateLoadedPattern,
    modifyLoadedSubpatternCount,
    base_url
};