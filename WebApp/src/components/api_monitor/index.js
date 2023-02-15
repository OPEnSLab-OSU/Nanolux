import { h } from 'preact';
import style from './style.css';
import {useEffect, useRef, useState} from "preact/hooks";
import {getHistory} from "../../utils/api";

const ApiMonitor = () => {
    const [history, setHistory] = useState([]);
    const textarea = useRef(null)

    useEffect(() => {
        let timer = setInterval(
            () => getHistory().then(response =>
                setHistory(oldHistory => [...oldHistory, response])),
            500);
        return () => clearInterval(timer);
    }, []);

    useEffect(() => {
            if (textarea.current) {
                textarea.current.scrollTop = textarea.current.scrollHeight;
            }
        }, [history]);

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
                    ref={textarea}
                >
                    {history}
                </textarea>

            </div>
        </div>
    );
}

export default ApiMonitor;
