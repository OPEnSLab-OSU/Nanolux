import {h} from 'preact';
import style from './style.css';
import {useEffect, useState} from "preact/hooks";
import {getPattern, getPatternList, savePattern} from "../../utils/api";
import {useConnectivity} from "../../context/online_context";
import useInterval from "../../utils/use_interval";
import Spinner from "../spinner";

const Patterns = () => {
    const {isConnected} = useConnectivity();
    const [patterns, setPatterns] = useState([]);
    const [currentPattern, setCurrentPattern] = useState(-1)
    const [loading, setLoading] = useState(true);


    useEffect(() => {
        if (isConnected) {
            getPatternList().then(data => setPatterns(data));
            getPattern().then(data => setCurrentPattern(data.index));
            setLoading(false);
        }
    }, [isConnected])

    const patternOptions = patterns.map(pattern => {
        return <option key={pattern.index} value={pattern.index}>
            {pattern.name}
        </option>
    });

    const refreshPattern = () => {
        getPattern().then(data => setCurrentPattern(data.index));
    }

    useInterval(() => {
        if (isConnected) {
            refreshPattern();
        }
    }, 5000);

    const handleSelection = async (event) => {
        const newPattern = event.target.value;
        setCurrentPattern(newPattern);
        if  (isConnected) {
            await savePattern(newPattern);
        }
    }

    return (
        <div>
            {loading ? (
                <Spinner />
            ) : (
                <div>
                    <label className={style.label} htmlFor="pattern-options">Current Pattern</label>
                    <select className={style.label}
                            id="pattern-options"
                            value={currentPattern}
                            onChange={handleSelection}
                    >
                        <option value="-1">Requesting pattern list...</option>
                        {patternOptions}
                    </select>
                </div>
                )}
        </div>
    );
}

export default Patterns;
