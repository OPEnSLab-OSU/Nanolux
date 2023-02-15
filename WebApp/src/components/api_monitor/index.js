import { h } from 'preact';
import style from './style.css';
import {useEffect, useState} from "preact/hooks";
import {getHistory} from "../../utils/api";

const ApiMonitor = () => {
    const [history, setHistory] = useState([]);

    useEffect(() => {
        let timer = setInterval(
            () => getHistory().then(response =>
                setHistory(oldHistory => [...oldHistory, response])),
            500);
        return () => clearInterval(timer);
    }, []);

    return (
        <div>
            <div>
                <span>API Usage</span>
            </div>
            <div>
                <textarea
                    className={style.textarea}
                    rows={10}
                    cols={90}
                >
                    {history}
                </textarea>

            </div>
        </div>
    );
}

export default ApiMonitor;
