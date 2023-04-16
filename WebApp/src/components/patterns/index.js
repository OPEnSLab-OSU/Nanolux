import {h} from 'preact';
import style from './style.css';
import {useEffect, useState} from "preact/hooks";
import {getPattern, getPatternList, savePattern} from "../../utils/api";

const Patterns = () => {
    const [patterns, setPatterns] = useState([]);
    const [currentPattern, setCurrentPattern] = useState("")

    useEffect(() => {
        // getPatternList().then(data => setPatterns(data));
        // getPattern().then(data => setCurrentPattern(data));
    }, [])

    const patternOptions = patterns.map(pattern => {
        return <option key={pattern} value={pattern}>
            {pattern}
        </option>
    });

    const handleSelection = async (event) => {
        const newPattern = event.target.value;
        setCurrentPattern(newPattern);
        // await savePattern(newPattern);
    }

    return (
        <div>
            <label className={style.label} htmlFor="pattern-options">Current Pattern</label>
            <select className={style.label}
                    id="pattern-options"
                    value={currentPattern}
                    onChange={handleSelection}
            >
                {patternOptions}
            </select>
        </div>
    );
}

export default Patterns;
