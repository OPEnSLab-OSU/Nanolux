import { h } from 'preact';
import style from './style.css';
import {useEffect, useState} from "preact/hooks";
import {getPatternList} from "../../utils/api";

const Patterns = () => {
    const [patterns, setPatterns] = useState([]);

    useEffect(() => {
        getPatternList().then(data => setPatterns(data))
    }, [])

    const patternOptions = patterns.map(pattern => {
        return <option key={pattern} value={pattern}>
            {pattern}
        </option>
    });

    return (
    <div>
        <label className={style.label} htmlFor="pattern-options">Current Pattern</label>
        <select className={style.label} id="pattern-options">
            {patternOptions}
        </select>
    </div>
    );
}

export default Patterns;
