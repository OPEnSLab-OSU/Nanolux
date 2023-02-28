import axios from 'redaxios';

const BASE_URL = 'https://audioluxmockapi.azurewebsites.net/';

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

const getHistory = () =>
    getData('history')


const getData = (path) =>
    axios.get(`${BASE_URL}/api/${path}`).then(response => response.data);

const savePattern = (pattern) =>
    axios.put(`${BASE_URL}/api/pattern`,{pattern});


const saveSettings = (settings) =>
    axios.put(`${BASE_URL}/api/settings`, {...settings}, );

const joinWiFi = (wifi) =>
    axios.put(`${BASE_URL}/api/pattern`,{wifi});


export {
    getSettings,
    saveSettings,
    getPatternList,
    getPattern,
    savePattern,
    getWiFiList,
    getWiFi,
    joinWiFi,
    getHistory
};