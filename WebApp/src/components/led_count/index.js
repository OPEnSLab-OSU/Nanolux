import { h } from 'preact';
import style from './style.css';
import {useState, useEffect} from "preact/hooks";

const LedCount = ({
        label,
        savedValue,
        min,
        max,
        onValueChanged
}) => {
    const [count, setCount] = useState(savedValue);

    useEffect(() => {
        if (savedValue) {
            setCount(savedValue);
        }
    }, [savedValue])


    const valueChanged = event => {
        setCount(event.target.value);
        onValueChanged(count);
    }

    return (
        <div>
            <div>
                <span className={style.label}>{label}</span>
            </div>
            <div>
                <input
                    className={style.spin_button}
                    type="number"
                    id="spinner"
                    name="spinner"
                    min={min}
                    max={max}
                    value={count}
                    onChange={valueChanged}
                />
            </div>
        </div>
    );
}

export default LedCount;
