import axios from 'redaxios';

// const BASE_URL = 'https://audioluxmockapi.azurewebsites.net/';
const BASE_URL = 'http://localhost:8000';
// const BASE_URL = 'http://192.168.4.1';

const getSettings = () =>
    getData('settings');


const getPatternList = () =>
    getData('patterns');


const getPattern = () =>
    getData('pattern');


const getWiFiList = () =>
    getData('wifis');

const getWiFi = () =>
    getData('wifi')

const getHostname = () =>
    getData('hostname');

const getHistory = () =>
    getData('history')


const getData = (path) =>
    axios.get(`${BASE_URL}/api/${path}`, {headers: {'Access-Control-Allow-Origin': '*'}})
        .then(response => response.data);

const savePattern = (pattern) =>
    axios.put(`${BASE_URL}/api/pattern`,{pattern});


const saveSettings = (settings) =>
    axios.put(`${BASE_URL}/api/settings`, {...settings}, );

const joinWiFi = (wifi) =>
    axios.put(`${BASE_URL}/api/wifi`,{...wifi});

const saveHostname = (hostname) =>
    axios.put(`${BASE_URL}/api/hostname`,{hostname});


export {
    getSettings,
    saveSettings,
    getPatternList,
    getPattern,
    savePattern,
    getWiFiList,
    getWiFi,
    joinWiFi,
    getHostname,
    saveHostname,
    getHistory
};