import { h } from 'preact';
import style from './style.css';

const Patterns = (props) => {
    const patterns = [];

    for (let pattern in props.patterns) {
        patterns.push(pattern)
    }

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
