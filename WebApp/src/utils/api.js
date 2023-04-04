import axios from 'redaxios';

const BASE_URL = 'http://localhost:8000';

const getSettings = () =>
    getData('settings');


const getPatternList = () =>
    getData('patterns');


const getPattern = () =>
    getData('pattern');


const getHistory = () =>
    getData('history')


const getData = (path) =>
    axios.get(`${BASE_URL}/api/${path}`).then(response => response.data);


const savePattern = (pattern) =>
    axios.put(`${BASE_URL}/api/pattern`,{pattern});


const saveSettings = (settings) =>
    axios.put(`${BASE_URL}/api/settings`, {...settings}, );


export { getSettings, saveSettings, getPatternList, getPattern, savePattern, getHistory };