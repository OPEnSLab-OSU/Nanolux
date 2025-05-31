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
get_base_url().then(() => console.log(`base_url: ${base_url}`));


const getSettings = () =>
    getData('settings');

const getPatternList = () =>
    getData('patterns');

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

const saveSettings = (settings) =>
    axios.put(`${base_url}/api/settings`, {...settings}, );

const joinWiFi = (wifi) =>
    axios.put(`${base_url}/api/wifi`,{...wifi});

const saveHostname = (hostname) =>
    axios.put(`${base_url}/api/hostname`,{hostname});

const updateLocalPassword = (pass) =>
    axios.put(`${base_url}/api/updatePassword`,{new_password: pass});

// ESSENTIAL API CALLS

// handle_pattern_put_request
const updatePattern = (patternNum, data) =>
    axios.put(`${base_url}/api/putPattern?p=${patternNum}`, data)

// handle_strip_put_request
const updateStripSettings = (data) =>
    axios.put(`${base_url}/api/putStrip`, data)

// handle_loaded_subpattern_get_request
const getPattern = (patternNum) =>
    axios.get(`${base_url}/api/getPattern?p=${patternNum}`, {headers: {'Access-Control-Allow-Origin': '*'}})
        .then(response => response.data);

// handle_loaded_pattern_settings_get_request
const getStripSettings = () =>
    axios.get(`${base_url}/api/getStrip`, {headers: {'Access-Control-Allow-Origin': '*'}})
        .then(response => response.data);

// handle_load_save_slot_put_request
const loadSaveSlot = async (slot) =>
    await axios.put(`${base_url}/api/load`, {slot, timeout: 1000})

// handle_save_to_slot_put_request
const saveToSlot = (slot) =>
    axios.put(`${base_url}/api/save`, {slot}) ;

// handle_system_settings_put_request
const updateDeviceSettings = (data) => 
    axios.put(`${base_url}/api/putSettings`, data)

// handle_system_settings_get_request
const getDeviceSettings = () => 
    axios.get(`${base_url}/api/getSettings`,  {headers: {'Access-Control-Allow-Origin': '*'}})
    .then(response => response.data);


export {
    getSettings,
    saveSettings,
    getPatternList,
    getWiFiList,
    getWiFi,
    joinWiFi,
    getWiFiJoinStatus,
    getHostname,
    saveHostname,
    getHistory,
    getPattern,
    getStripSettings,
    updatePattern,
    updateStripSettings,
    loadSaveSlot,
    saveToSlot,
    updateDeviceSettings,
    getDeviceSettings,
    updateLocalPassword,
    base_url
};