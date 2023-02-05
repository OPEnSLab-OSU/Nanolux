import axios from 'redaxios';

const BASE_URL = 'http://localhost:8000';

const getSettings = () =>
    getData('settings');


const getPatternList = () =>
    getData('patterns');


const getPattern = () =>
    getData('pattern');


const getData = (path) =>
    axios.get(`${BASE_URL}/api/${path}`).then(response => response.data);

export { getSettings, getPatternList, getPattern };