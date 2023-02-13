import axios from 'redaxios';

const BASE_URL = 'https://audioluxmockapi.azurewebsites.net/';

const getSettings = () =>
    getData('settings');


const getPatternList = () =>
    getData('patterns');


const getPattern = () =>
    getData('pattern');


const getData = (path) =>
    axios.get(`${BASE_URL}/api/${path}`).then(response => response.data);


const setPattern = (pattern) =>
    axios.put(`${BASE_URL}/api/pattern/`,{pattern});


export { getSettings, getPatternList, getPattern, setPattern };