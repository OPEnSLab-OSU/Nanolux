import axios from 'redaxios';

const BASE_URL = 'https://audioluxmockapi.azurewebsitesnet/';
// const BASE_URL = 'http://localhost:8000';
// const BASE_URL = 'http://192.168.4.1';


// import *  as data from '../assets/url.json'
let base_url;
// if (data) {
//     base_url = data.url;
// } else {
    base_url = BASE_URL;
// }


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
    axios.get(`${base_url}/api/${path}`, {headers: {'Access-Control-Allow-Origin': '*'}})
        .then(response => response.data);

const savePattern = (pattern) =>
    axios.put(`${base_url}/api/pattern`,{pattern});


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
    getWiFiList,
    getWiFi,
    joinWiFi,
    getHostname,
    saveHostname,
    getHistory
};